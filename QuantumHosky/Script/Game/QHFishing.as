//______________________________________________________________________________
enum EQHFishingBobberState
{
	None,
	Casting,
	Floating,
	Bitten
}

//______________________________________________________________________________
UCLASS(Abstract)
class AQHFishingBobber
	: AActor
{
	// NOTE: Tick as late as possible to render fishing line.
	default PrimaryActorTick.TickGroup = ETickingGroup::TG_PostUpdateWork;
	default PrimaryActorTick.EndTickGroup = ETickingGroup::TG_PostUpdateWork;

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BobberRootComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = BobberRootComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent BobberMeshComponent = nullptr;
	default BobberMeshComponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BobberReelInSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BobberLandInWaterSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> BobberLandInWaterVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BobberLostSound;

	//------------------------------------------------------------------------------

	UQHItemInst OwnerItem = nullptr;
	FVector4 BobberVelocity;

//==============================================================================
// private:

	private EQHFishingBobberState State = EQHFishingBobberState::None;
	private EQHFishingBobberState NextState = EQHFishingBobberState::None;
	private float StateElapsedTime = 0.0f;

	private TArray<AQHWaterVolume> WaterVolumes;

	private FVector4 FloatingPosition = FVector4::Zero;

	private AQHFishingPool BittenFishingPool = nullptr;
	private FVector4 BittenPosition = FVector4::Zero;
	private float BittenShakeTimer = 0.0f;
	private float BittenReelInTimer = 0.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void ReelIn()
	{
		switch (State)
		{
			// Bitten
			case EQHFishingBobberState::Bitten:
			{
				if (BittenFishingPool != nullptr)
				{
					BittenFishingPool.NotifyFishCaught(this);
				}
			}

			break;

			// default
			default:
			{
				Gameplay::PlaySound2D(System::LoadAsset_Blocking(BobberReelInSound));
			}
		}

		// Destroy bobber
		DestroyActor();
	}

	////////////////////////////////////////////////////////////////////////////////
	void NotifyBitten(AQHFishingPool InFishingPool)
	{
		NextState = EQHFishingBobberState::Bitten;

		BittenFishingPool = InFishingPool;
	}

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		const float InstigatorDistance = BobberRootComponent.GetGlobalPosition4D().DistanceSquared(QHGame::GetActorLocation4(Instigator));
		const float MaxBobberDistance = 3000.0f;
		if (InstigatorDistance > Math::Square(MaxBobberDistance))
		{
			NotifyBobberLost();
		}

		UQHASInventoryComponent InventoryComponent = Cast<UQHASInventoryComponent>(OwnerItem.GetInventory());

		if (InventoryComponent.GetHeldItem() == OwnerItem)
		{
			UStaticMeshComponent HeldItemMesh = InventoryComponent.GetHeldItemMesh();

			if (HeldItemMesh != nullptr)
			{
				// Render fishing line;
				// TODO: Add support for 4D debug primitive rendering.
				QHGame::DrawLine(QHGame::Position4DTo3D(BobberRootComponent.GetGlobalPosition4D()), HeldItemMesh.GetSocketLocation(n"FishingLine"), FColor::White, InThickness = 0.5f);
			}
		}
		else
		{
			// Destroy bobber when changing items;
			// TODO: Add support for OnHeld/OnUnheld item events via custom `UQHASItemInst` / `UQHASItemComponent` sub-classes.
			DestroyActor();
		}

		// Transition states
		if ((State != NextState) && (NextState != EQHFishingBobberState::None))
		{
			State = NextState;
			NextState = EQHFishingBobberState::None;
			StateElapsedTime = 0.0f;

			switch (State)
			{
				// Casting
				case EQHFishingBobberState::Casting:
				{
					// Cache water volumes
					GetAllActorsOfClass(WaterVolumes);
				}

				break;

				// Floating
				case EQHFishingBobberState::Floating:
				{
					const FVector4 Position = BobberRootComponent.GetGlobalPosition4D();

					FloatingPosition = Position;

					// Detect collision with fishing pools

					TArray<AQHFishingPool> FishingPools;
					GetAllActorsOfClass(FishingPools);

					for (AQHFishingPool FishingPool : FishingPools)
					{
						if (FishingPool.FishingPoolCollisionComponent.HasWorldPoint(Position))
						{
							FishingPool.NotifyBobberEntered(this);
						}
					}

					// Play FX
					const FVector FXLocation = GetActorLocation();
					Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(BobberLandInWaterSound), FXLocation, FRotator::ZeroRotator, VolumeMultiplier = QH::CalculateSoundVolumeAttenuation4D(Position, InAttenuationDistance3D = MaxBobberDistance));
					Niagara::SpawnSystemAtLocation(System::LoadAsset_Blocking(BobberLandInWaterVFX), FXLocation);
				}

				break;

				// Bitten
				case EQHFishingBobberState::Bitten:
				{
					BittenPosition = BobberRootComponent.GetGlobalPosition4D();
					BittenShakeTimer = 0.0f;
					BittenReelInTimer = 0.0f;
				}

				break;

				// default
				default:
				{
				}

				break;
			}
		}

		// Update state
		switch (State)
		{
			// None
			case EQHFishingBobberState::None:
			{
				NextState = EQHFishingBobberState::Casting;
			}

			break;

			// Casting
			case EQHFishingBobberState::Casting:
			{
				FVector4 Position = BobberRootComponent.GetGlobalPosition4D();

				// Casting physics
				{
					// Integrate velocity
					const FVector4 BobberGravity = FVector4(0.0f, 0.0f, (-980.0f * 2.0f), 0.0f);
					BobberVelocity += (BobberGravity * InDeltaTime);

					// Integrate position
					Position += (BobberVelocity * InDeltaTime);

					BobberRootComponent.SetGlobalPosition4D(Position);
				}

				// Detect fallen out of world
				const float BobberKillZ = -500.0f;
				if (Position.Z < BobberKillZ)
				{
					DestroyActor();
				}

				// Detect collision with world
				if (IsValid(this))
				{
					TArray<FCollisionManifold4D> Collisions;
					UPhysicsServer4DWorldSubsystem PhysicsServer = UPhysicsServer4DWorldSubsystem::Get();
					if (PhysicsServer != nullptr && PhysicsServer.QueryPointCollision(Position, Collisions, true))
					{
						DestroyActor();
					}
				}

				// Detect collision with water
				if (IsValid(this))
				{
					for (AQHWaterVolume WaterVolume : WaterVolumes)
					{
						if (WaterVolume.WaterCollisionComponent.HasWorldPoint(Position))
						{
							NextState = EQHFishingBobberState::Floating;

							break;
						}
					}
				}
			}

			break;

			// Floating
			case EQHFishingBobberState::Floating:
			{
				// Bobbing motion

				FVector4 Position = FloatingPosition;

				const float BobAmplitude = 10.0f;
				const float BobFrequency = 3.0f;
				Position.Z += (Math::Sin(StateElapsedTime / BobFrequency * TWO_PI) * BobAmplitude);

				BobberRootComponent.SetGlobalPosition4D(Position);
			}

			break;

			// Bitten
			case EQHFishingBobberState::Bitten:
			{
				// Shaking motion

				BittenShakeTimer += InDeltaTime;

				const float ShakeDuration = 0.01f;
				if (BittenShakeTimer >= ShakeDuration)
				{
					BittenShakeTimer = 0.0f;

					FVector4 Position = BittenPosition;

					const float Shake = 10.0f;
					Position += FVector4(-Shake).RandomInRange(FVector4(Shake));

					BobberRootComponent.SetGlobalPosition4D(Position);
				}

				// If the player does not reel-in fast enough, destroy bobber

				BittenReelInTimer += InDeltaTime;

				const float TimeToReelIn = 1.0f;
				if (BittenReelInTimer >= TimeToReelIn)
				{
					NotifyBobberLost();
				}
			}

			break;

			// default
			default:
			{
			}

			break;
		}

		StateElapsedTime += InDeltaTime;
	}

	//~ AActor END

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void NotifyBobberLost()
	{
		Gameplay::PlaySound2D(System::LoadAsset_Blocking(BobberLostSound));

		DestroyActor();
	}
}

