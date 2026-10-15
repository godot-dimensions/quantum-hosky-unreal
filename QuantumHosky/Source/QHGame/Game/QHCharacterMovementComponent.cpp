#include "QHGame/Game/QHCharacterMovementComponent.h"

#include "Unreal4DCore/Math/Math4D.h"

#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "KismetTraceUtils.h"

//______________________________________________________________________________
static TAutoConsoleVariable<float> CVar_QHCharacterCapsuleDebugDraw(TEXT("char.capsule.show"), -1.0f, TEXT("Draw character capsules if >= 0."));
static TAutoConsoleVariable<float> CVar_QHCharacterMovementClimbDebugDraw(TEXT("char.move.climb.show"), -1.0f, TEXT("Draw character climb operations if >= 0."));
static TAutoConsoleVariable<float> CVar_QHCharacterMovementMantleDebugDraw(TEXT("char.move.mantle.show"), -1.0f, TEXT("Draw character mantling operations if >= 0."));

////////////////////////////////////////////////////////////////////////////////
bool UQHCharacterMovementComponent::QueryClimb(FQHCharacterMovementClimbInfo& OutInfo, const FQHCharacterMovementClimbQuery& InQuery) const
{
	// TODO: Add support for convex / concave corner detection.

	// 1. Sample in a square grid
	// 2. Accumulate all samples into an aggregate climb surface

	ACharacter* Owner = GetCharacterOwner();
	check(Owner);

	UCapsuleComponent* CharacterCapsuleComponent = Owner->GetCapsuleComponent();
	check(CharacterCapsuleComponent);

	UWorld* World = GetWorld();
	check(World);

	// Query from the character's eyes
	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	const FVector QueryForward = InQuery.QueryDirection;
	const FVector QueryRight = FVector::CrossProduct(FVector::UpVector, QueryForward).GetSafeNormal();
	const FVector QueryUp = FVector::CrossProduct(QueryForward, QueryRight).GetSafeNormal();

	FCollisionQueryParams QueryParams;
	FCollisionResponseParams QueryResponseParams;
	CharacterCapsuleComponent->InitSweepCollisionParams(QueryParams, QueryResponseParams);

	QueryParams.AddIgnoredActor(Owner);

#if UE_ENABLE_DEBUG_DRAWING
	const float DebugDrawDuration = (InQuery.DebugDrawDuration >= 0.0f)
								  ? InQuery.DebugDrawDuration
								  : CVar_QHCharacterMovementClimbDebugDraw.GetValueOnGameThread();

	const bool DebugDrawQuery = (DebugDrawDuration >= 0.0f);
#endif

	// Sample in a square grid

	// Start at the top-left corner
	FVector SampleQueryOrigin = ViewLocation
							  - (QueryRight * (InQuery.QueryDimension * 0.5f))
							  + (QueryUp * (InQuery.QueryDimension * 0.5f));

	const float SampleLength = UMath4D::SafeDivide(InQuery.QueryDimension, (InQuery.QuerySamples - 1));

	const FVector SampleQueryRight = (QueryRight * SampleLength);
	const FVector SampleQueryDown = -(QueryUp * SampleLength);

	int32 NumSamples = 0;

	for (int32 i = 0; i < InQuery.QuerySamples; ++i)
	{
		for (int32 j = 0; j < InQuery.QuerySamples; ++j)
		{
			const FVector SampleQueryStart = SampleQueryOrigin
										   + (SampleQueryRight * i)
										   + (SampleQueryDown * j);

			const FVector SampleQueryEnd = (SampleQueryStart + (QueryForward * InQuery.QueryDistance));

			FHitResult SampleQueryHit;
			World->LineTraceSingleByChannel(SampleQueryHit, SampleQueryStart, SampleQueryEnd, InQuery.QueryChannel, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
			if (DebugDrawQuery)
			{
				DrawDebugLineTraceSingle(World, SampleQueryStart, SampleQueryEnd, EDrawDebugTrace::ForDuration, SampleQueryHit.bBlockingHit, SampleQueryHit, FLinearColor::Green, FLinearColor::Red, DebugDrawDuration);
			}
#endif

			// Accumulate the sample
			if (SampleQueryHit.bBlockingHit)
			{
				++NumSamples;

				OutInfo.ClimbSurfaceLocation += SampleQueryHit.ImpactPoint;
				OutInfo.ClimbSurfaceNormal += SampleQueryHit.ImpactNormal;
			}
		}
	}

	// Finalize aggregate climb surface parameters
	if (NumSamples > 0)
	{
		OutInfo.ClimbSurfaceLocation /= NumSamples;
		OutInfo.ClimbSurfaceNormal.Normalize();
	}

	// Query succeeds if any samples are found;
	// in the future we can make this more complex if necessary
	return (NumSamples > 0);
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::StartClimbing(const FQHCharacterMovementClimbQuery& InQuery, const FQHCharacterMovementClimbInfo& InInfo)
{
	ClimbQuery = InQuery;
	ClimbInfo = InInfo;

	SetMovementMode(EMovementMode::MOVE_Custom, uint8(EQHCharacterMovementMode::Climbing));
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::StopClimbing()
{
	if (IsClimbing())
	{
		SetDefaultMovementMode();
	}
}

////////////////////////////////////////////////////////////////////////////////
bool UQHCharacterMovementComponent::IsClimbing() const
{
	return (MovementMode == EMovementMode::MOVE_Custom)
		&& (CustomMovementMode == uint8(EQHCharacterMovementMode::Climbing));
}

////////////////////////////////////////////////////////////////////////////////
FQHCharacterMovementClimbInfo UQHCharacterMovementComponent::GetClimbInfo() const
{
	return IsClimbing()
		 ? ClimbInfo
		 : FQHCharacterMovementClimbInfo();
}

////////////////////////////////////////////////////////////////////////////////
bool UQHCharacterMovementComponent::QueryMantle(FQHCharacterMovementMantleInfo& OutInfo, const FQHCharacterMovementMantleQuery& InQuery) const
{
	// TODO: Deflate the character capsule when performing mantle queries to avoid over-conservatively snagging on surfaces

	// 1. Query forward to find a wall
	// 2. From the wall, query up to ensure there isnt a ceiling above us
	// 3. Query forward to ensure there is clearance over the ledge
	// 4. Query down to find a landing spot
	// 5. Verify there is room to land

	bool Result = false;

	ACharacter* Owner = GetCharacterOwner();
	check(Owner);

	UCapsuleComponent* CharacterCapsuleComponent = Owner->GetCapsuleComponent();
	check(CharacterCapsuleComponent);

	UWorld* World = GetWorld();
	check(World);

	// Query from the character's eyes
	FVector ViewLocation;
	FRotator ViewRotation;
	Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);

	// Query in the character's view direction
	const FVector QueryDirection = Owner->GetActorForwardVector();
	const FCollisionShape QueryShape = FCollisionShape::MakeSphere(InQuery.QueryRadius);

	FCollisionQueryParams QueryParams;
	FCollisionResponseParams QueryResponseParams;
	CharacterCapsuleComponent->InitSweepCollisionParams(QueryParams, QueryResponseParams);

	QueryParams.AddIgnoredActor(Owner);
	QueryParams.bFindInitialOverlaps = false; // NOTE: We wish to ignore initial overlaps to avoid snagging on surfaces, however this doesnt seem to do anything under Chaos physics.

#if UE_ENABLE_DEBUG_DRAWING
	const float DebugDrawDuration = (InQuery.DebugDrawDuration >= 0.0f)
								  ? InQuery.DebugDrawDuration
								  : CVar_QHCharacterMovementMantleDebugDraw.GetValueOnGameThread();

	const bool DebugDrawQuery = (DebugDrawDuration >= 0.0f);
#endif

	// Query forward to find a wall

	const FVector WallQueryStart = ViewLocation;
	const FVector WallQueryEnd = (WallQueryStart + (QueryDirection * InQuery.QueryDistance));

	FHitResult WallQueryHit;
	World->SweepSingleByChannel(WallQueryHit, WallQueryStart, WallQueryEnd, FQuat::Identity, InQuery.QueryChannel, QueryShape, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
	if (DebugDrawQuery)
	{
		DrawDebugSphereTraceSingle(World, WallQueryStart, WallQueryEnd, InQuery.QueryRadius, EDrawDebugTrace::ForDuration, WallQueryHit.bBlockingHit, WallQueryHit, FLinearColor::Green, FLinearColor::Red, DebugDrawDuration);
	}
#endif

	if (WallQueryHit.bBlockingHit)
	{
		const float CharacterCapsuleRadius = CharacterCapsuleComponent->GetScaledCapsuleRadius();
		const float CharacterCapsuleHalfHeight = CharacterCapsuleComponent->GetScaledCapsuleHalfHeight();
		const FCollisionShape CharacterQueryShape = FCollisionShape::MakeCapsule(CharacterCapsuleRadius, CharacterCapsuleHalfHeight);
		const ECollisionChannel CharacterQueryChannel = CharacterCapsuleComponent->GetCollisionObjectType();

		// Query upwards to ensure there isnt a ceiling above us

		const float HeightQueryBackaway = (InQuery.QueryRadius * 0.25f); // Backaway from the wall to avoid snagging on it; the further the backaway distance, the more of an outwardly steep surface we can mantle
		const FVector HeightQueryStart = (WallQueryHit.ImpactPoint + (WallQueryHit.ImpactNormal * (CharacterCapsuleRadius + HeightQueryBackaway)));
		const FVector HeightQueryEnd = (HeightQueryStart + (FVector::UpVector * InQuery.QueryHeight));

		FHitResult HeightQueryHit;
		World->SweepSingleByChannel(HeightQueryHit, HeightQueryStart, HeightQueryEnd, FQuat::Identity, CharacterQueryChannel, CharacterQueryShape, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
		if (DebugDrawQuery)
		{
			DrawDebugCapsuleTraceSingle(World, HeightQueryStart, HeightQueryEnd, CharacterCapsuleRadius, CharacterCapsuleHalfHeight, FRotator::ZeroRotator, EDrawDebugTrace::ForDuration, HeightQueryHit.bBlockingHit, HeightQueryHit, FLinearColor::Green, FLinearColor::Red, DebugDrawDuration);
		}
#endif

		if (!HeightQueryHit.bBlockingHit)
		{
			// Query forward to ensure there is clearance over the ledge

			const FVector ClearanceQueryStart = HeightQueryEnd;
			const FVector ClearanceQueryEnd = (ClearanceQueryStart + (QueryDirection * ((CharacterCapsuleRadius * 2.0f) + HeightQueryBackaway))); // Query forward just enough so that we are standing on the edge
			const FCollisionShape ClearanceQueryShape = FCollisionShape::MakeSphere(CharacterCapsuleRadius); // NOTE: Query by sphere to avoid snagging with our feet; TODO: Replace with deflated capsule query

			FHitResult ClearanceQueryHit;
			World->SweepSingleByChannel(ClearanceQueryHit, ClearanceQueryStart, ClearanceQueryEnd, FQuat::Identity, CharacterQueryChannel, ClearanceQueryShape, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
			if (DebugDrawQuery)
			{
				DrawDebugSphereTraceSingle(World, ClearanceQueryStart, ClearanceQueryEnd, CharacterCapsuleRadius, EDrawDebugTrace::ForDuration, ClearanceQueryHit.bBlockingHit, ClearanceQueryHit, FLinearColor::Green, FLinearColor::Red, DebugDrawDuration);
			}
#endif

			if (!ClearanceQueryHit.bBlockingHit)
			{
				// Query down to find a landing spot

				const FVector LandingQueryStart = ClearanceQueryEnd;
				const FVector LandingQueryEnd = (LandingQueryStart + (FVector::DownVector * InQuery.QueryHeight));
				const FCollisionShape LandingQueryShape = FCollisionShape::MakeSphere(CharacterCapsuleRadius); // NOTE: Query by sphere to avoid initial penetration; TODO: Replace with deflated capsule query

				FHitResult LandingQueryHit;
				World->SweepSingleByChannel(LandingQueryHit, LandingQueryStart, LandingQueryEnd, FQuat::Identity, CharacterQueryChannel, LandingQueryShape, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
				if (DebugDrawQuery)
				{
					DrawDebugSphereTraceSingle(World, LandingQueryStart, LandingQueryEnd, CharacterCapsuleRadius, EDrawDebugTrace::ForDuration, LandingQueryHit.bBlockingHit, LandingQueryHit, FLinearColor::Green, FLinearColor::Red, DebugDrawDuration);
				}
#endif

				if (LandingQueryHit.bBlockingHit)
				{
					const FVector LandingQueryHitCapsuleLocation = (LandingQueryHit.Location + (FVector::UpVector * (CharacterCapsuleHalfHeight - CharacterCapsuleRadius)));
					const FVector StandingQueryLocation = (LandingQueryHitCapsuleLocation + (FVector::UpVector * MIN_FLOOR_DIST));

					// Verify the landing spot

					LandingQueryHit.Location = LandingQueryHitCapsuleLocation; // `IsValidLandingSpot` assumes an `FHitResult` from a capsule sweep but we used a sphere sweep so fix-up the `FHitResult`
					if (IsValidLandingSpot(StandingQueryLocation, LandingQueryHit))
					{
						// Verify there is room to land

						const bool StandingQueryOverlap = World->OverlapBlockingTestByChannel(StandingQueryLocation, FQuat::Identity, CharacterQueryChannel, CharacterQueryShape, QueryParams, QueryResponseParams);

#if UE_ENABLE_DEBUG_DRAWING
						if (DebugDrawQuery)
						{
							DrawDebugCapsule(World, StandingQueryLocation, CharacterCapsuleHalfHeight, CharacterCapsuleRadius, FQuat::Identity, (StandingQueryOverlap ? FColor::Red : FColor::Green), false, DebugDrawDuration);
						}
#endif

						if (!StandingQueryOverlap)
						{
							OutInfo.MantleTargetLocation = StandingQueryLocation; // Mantle to the validated location

							Result = true;
						}
					}
				}
			}
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::StartMantling(const FQHCharacterMovementMantleInfo& InInfo)
{
	MantleInfo = InInfo;

	SetMovementMode(EMovementMode::MOVE_Custom, uint8(EQHCharacterMovementMode::Mantling));
}

////////////////////////////////////////////////////////////////////////////////
bool UQHCharacterMovementComponent::IsMantling() const
{
	return (MovementMode == EMovementMode::MOVE_Custom)
		&& (CustomMovementMode == uint8(EQHCharacterMovementMode::Mantling));
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::PhysClimbing(const float InDeltaTime, const int32 InIterations)
{
	// TODO: Align the capsule with the climb surface to better support scrambling-up steep slopes or climbing on the underside of over-hangs.

	ACharacter* Owner = GetCharacterOwner();
	check(Owner);

	UCapsuleComponent* CharacterCapsuleComponent = Owner->GetCapsuleComponent();
	check(CharacterCapsuleComponent);

	// Update the climb surface by querying in the direction of the current climb surface

	FQHCharacterMovementClimbQuery Query = ClimbQuery;
	Query.QueryDirection = -ClimbInfo.ClimbSurfaceNormal;

	ClimbInfo = FQHCharacterMovementClimbInfo();
	if (!QueryClimb(ClimbInfo, Query))
	{
		// If no climb surface, stop climbing

		SetDefaultMovementMode();

		StartNewPhysics(InDeltaTime, InIterations);
	}
	else
	{
		const FQuat CharacterRotation = Owner->GetActorQuat();

		const FVector ClimbRight = FVector::CrossProduct(ClimbInfo.ClimbSurfaceNormal, FVector::UpVector).GetSafeNormal();
		const FVector ClimbUp = FVector::CrossProduct(ClimbRight, ClimbInfo.ClimbSurfaceNormal).GetSafeNormal();

		// Unrotate input / acceleration from world-space to local-space;
		// TODO: Store controller-space input / acceleration for convenience
		Acceleration = CharacterRotation.UnrotateVector(Acceleration);

		// Calculate input / acceleration in climb-space
		Acceleration = (ClimbUp * Acceleration.X)
					 + (ClimbRight * Acceleration.Y);

		// Integrate velocity / apply braking
		CalcVelocity(InDeltaTime, 0.0f, false, GetMaxBrakingDeceleration());

		// Apply gravity towards the climb surface
		const float ClimbGravity = 980.0f;
		Velocity += (-ClimbInfo.ClimbSurfaceNormal * (ClimbGravity * InDeltaTime));

		float DeltaTime = InDeltaTime;
		int32 Iterations = InIterations;

		// Integrate position and perform collision
		while (!Velocity.IsNearlyZero() && (DeltaTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
		{
			// Integrate position

			const FVector MovementDelta = (Velocity * DeltaTime);

			// Sweep capsule
			FHitResult Hit;
			SafeMoveUpdatedComponent(MovementDelta, CharacterRotation, true, Hit);

			float HitT = 1.0f;

			// Handle collision
			if (Hit.bBlockingHit)
			{
				const float HitDot = FVector::DotProduct(Hit.ImpactNormal, Velocity);

				// Ignore collisions that we are moving away from
				if (HitDot < 0.0f)
				{
					// Slide along the collision surface
					Velocity -= (Hit.ImpactNormal * HitDot);

					HitT = Hit.Time;

					// Notify hit
					HandleImpact(Hit, (DeltaTime * HitT), MovementDelta);
				}
			}

			// Update timestep

			DeltaTime *= (1.0f - HitT);

			Iterations++;
		}

		// Backaway the capsule from the climb surface to avoid snagging on it;
		// NOTE: It is important to perform this following gravity otherwise the character will jitter against the climb surface
		{
			const FVector CharacterLocation = UpdatedComponent->GetComponentLocation();

			const float ClimbBackaway = (ClimbQuery.QueryDistance * 0.25f);
			const FVector ClimbBackawayLocation = (ClimbInfo.ClimbSurfaceLocation + (ClimbInfo.ClimbSurfaceNormal * (CharacterCapsuleComponent->GetScaledCapsuleRadius() + ClimbBackaway)));
			const FVector ToClimbBackaway = (ClimbBackawayLocation - CharacterLocation);
			const float ClimbBackwayError = FVector::DotProduct(ToClimbBackaway, ClimbInfo.ClimbSurfaceNormal);

			// If the capsule is between the backaway point and the climb surface
			if (ClimbBackwayError > 0.0f)
			{
				// Move the capsule to the backaway point
				FHitResult BackawayHit;
				SafeMoveUpdatedComponent((ClimbInfo.ClimbSurfaceNormal * ClimbBackwayError), CharacterRotation, true, BackawayHit);

				// Remove velocity into the climb surface
				Velocity -= (ClimbInfo.ClimbSurfaceNormal * FMath::Min(FVector::DotProduct(Velocity, ClimbInfo.ClimbSurfaceNormal), 0.0f));
			}
		}

		// Check if we have landed on the ground
		{
			const FVector CharacterLocation = UpdatedComponent->GetComponentLocation();

			FFindFloorResult FloorResult;
			FindFloor(CharacterLocation, FloorResult, false, nullptr);

			// If moving into the ground, stop climbing
			if (FloorResult.IsWalkableFloor() && IsValidLandingSpot(CharacterLocation, FloorResult.HitResult) && (FVector::DotProduct(Velocity, FloorResult.HitResult.ImpactNormal) < 0.0f))
			{
				SetDefaultMovementMode();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::PhysMantling(const float InDeltaTime, const int32 InIterations)
{
	// TODO: Currently mantling does not perform collision checks to avoid over-conservatively canceling the mantle, however this can result in the character potentially clipping into geometry.
	// Instead, perform collision with a deflated capsule to avoid snagging on surfaces.

	// TODO: Mantle towards the current view direction to improve responsive of mantle.

	// TODO: Prevent/cancel mantle if looking away from the mantle target.

	bool MantleComplete = false;

	ACharacter* Owner = GetCharacterOwner();
	check(Owner);

	const FVector Location = UpdatedComponent->GetComponentLocation();
	const FVector ToMantle = (MantleInfo.MantleTargetLocation - Location).GetSafeNormal();

	// Accelerate towards the mantle target
	float Speed = Velocity.Size();
	Speed += (MantleAcceleration * InDeltaTime);
	Speed = FMath::Min(Speed, MantleMaxSpeed);

	// Move directly towards the mantle target to avoid oscillating about it
	Velocity = (ToMantle * Speed);

	// Perform move
	FHitResult Hit;
	SafeMoveUpdatedComponent((Velocity * InDeltaTime), UpdatedComponent->GetComponentRotation(), false, Hit);

	const FVector NewLocation = UpdatedComponent->GetComponentLocation();

	// Check if we've reached the mantle target
	const float MantleCompletionTolerance = 10.0f;
	if (FVector::DistSquared(NewLocation, MantleInfo.MantleTargetLocation) <= FMath::Square(MantleCompletionTolerance))
	{
		// Dismount from the mantle
		Velocity = (Owner->GetActorForwardVector() * MantleDismountSpeed);

		MantleInfo = FQHCharacterMovementMantleInfo();

		// Transition to ground movement

		FindFloor(NewLocation, CurrentFloor, false);

		SetDefaultMovementMode();
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::TickComponent(const float InDeltaTime, const ELevelTick InTickType, FActorComponentTickFunction *InTickFunction)
{
	Super::TickComponent(InDeltaTime, InTickType, InTickFunction);

	// Debug draw capsule;
	// TODO: Move character capsule debug drawing to `AQHCharacter`
	{
		const float DebugDrawCapsuleDuration = CVar_QHCharacterCapsuleDebugDraw.GetValueOnGameThread();

		if (DebugDrawCapsuleDuration >= 0.0f)
		{
			ACharacter* Owner = GetCharacterOwner();
			UCapsuleComponent* CharacterCapsuleComponent = Owner->GetCapsuleComponent();
			const FVector CharacterLocation = CharacterCapsuleComponent->GetComponentLocation();

#if UE_ENABLE_DEBUG_DRAWING
			DrawDebugCapsule(GetWorld(), CharacterCapsuleComponent->GetComponentLocation(), CharacterCapsuleComponent->GetScaledCapsuleHalfHeight(), CharacterCapsuleComponent->GetScaledCapsuleRadius(), Owner->GetActorQuat(), FColor::Yellow, false, DebugDrawCapsuleDuration);
			DrawDebugDirectionalArrow(GetWorld(), CharacterLocation, (CharacterLocation + (CharacterCapsuleComponent->GetForwardVector() * 100.0f)), 20.0f, FColor::Red, false, DebugDrawCapsuleDuration);
#endif

		}
	}
}

////////////////////////////////////////////////////////////////////////////////
float UQHCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();

	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		switch (EQHCharacterMovementMode(CustomMovementMode))
		{
			// Climbing
			case EQHCharacterMovementMode::Climbing:
			{
				Result = ClimbMaxSpeed;
			}

			break;

			// Mantling
			case EQHCharacterMovementMode::Mantling:
			{
				Result = MantleMaxSpeed;
			}

			break;
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
float UQHCharacterMovementComponent::GetMaxAcceleration() const
{
	float Result = Super::GetMaxAcceleration();

	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		switch (EQHCharacterMovementMode(CustomMovementMode))
		{
			// Climbing
			case EQHCharacterMovementMode::Climbing:
			{
				Result = ClimbAcceleration;
			}

			break;

			// Mantling
			case EQHCharacterMovementMode::Mantling:
			{
				Result = MantleAcceleration;
			}

			break;
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
float UQHCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	float Result = Super::GetMaxBrakingDeceleration();

	if (MovementMode == EMovementMode::MOVE_Custom)
	{
		switch (EQHCharacterMovementMode(CustomMovementMode))
		{
			// Climbing
			case EQHCharacterMovementMode::Climbing:
			{
				Result = ClimbDeceleration;
			}

			break;
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
void UQHCharacterMovementComponent::PhysCustom(const float InDeltaTime, const int32 InIterations)
{
	switch (EQHCharacterMovementMode(CustomMovementMode))
	{
		// Climbing
		case EQHCharacterMovementMode::Climbing:
		{
			PhysClimbing(InDeltaTime, InIterations);
		}

		break;

		// Mantling
		case EQHCharacterMovementMode::Mantling:
		{
			PhysMantling(InDeltaTime, InIterations);
		}

		break;

		// default
		default:
		{
			Super::PhysCustom(InDeltaTime, InIterations);
		}

		break;
	}
}
