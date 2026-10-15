//______________________________________________________________________________
enum EQHHorseState
{
	None,
	Idle,
	Walking,
	Eating
}

//______________________________________________________________________________
class AQHHorse
	: AActor
{

//==============================================================================
// public:

	//------------------------------------------------------------------------------
	// Components

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Scene4Component = nullptr;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent BodyMeshComponent = nullptr;
	default BodyMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default BodyMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent NeckSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = NeckSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent NeckMeshComponent = nullptr;
	default NeckMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default NeckMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = NeckSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent HeadMeshComponent = nullptr;
	default HeadMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default HeadMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = NeckSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent LeftEarMeshComponent = nullptr;
	default LeftEarMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default LeftEarMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = NeckSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent RightEarMeshComponent = nullptr;
	default RightEarMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default RightEarMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = NeckSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent ManeMeshComponent = nullptr;
	default ManeMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default ManeMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent FrontLeftLegSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = FrontLeftLegSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent FrontLeftLegMeshComponent = nullptr;
	default FrontLeftLegMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default FrontLeftLegMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent FrontRightLegSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = FrontRightLegSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent FrontRightLegMeshComponent = nullptr;
	default FrontRightLegMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default FrontRightLegMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BackLeftLegSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = BackLeftLegSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent BackLeftLegMeshComponent = nullptr;
	default BackLeftLegMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default BackLeftLegMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent BackRightLegSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = BackRightLegSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent BackRightLegMeshComponent = nullptr;
	default BackRightLegMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default BackRightLegMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = Scene4Component, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent TailSceneComponent = nullptr;

	UPROPERTY(DefaultComponent, Attach = TailSceneComponent, VisibleAnywhere, BlueprintReadOnly)
	UMesh4DComponent TailMeshComponent = nullptr;
	default TailMeshComponent.Mesh4D = QH::GetUnitBoxMesh4D();
	default TailMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHSlice4MemberComponent SliceMemberComponent = nullptr;

	//------------------------------------------------------------------------------
	// Properties

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<USoundBase> FootstepSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<USoundBase> EmoteSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSoftObjectPtr<USoundBase> EatingSound;

//==============================================================================
// private:

	//------------------------------------------------------------------------------
	// States

	private EQHHorseState State = EQHHorseState::None;
	private EQHHorseState NextState = EQHHorseState::None;
	private float StateElapsedTime = 0.0f;

	// Idle
	private float IdleDuration = 0.0f;

	// Walking
	private FVector4 WalkingTargetLocation;
	private float WalkingTargetRadius = 0.0f;
	private bool WalkingMoveTo = false;
	private float FootstepTimer = 0.0f;

	// Eating
	private float EatingDuration = 0.0f;
	private UAudioComponent EatingSoundComponent = nullptr;

	//------------------------------------------------------------------------------

	private TArray<UScene4DComponent> AnimationComponents;
	private TArray<FTransform4D> AnimationBindPoseTransforms;

	private float EmoteCooldown = 0.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void MoveTo(const FVector4& InLocation, const float InRadius = 0.0f)
	{
		WalkingTargetLocation = InLocation;
		WalkingTargetLocation.Z = Scene4Component.GetGlobalPosition4D().Z; // TODO: We only support plane motion for now.

		WalkingTargetRadius = InRadius;

		WalkingMoveTo = true;

		NextState = EQHHorseState::Walking;
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private FVector GetLocation3D() const
	{
		// TODO: `UScene4DComponent` does not properly update the 3D transform on slice changes, and so we manually calculate it for now.
		return SliceMemberComponent.OwnerSlice4.Slice4.World4ToSlice3(Scene4Component.GetGlobalPosition4D());
	}

	//------------------------------------------------------------------------------
	// Animation

	////////////////////////////////////////////////////////////////////////////////
	private void ResetToBindPose()
	{
		for (int32 i = 0, end = AnimationComponents.Num(); i < end; ++i)
		{
			AnimationComponents[i].SetLocalTransform4D(AnimationBindPoseTransforms[i]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	private float PeriodicMotion(const float InMinAngle, const float InMaxAngle, const float InSpeed, const float InPhase)
	{
		return Math::GetMappedRangeValueClamped(FVector2D(-1.0f, 1.0f), FVector2D(InMinAngle, InMaxAngle), Math::Sin(InPhase + (GetGameTimeSinceCreation() * InSpeed)));
	}

	////////////////////////////////////////////////////////////////////////////////
	private float CurveMotion(UCurveFloat InCurve, const float InTime, const float InPlayRate = 1.0f)
	{
		float32 MinTime = 0.0f;
		float32 MaxTime = 0.0f;
		InCurve.GetTimeRange(MinTime, MaxTime);

		return InCurve.GetFloatValue((InTime * InPlayRate) % MaxTime);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void RotateAppendageZX(UScene4DComponent InScene4Component, const float InValue)
	{
		FEuler4D Rotation4 = InScene4Component.GetLocalRotationDegrees4D();

		Rotation4.ZX = InValue;

		InScene4Component.SetLocalRotationDegrees4D(Rotation4);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void RotateAppendageXY(UScene4DComponent InScene4Component, const float InValue)
	{
		FEuler4D Rotation4 = InScene4Component.GetLocalRotationDegrees4D();

		Rotation4.XY = InValue;

		InScene4Component.SetLocalRotationDegrees4D(Rotation4);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void TailWagAnimation(const float InSpeed = 1.0f)
	{
		RotateAppendageZX(TailSceneComponent, PeriodicMotion(-50.0f, -70.0f, InSpeed, 0.0f));
		RotateAppendageXY(TailSceneComponent, PeriodicMotion(-10.0f, 10.0f, InSpeed, 0.0f));
	}

	////////////////////////////////////////////////////////////////////////////////
	private void IdleAnimation()
	{
		TailWagAnimation();
	}

	////////////////////////////////////////////////////////////////////////////////
	private void WalkingAnimation()
	{
		const float AnimationSpeed = 3.0f;

		RotateAppendageZX(NeckSceneComponent, PeriodicMotion(15.0f, 30.0f, AnimationSpeed, 0.0f));
		RotateAppendageZX(FrontLeftLegSceneComponent, PeriodicMotion(-30.0f, 30.0f, AnimationSpeed, 0.0f));
		RotateAppendageZX(FrontRightLegSceneComponent, PeriodicMotion(-30.0f, 30.0f, AnimationSpeed, PI));
		RotateAppendageZX(BackLeftLegSceneComponent, PeriodicMotion(-30.0f, 30.0f, AnimationSpeed, PI));
		RotateAppendageZX(BackRightLegSceneComponent, PeriodicMotion(-30.0f, 30.0f, AnimationSpeed, 0.0f));

		TailWagAnimation(AnimationSpeed);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void EatingAnimation()
	{
		RotateAppendageZX(NeckSceneComponent, CurveMotion(CURVE_FLOAT_Horse_Animation_Eating_Neck_Rotation_ZX, StateElapsedTime));

		TailWagAnimation();
	}

	//------------------------------------------------------------------------------
	// Sound

	////////////////////////////////////////////////////////////////////////////////
	private float CalculateSoundVolumeAttenuation() const
	{
		// NOTE: Direct 4D volume attenuation is technically more `correct`, however qualitatively it can result in very loud sounds if the animal is near you in 4D but very far away from your slice, which feels at odds with 4D mesh projection.
		// Instead, scale volume attenuation by a mixture of 3D distance and slice distance, which results in realistic attenuation when the horse is "far away from you" both in 3D-space and in slice-space.

		const float ListenerAttenuationDistance = 4000.0f;
		const float SliceAttenuationDistance = 1000.0f;

		const float ListenerDistance = GetLocation3D().Distance(QH::GetCameraTransform().Location);
		const float SliceDistance = Math::Max(SliceMemberComponent.GetDistanceToOwnerSlice4(), 0.0f);

		const float VolumeMultiplier = Math::Max((1.0f - Math::Square(ListenerDistance / ListenerAttenuationDistance)), 0.0f)
			 						 * Math::Max((1.0f - Math::Pow((SliceDistance / SliceAttenuationDistance), 3.0f)), 0.0f);

		return Math::Pow(VolumeMultiplier, 3.0f);

		// 4D volume attenuation
		/*
		const float AttenuationDistance = 4000.0f;

		const float VolumeDistance = Scene4Component.GetGlobalPosition4D().Distance(QH::GetPlayerCharacter().Scene4Component.GetTransform4().Origin);

		return Math::Max((1.0f - Math::Square(VolumeDistance / AttenuationDistance)), 0.0f);
		*/
	}

	////////////////////////////////////////////////////////////////////////////////
	private UAudioComponent PlaySound(USoundBase InSound)
	{
		// TODO: Play sounds through attached audio component once `UScene4DComponent` properly updates 3D scene hierarchy on slice changes.

		return Gameplay::SpawnSoundAtLocation(InSound, GetLocation3D(), FRotator::ZeroRotator, VolumeMultiplier = CalculateSoundVolumeAttenuation());
	}

	//------------------------------------------------------------------------------

	////////////////////////////////////////////////////////////////////////////////
	private void UpdateEmote(const float InDeltaTime)
	{
		EmoteCooldown -= InDeltaTime;

		if (EmoteCooldown <= 0.0f)
		{
			EmoteCooldown = Math::RandRange(10.0f, 20.0f);

			// Roll the dice to play the emote
			if (Math::RandBool())
			{
				PlaySound(System::LoadAsset_Blocking(EmoteSound));
			}
		}
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// Initialize animation
		{
			AnimationComponents.Add(NeckSceneComponent);
			AnimationComponents.Add(FrontLeftLegSceneComponent);
			AnimationComponents.Add(FrontRightLegSceneComponent);
			AnimationComponents.Add(BackLeftLegSceneComponent);
			AnimationComponents.Add(BackRightLegSceneComponent);

			for (UScene4DComponent AnimationComponent : AnimationComponents)
			{
				AnimationBindPoseTransforms.Add(AnimationComponent.GetLocalTransform4D());
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Handle state transition
		if ((State != NextState) && (NextState != EQHHorseState::None))
		{
			// End old state
			switch (State)
			{
				// None
				case EQHHorseState::None:
				{
				}

				break;

				// Idle
				case EQHHorseState::Idle:
				{
				}

				break;

				// Walking
				case EQHHorseState::Walking:
				{
				}

				break;

				// Eating
				case EQHHorseState::Eating:
				{
					if (EatingSoundComponent != nullptr)
					{
						EatingSoundComponent.Stop();
					}

					EatingSoundComponent = nullptr;
				}

				break;
			}

			// Transition state
			{
				State = NextState;
				NextState = EQHHorseState::None;
				StateElapsedTime = 0.0f;
			}

			// Begin new state
			switch (State)
			{
				// None
				case EQHHorseState::None:
				{
				}

				break;

				// Idle
				case EQHHorseState::Idle:
				{
					IdleDuration = Math::RandRange(2.0f, 6.0f);

					ResetToBindPose();
				}

				break;

				// Walking
				case EQHHorseState::Walking:
				{
					// Choose random location to wander to if not moving to a specific location
					if (!WalkingMoveTo)
					{
						WalkingTargetLocation = Scene4Component.GetGlobalPosition4D();

						const float WanderRange = 1500.0f;
						const FVector WanderVector = Math::RandomPointInBoundingBox(FVector::ZeroVector, FVector(WanderRange));
						WalkingTargetLocation += FVector4(WanderVector.X, WanderVector.Y, 0.0f, WanderVector.Z);

						WalkingTargetRadius = 0.0f; // Move directly to the wander location
					}

					WalkingMoveTo = false;

					ResetToBindPose();
				}

				break;

				// Eating
				case EQHHorseState::Eating:
				{
					EatingDuration = Math::RandRange(6.0f, 9.0f);

					EatingSoundComponent = PlaySound(System::LoadAsset_Blocking(EatingSound));

					ResetToBindPose();
				}

				break;
			}
		}

		// Update state
		switch (State)
		{
			// None
			case EQHHorseState::None:
			{
				// Go to idle if state unset
				NextState = EQHHorseState::Idle;
			}

			break;

			// Idle
			case EQHHorseState::Idle:
			{
				IdleAnimation();

				UpdateEmote(InDeltaTime);

				if (StateElapsedTime >= IdleDuration)
				{
					// Randomly walk or eat, with a preference for walking
					const float WalkingChance = 0.75f;
					NextState = (Math::RandRange(0.0f, 1.0f) <= WalkingChance)
							  ? EQHHorseState::Walking
							  : EQHHorseState::Eating;
				}
			}

			break;

			// Walking
			case EQHHorseState::Walking:
			{
				WalkingAnimation();

				UpdateEmote(InDeltaTime);

				// Update footsteps
				{
					FootstepTimer -= InDeltaTime;

					if (FootstepTimer <= 0.0f)
					{
						const float FootstepCooldown = 0.75f;
						FootstepTimer = FootstepCooldown;

						PlaySound(System::LoadAsset_Blocking(FootstepSound));
					}
				}

				FVector4 Location = Scene4Component.GetGlobalPosition4D();

				// Move toward the target location
				{
					const FVector4 ToTarget = (WalkingTargetLocation - Location).GetSafeNormal();

					const float WalkSpeed = 300.0f;
					Location += (ToTarget * WalkSpeed * InDeltaTime);

					Scene4Component.SetGlobalPosition4D(Location);
				}

				// Return to idle after reaching the target location
				const float MinWalkingTargetRadius = 10.0f;
				if (Location.DistanceSquared(WalkingTargetLocation) <= Math::Square(Math::Max(WalkingTargetRadius, MinWalkingTargetRadius)))
				{
					NextState = EQHHorseState::Idle;
				}
				else
				{
					// Face towards the target location
					Scene4Component.LookAt(WalkingTargetLocation, FVector4(FVector::UpVector, 0.0f));
				}
			}

			break;

			// Eating
			case EQHHorseState::Eating:
			{
				EatingAnimation();

				// Return to idle after eating
				if (StateElapsedTime >= EatingDuration)
				{
					NextState = EQHHorseState::Idle;
				}
			}

			break;
		}

		StateElapsedTime += InDeltaTime;
	}
}

//______________________________________________________________________________
asset CURVE_FLOAT_Horse_Animation_Eating_Neck_Rotation_ZX of UCurveFloat
{
	/*
	    ------------------------------------------------------------------
	115.|              '''''                          '''''              |
	    |             '     '                        '     '             |
	    |            '       '                      '       '            |
	    |           '                                        '           |
	    |          '          '                    '          '          |
	    |                      '                  '                      |
	    |         '                                            '         |
	    |        '              '                '              '        |
	    |                        '              '                        |
	    |       '                                                '       |
	    |      '                  '            '                  '      |
	    |                          '          '                          |
	    |     '                     '        '                     '     |
	    |    '                       '      '                       '    |
	    |   '                         '    '                         '   |
	110.|'''                           ''''                           '''|
	    ------------------------------------------------------------------
	    0.0                                                            4.0
	*/
	AddAutoCurveKey(0.0, 110.0);
	AddAutoCurveKey(1.0, 115.0);
	AddAutoCurveKey(2.0, 110.0);
	AddAutoCurveKey(3.0, 115.0);
	AddAutoCurveKey(4.0, 110.0);
	PreInfinityExtrap = ERichCurveExtrapolation::RCCE_Cycle;
	PostInfinityExtrap = ERichCurveExtrapolation::RCCE_Cycle;
}

//______________________________________________________________________________
UCLASS(meta = (DisplayName = "Whistle"))
class UQHWhistleItemComponent
	: UQHItemComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> WhistleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WhistleDistance = 5000.0f;

//==============================================================================
// protected:

	//~ UQHItemComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected TOptional<FQHItemComponentUsageResult> OnUse(FQHItemUsageInfo InInfo, bool InTest)
	{
		if (!InTest)
		{
			// Play whistle sound
			Gameplay::PlaySoundAtLocation(System::LoadAsset_Blocking(WhistleSound), InInfo.Instigator.GetActorLocation(), InInfo.Instigator.GetActorRotation(), OwningActor = InInfo.Instigator);

			// Call all animals within range to instigator;
			// TODO: Expand to all animals once we add more!

			TArray<AQHHorse> Horses;
			GetAllActorsOfClass(Horses);

			const FVector4 InstigatorLocation4 = QHGame::GetActorLocation4(InInfo.Instigator);

			for (AQHHorse Horse : Horses)
			{
				if (Horse.Scene4Component.GetGlobalPosition4D().DistanceSquared(InstigatorLocation4) <= Math::Square(WhistleDistance))
				{
					const float MoveToRadius = 300.0f;
					Horse.MoveTo(InstigatorLocation4, InRadius = MoveToRadius);
				}
			}
		}

		return FQHItemComponentUsageResult(EQHItemUsageResultType::Success);
	}

	//~ UQHItemComponent END
}