//______________________________________________________________________________
UCLASS(meta = (DisplayName = "Fishing Pole"))
class UQHFishingPoleItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AQHFishingBobber> BobberType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> CastSound;

//==============================================================================
// private:

	private AQHFishingBobber Bobber = nullptr;

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		if (!InTest)
		{
			// If no bobber, cast one
			if (!IsValid(Bobber))
			{
				Bobber = Cast<AQHFishingBobber>(SpawnActor(BobberType, bDeferredSpawn = true));

				if (Bobber != nullptr)
				{
					Bobber.Instigator = Cast<APawn>(InInfo.Instigator);
					Bobber.OwnerItem = Owner;

					// Cast the bobber from the instigator's eye;
					// TODO: This should be relative to the instigator, however we currently don't have a good way of grabbing their 4D eye-location

					const FTransform4D CameraTransform4D = QH::GetCameraTransform4D();
					const FVector4 CameraForward4D = QHGame::Vector3DTo4D(QH::GetCameraTransform().GetRotation().GetForwardVector());

					const float SpawnOffset = 100.0f;
					Bobber.BobberRootComponent.SetGlobalPosition4D(CameraTransform4D.GetOrigin() + (CameraForward4D * SpawnOffset));

					// Cast the bobber forward

					const float CastForwardSpeed = 1500.0f;
					const float CastUpSpeed = 1000.0f;
					Bobber.BobberVelocity = (CameraForward4D * CastForwardSpeed)
										  + (FVector4::Up * CastUpSpeed)
										  + QHGame::Vector3DTo4D(InInfo.Instigator.GetVelocity() * 0.5f); // Inherit instigator's velocity

					FinishSpawningActor(Bobber);

					Gameplay::PlaySound2D(System::LoadAsset_Blocking(CastSound));
				}
			}
			// Else, reel-in current bobber
			else
			{
				Bobber.ReelIn();
			}
		}

		return FQHItemComponentUsageResult(EQHItemUsageResultType::Success);
	}

	//~ UQHItemComponent END
}

struct FQHFishingPoolFishType
{
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AQHFishingPoolFish> FishActorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UQHItemDef> FishItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> FishMaterial;
}

//______________________________________________________________________________
UCLASS(Abstract)
class AQHFishingPool
	: AActor
{
//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent FishingPoolRootComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = FishingPoolRootComponent, VisibleAnywhere, BlueprintReadOnly)
	USphereShape4DComponent FishingPoolCollisionComponent = nullptr;

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQHFishingPoolFishType> FishTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D BiteTime = FVector2D(5.0f, 10.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BiteChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> SplashSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> SplashVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> BiteSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UNiagaraSystem> BiteVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> CaughtSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DestroyOnCaught = true;

//==============================================================================
// private:

	private int32 FishTypeIndex = -1;

	private TArray<AQHFishingPoolFish> FishActors;

	private AQHFishingBobber Bobber = nullptr;
	AQHFishingPoolFish BobberFish = nullptr;

	private float BiteTimer = 0.0f;

	private float SplashTimer = 0.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void NotifyBobberEntered(AQHFishingBobber InBobber)
	{
		if (!IsValid(Bobber) && IsValid(InBobber))
		{
			Bobber = InBobber;

			BobberFish = FishActors[Math::RandRange(0, (FishActors.Num() - 1))];
			BobberFish.SetBobber(Bobber);

			BiteTimer = Math::RandRange(BiteTime.X, BiteTime.Y);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void NotifyFishCaught(AQHFishingBobber InBobber)
	{
		if (InBobber != nullptr)
		{
			// Grant item to fisherman

			UQHInventoryComponent BobberInventoryComponent = UQHInventoryComponent::Get(InBobber.GetActorInstigator());

			if (BobberInventoryComponent != nullptr)
			{
				BobberInventoryComponent.AddItem(QHItem::CreateItem(System::LoadAsset_Blocking(FishTypes[FishTypeIndex].FishItem)));
			}

			// TODO: Play fish caught feedback in 4D with proper attenuation based on distance to slice
			Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(CaughtSound), Bobber.GetActorLocation(), FRotator());

			if (DestroyOnCaught)
			{
				DestroyActor();
			}
		}
	}

//==============================================================================
// protected:

	//~ UActorComponent BEGIN

	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// Roll fish type
		FishTypeIndex = Math::RandRange(0, (FishTypes.Num() - 1));

		// Spawn fish

		const TSubclassOf<AQHFishingPoolFish> FishActorTypeClass = System::LoadClassAsset_Blocking(FishTypes[FishTypeIndex].FishActorType);

		if (FishActorTypeClass.IsValid())
		{
			const FVector4 FishSpawnOrigin = FishingPoolCollisionComponent.GetGlobalPosition4D();
			const float FishSpawnRadius = (FishingPoolCollisionComponent.GetRadiusWorld() * (2.0f / 3.0f));

			for (int32 i = 0, end = Math::RandRange(5, 9); i < end; ++i)
			{
				FVector4 FishSpawnLocationOffset = UVectorFuncs4D::RandomInRadius(FishSpawnRadius);
				FishSpawnLocationOffset.Z = -Math::Abs(FishSpawnLocationOffset.Z); // Offset fish below waterline

				FEuler4D FishSpawnRotation;
				FishSpawnRotation.XY = Math::RandRange(0.0f, TWO_PI);

				AQHFishingPoolFish FishActor = SpawnActor(FishActorTypeClass, bDeferredSpawn = true);
				FishActor.Root4DComponent.SetGlobalPosition4D(FishSpawnOrigin + FishSpawnLocationOffset);
				FishActor.Root4DComponent.SetGlobalRotationRadians4D(FishSpawnRotation);
				FishActor.Root4DComponent.SetGlobalUniformScale4D(Math::RandRange(1.0f, 3.0f));
				FishActor.BodyMeshComponent.SetMaterial(0, System::LoadAsset_Blocking(FishTypes[FishTypeIndex].FishMaterial));
				FishActor.SpawnOrigin = FishSpawnOrigin;
				FishActor.MoveRadius = FishSpawnRadius;

				FinishSpawningActor(FishActor);

				FishActors.Add(FishActor);
			}
		}
	}

	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason InReason)
	{
		for (AActor FishActor : FishActors)
		{
			FishActor.DestroyActor();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		if (IsValid(Bobber))
		{
			// Roll for bite every X seconds

			BiteTimer -= InDeltaTime;

			if (BiteTimer <= 0.0f)
			{
				BiteTimer = Math::RandRange(BiteTime.X, BiteTime.Y);

				if (Math::RandRange(0.0f, 1.0f) <= BiteChance)
				{
					Bobber.NotifyBitten(this);

					// TODO: Play fish bite feedback in 4D with proper attenuation based on distance to slice
					Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(BiteSound), Bobber.GetActorLocation(), FRotator());
					Niagara::SpawnSystemAtLocation(System::LoadAsset_Blocking(BiteVFX), Bobber.GetActorLocation());
				}
			}
		}
		else
		{
			// Play fish splash FX
			{
				SplashTimer -= InDeltaTime;

				if (SplashTimer <= 0.0f)
				{
					SplashTimer = Math::RandRange(5.0f, 10.0f);

					const float FishSplashChance = 0.1f;
					if (Math::RandRange(0.0f, 1.0f) < FishSplashChance)
					{
						const FVector4 Location = FishingPoolRootComponent.GetGlobalPosition4D();
						const float SliceDistance = QHGame::SliceDistance4D(Location);

						const float MaxSliceDistance = 1000.0f;
						if (SliceDistance <= MaxSliceDistance)
						{
							const float VolumeAttenuation = QH::CalculateSoundVolumeAttenuation4D(Location, InAttenuationDistance3D = 3000.0f, InAttenuationDistanceSlice = MaxSliceDistance);

							FVector FXLocation = QHGame::Position4DTo3D(Location);
							const FVector2D FXLocationOffset = Math::RandPointInCircle(FishingPoolCollisionComponent.GetRadiusWorld() * 0.5f);
							FXLocation += FVector(FXLocationOffset.X, FXLocationOffset.Y, 0.0f);

							Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(SplashSound), FXLocation, FRotator::ZeroRotator, VolumeMultiplier = VolumeAttenuation);
							Niagara::SpawnSystemAtLocation(System::LoadAsset_Blocking(SplashVFX), FXLocation);
						}
					}
				}
			}

			if (BobberFish != nullptr)
			{
				BobberFish.SetBobber(nullptr);
			}
		}
	}

	//~ UActorComponent END
}

UCLASS(Abstract)
class AQHFishingPoolFish
	: AActor
{
// public:
	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Root4DComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = Root4DComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent BodyMeshComponent = nullptr;
	default BodyMeshComponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	FVector4 SpawnOrigin = FVector4::Zero;
	float MoveRadius = 0.0f;

// private:
	private float TickTimer = 0.0f;
	private float TickFrequency = 0.0f;
	private FEuler4D SpawnRotation;
	private float IdleTimer = 0.0f;
	private FVector4 MoveGoal = FVector4::Zero;
	private bool IsMoving = false;
	private AQHFishingBobber Bobber = nullptr;

// public:
	void SetBobber(AQHFishingBobber InBobber)
	{
		Bobber = InBobber;

		if (Bobber == nullptr)
		{
			IsMoving = false;
		}
	}

// protected:
	// AActor BEGIN
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// Offset initial tick to avoid spiky frames
		TickTimer = Math::RandRange(0.0f, 3.0f);
		TickFrequency = TickTimer;

		SpawnRotation = BodyMeshComponent.GetLocalRotationDegrees4D();
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Manually control ticking rather than leveraging `PrimaryActorTick.TickInterval` to support variable tick frequency to avoid spiky frames
		TickTimer -= InDeltaTime;

		if (TickTimer > 0.0f)
		{
			return;
		}

		const float DeltaTime = (TickFrequency + Math::Abs(TickTimer));

		TickFrequency = Math::RandRange(0.075f, 0.125f);
		TickTimer = TickFrequency;

		// Bobber
		if (Bobber != nullptr)
		{
			const FVector4 BobberLocation = Bobber.BobberRootComponent.GetGlobalPosition4D();

			const float GoalRadius = 30.0f;
			if (!IsAtGoal(BobberLocation, GoalRadius))
			{
				MoveTo(BobberLocation, DeltaTime);
			}

			SwimmingAnimation(8.0f);
		}
		// Moving
		else if (IsMoving)
		{
			MoveTo(MoveGoal, DeltaTime);

			const float GoalRadius = 10.0f;
			if (IsAtGoal(MoveGoal, GoalRadius))
			{
				IdleTimer = Math::RandRange(3.0f, 8.0f);

				IsMoving = false;
			}

			SwimmingAnimation(8.0f);
		}
		// Idle
		else
		{
			IdleTimer -= DeltaTime;

			if (IdleTimer <= 0.0f)
			{
				FVector4 WanderVector = UVectorFuncs4D::RandomInRadius(MoveRadius);
				WanderVector.Z = -Math::RandRange(0.0f, 25.0f); // Do not wander upwards above waterline

				MoveGoal = (SpawnOrigin + WanderVector);

				IsMoving = true;
			}

			SwimmingAnimation(1.0f);
		}
	}
	// AActor END

//private:
	private void MoveTo(const FVector4& InGoal, const float InDeltaTime)
	{
		FVector4 Location = Root4DComponent.GetGlobalPosition4D();
		const FVector4 ToGoal = (InGoal - Location).GetSafeNormal();

		const float MoveSpeed = 100.0f;
		Location += (ToGoal * MoveSpeed * InDeltaTime);

		Root4DComponent.SetGlobalPosition4D(Location);

		FVector4 LookAt = InGoal;
		LookAt.Z = Location.Z; // Maintain horizontal orientation
		Root4DComponent.LookAt(LookAt, FVector4::Up);
	}

	private bool IsAtGoal(const FVector4& InGoal, const float InGoalRadius) const
	{
		return (Root4DComponent.GetGlobalPosition4D().DistanceSquared(InGoal) <= Math::Square(InGoalRadius));
	}

	private void SwimmingAnimation(const float InSpeed)
	{
		FEuler4D Rotation = SpawnRotation;
		Rotation.XY += Math::Sin(Gameplay::GetTimeSeconds() * InSpeed) * 15.0f;

		BodyMeshComponent.SetLocalRotationDegrees4D(Rotation);
	}
}

//______________________________________________________________________________
UCLASS(Abstract)
class AQHAquarium
	: AActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent AquariumRootComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = AquariumRootComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractionVolumeComponent AquariumInteractionVolumeComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHInteractableComponent AquariumInteractableComponent = nullptr;
	default AquariumInteractableComponent.InteractText = NSLOCTEXT("QHGame", "Interact_Aquarium", "Place fish");

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 FishSpawnExtents = FVector4(50.0f);

// protected:
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		AquariumInteractableComponent.OnInteract.AddUFunction(this, n"OnInteract");
		AquariumInteractableComponent.OnCanInteract.BindUFunction(this, n"OnCanInteract");
	}

// private:
	UFUNCTION()
	private void OnInteract(UQHInteractableComponent InInteractable, AActor InInstigator)
	{
		UQHASInventoryComponent InstigatorInventory = UQHASInventoryComponent::Get(InInstigator);

		if (InstigatorInventory != nullptr)
		{
			UQHItemInst HeldItem = InstigatorInventory.GetHeldItem();

			if ((HeldItem != nullptr) && HeldItem.Tags.HasTag(GameplayTags::item_fish))
			{
				FQHItemUsageInfo ItemUsage;
				ItemUsage.Instigator = this;

				HeldItem.Use(ItemUsage);
			}
		}
	}

	UFUNCTION()
	private bool OnCanInteract(const UQHInteractableComponent InInteractable, const AActor InInstigator)
	{
		bool Result = false;

		UQHASInventoryComponent InstigatorInventory = UQHASInventoryComponent::Get(InInstigator);

		if (InstigatorInventory != nullptr)
		{
			UQHItemInst HeldItem = InstigatorInventory.GetHeldItem();

			Result = (HeldItem != nullptr)
				  && HeldItem.Tags.HasTag(GameplayTags::item_fish);
		}

		return Result;
	}
}

//______________________________________________________________________________
class AQHFish
	: AActor
{
// public:
	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Root4DComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = Root4DComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent MeshRootComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = MeshRootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BodyLink0Component = nullptr;

	UPROPERTY(DefaultComponent, Attach = BodyLink0Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BodyLink1Component = nullptr;

	UPROPERTY(DefaultComponent, Attach = BodyLink1Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BodyLink2Component = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> FishMaterial;

	FVector4 MoveBoundsMin = FVector4::Zero;
	FVector4 MoveBoundsMax = FVector4::Zero;

// private:
	private FEuler4D SpawnRotation0;
	private FEuler4D SpawnRotation1;
	private FEuler4D SpawnRotation2;
	private float IdleTimer = 0.0f;
	private FVector4 MoveGoal = FVector4::Zero;
	bool IsMoving = false;

// protected:
	// AActor BEGIN
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		SpawnRotation0 = BodyLink0Component.GetLocalRotationDegrees4D();
		SpawnRotation1 = BodyLink1Component.GetLocalRotationDegrees4D();
		SpawnRotation2 = BodyLink2Component.GetLocalRotationDegrees4D();
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Moving
		if (IsMoving)
		{
			FVector4 Location = Root4DComponent.GetGlobalPosition4D();
			const FVector4 ToGoal = (MoveGoal - Location).GetSafeNormal();

			const float MoveSpeed = 100.0f;
			Location += (ToGoal * MoveSpeed * InDeltaTime);

			Root4DComponent.SetGlobalPosition4D(Location);

			FVector4 LookAt = MoveGoal;
			LookAt.Z = Location.Z; // Maintain horizontal rotation
			Root4DComponent.LookAt(LookAt, FVector4::Up);

			const float GoalRadius = 10.0f;
			if (Location.DistanceSquared(MoveGoal) <= Math::Square(GoalRadius))
			{
				IdleTimer = Math::RandRange(7.0f, 15.0f);

				IsMoving = false;
			}

			SwimmingAnimation(8.0f, false);
		}
		// Idle
		else
		{
			IdleTimer -= InDeltaTime;

			if (IdleTimer <= 0.0f)
			{
				MoveGoal = MoveBoundsMin.RandomInRange(MoveBoundsMax);

				IsMoving = true;
			}

			SwimmingAnimation(1.0f, true);
		}
	}
	// AActor END

//private:
	private void SwimmingAnimation(const float InSpeed, const bool InBob)
	{
		const float Time = Gameplay::GetTimeSeconds();

		if (InBob)
		{
			const FVector4 BobOffset = (FVector4::Up * Math::Sin(Time * InSpeed) * 5.0f);
			MeshRootComponent.SetLocalPosition4D(BobOffset);
		}

		FEuler4D Rotation0 = SpawnRotation0;
		Rotation0.XY += Math::Sin(Time * InSpeed) * 5.0f;

		FEuler4D Rotation1 = SpawnRotation1;
		Rotation1.XY += Math::Cos(Time * InSpeed) * 10.0f;

		FEuler4D Rotation2 = SpawnRotation2;
		Rotation2.XY += Math::Sin(Time * InSpeed + (PI / 4.0f)) * 15.0f;

		BodyLink0Component.SetLocalRotationDegrees4D(Rotation0);
		BodyLink1Component.SetLocalRotationDegrees4D(Rotation1);
		BodyLink2Component.SetLocalRotationDegrees4D(Rotation2);
	}
}

//______________________________________________________________________________
UCLASS(meta = (DisplayName = "Fish"))
class UQHFishItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AQHFish> FishActorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> FishMaterial;

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		TOptional<FQHItemComponentUsageResult> Result;

		AQHAquarium Aquarium = Cast<AQHAquarium>(InInfo.Instigator);

		if (Aquarium != nullptr)
		{
			if (!InTest)
			{
				const TSubclassOf<AQHFish> FishActorTypeClass = System::LoadClassAsset_Blocking(FishActorType);
				AQHFish FishActor = SpawnActor(FishActorTypeClass, bDeferredSpawn = true);

				if (FishActor != nullptr)
				{
					// Spawn fish at random location / rotation inside aquarium

					const FVector4 AquariumLocation = Aquarium.AquariumRootComponent.GetGlobalPosition4D();
					const FVector4 AquariumExtents = (Aquarium.FishSpawnExtents * Aquarium.AquariumRootComponent.GetGlobalScale4D());
					const FVector4 AquariumMins = (AquariumLocation - AquariumExtents);
					const FVector4 AquariumMaxs = (AquariumLocation + AquariumExtents);
					const FVector4 FishSpawnLocation = AquariumMins.RandomInRange(AquariumMaxs);

					FEuler4D FishSpawnRotation;
					FishSpawnRotation.XY = Math::RandRange(0, TWO_PI);
					FishSpawnRotation.XW = Math::RandRange(0, TWO_PI);
					FishSpawnRotation.WY = Math::RandRange(0, TWO_PI);

					FishActor.Root4DComponent.SetGlobalPosition4D(FishSpawnLocation);
					FishActor.Root4DComponent.SetGlobalRotationRadians4D(FishSpawnRotation);
					FishActor.Root4DComponent.SetGlobalUniformScale4D(Math::RandRange(0.5f, 1.1f));
					FishActor.FishMaterial = FishMaterial;
					FishActor.MoveBoundsMin = AquariumMins;
					FishActor.MoveBoundsMax = AquariumMaxs;

					FinishSpawningActor(FishActor);
				}
			}

			Result = EQHItemUsageResultType::Success;
		}

		return Result;
	}

	//~ UQHItemComponent END
}

class UQHFrogFXComponent
	: UScene4DComponent
{
// public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> FrogSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 5000.0f;

// private:
	private float Timer = 0.0f;

// public:
	UFUNCTION(BlueprintOverride)
	void Tick(const float InDeltaTime)
	{
		Timer -= InDeltaTime;

		if (Timer <= 0.0f)
		{
			Timer = Math::RandRange(0.1f, 15.0f);

			FVector4 Location = GetGlobalPosition4D();

			FVector4 LocationOffset = UVectorFuncs4D::RandomInRadius(Radius);
			LocationOffset.Z = 0.0f;

			Location += LocationOffset;

			float VolumeAttenuation = QH::CalculateSoundVolumeAttenuation4D(Location, InAttenuationDistance3D = Radius, InAttenuationDistanceSlice = Radius);
			VolumeAttenuation *= Math::RandRange(0.75f, 1.25f);

			Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(FrogSound), QHGame::Position4DTo3D(Location), FRotator::ZeroRotator, VolumeMultiplier = VolumeAttenuation);
		}
	}
}

UCLASS(Abstract)
class AQHFrogFX
	: AActor
{
// public:
	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHFrogFXComponent FrogFXComponent = nullptr;
}
