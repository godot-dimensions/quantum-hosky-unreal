//______________________________________________________________________________
class UQHPlayerCharacterClimbComponent
	: UActorComponent
{
	default PrimaryComponentTick.bStartWithTickEnabled = true;

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQHCharacterMovementClimbQuery ClimbQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClimbJumpOffSpeedHorizontal = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClimbJumpOffSpeedVertical = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> ClimbSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ClimbFeedbackCooldown = 0.5f;

//==============================================================================
// private:

	private UQHCharacterMovementComponent CharacterMovementComponent = nullptr;

	private int32 CanClimbCounter = 1;

	private uint32 Input_PlayerCharacterClimbStop;

	private float ClimbFeedbackTimer = 0.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	bool GetCanClimb() const
	{
		return (CanClimbCounter > 0);
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetCanClimb(const bool InCanClimb)
	{
		CanClimbCounter += (InCanClimb ? 1 : -1);

		SetComponentTickEnabled(GetCanClimb());
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void PlayClimbFeedback(const TSubclassOf<ULegacyCameraShake>& InCameraShake)
	{
		Gameplay::PlaySoundAtLocation(Sound = System::LoadAsset_Blocking(ClimbSound),
									  Location = Owner.GetActorLocation(),
									  Rotation = Owner.GetActorRotation(),
									  OwningActor = Owner);

		APlayerController PlayerController = QH::GetPlayerController();

		PlayerController.ClientStartCameraShake(InCameraShake);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnMovementModeChanged(ACharacter InCharacter, EMovementMode InMovementModePrevious, uint8 InCustomModePrevious)
	{
		if (CharacterMovementComponent != nullptr)
		{
			// Climb started
			if ((CharacterMovementComponent.MovementMode == EMovementMode::MOVE_Custom) && (CharacterMovementComponent.CustomMovementMode == uint8(EQHCharacterMovementMode::Climbing)))
			{
				// Setup input
				{
					UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

					if (InputSystem != nullptr)
					{
						InputSystem.AddMappingContext(INPUT_CTX_PlayerCharacterClimb, 1, FModifyContextOptions()); // Higher priority to override default input
					}

					UEnhancedInputComponent InputComponent = QH::GetInputComponent();

					if (InputComponent != nullptr)
					{
						Input_PlayerCharacterClimbStop = InputComponent.BindAction(INPUT_ACT_PlayerCharacterClimbStop, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerCharacterClimbStop")).GetHandle();
					}
				}

				// Climb started feedback
				{
					PlayClimbFeedback(UQHCameraShakePlayerCharacterClimbingStartHeadbob);

					ClimbFeedbackTimer = 0.0f;
				}
			}
			// Climb ended
			else if ((InMovementModePrevious == EMovementMode::MOVE_Custom) && (InCustomModePrevious == uint8(EQHCharacterMovementMode::Climbing)))
			{
				// Teardown input
				{
					UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

					if (InputSystem != nullptr)
					{
						InputSystem.RemoveMappingContext(INPUT_CTX_PlayerCharacterClimb, FModifyContextOptions());
					}

					UEnhancedInputComponent InputComponent = QH::GetInputComponent();

					if (InputComponent != nullptr)
					{
						InputComponent.RemoveBindingByHandle(Input_PlayerCharacterClimbStop);
					}
				}
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerCharacterClimbStop(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		if ((CharacterMovementComponent != nullptr) && CharacterMovementComponent.IsClimbing())
		{
			const FQHCharacterMovementClimbInfo ClimbInfo = CharacterMovementComponent.GetClimbInfo();

			// Detach from the climb surface
			CharacterMovementComponent.StopClimbing();

			if (CharacterMovementComponent.IsFalling())
			{
				// If facing away when detaching, jump-off climb surface

				const FVector CharacterForward = Owner.GetActorForwardVector();

				if (CharacterForward.DotProduct(ClimbInfo.ClimbSurfaceNormal) > 0)
				{
					CharacterMovementComponent.Velocity += (CharacterForward * ClimbJumpOffSpeedHorizontal);
					CharacterMovementComponent.Velocity.Z += ClimbJumpOffSpeedVertical;
				}
			}

			PlayClimbFeedback(UQHCameraShakePlayerCharacterClimbingEndHeadbob);
		}
	}

//==============================================================================
// protected:

	//~ UActorComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		CharacterMovementComponent = UQHCharacterMovementComponent::Get(Owner);

		ACharacter CharacterOwner = Cast<ACharacter>(Owner);
		CharacterOwner.MovementModeChangedDelegate.AddUFunction(this, n"OnMovementModeChanged");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		ACharacter CharacterOwner = Cast<ACharacter>(Owner);
		CharacterOwner.MovementModeChangedDelegate.UnbindObject(this);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		if (CharacterMovementComponent == nullptr)
		{
			return;
		}

		if (!CVar_QHCharacterClimbEnabled.GetBool())
		{
			return;
		}

		if (CharacterMovementComponent.IsClimbing())
		{
			// Climb feeback
			if (!CharacterMovementComponent.GetLastInputVector().IsNearlyZero())
			{
				ClimbFeedbackTimer += InDeltaTime;

				if (ClimbFeedbackTimer >= ClimbFeedbackCooldown)
				{
					PlayClimbFeedback(UQHCameraShakePlayerCharacterClimbingHeadbob);

					ClimbFeedbackTimer = 0.0f;
				}
			}
		}
		else if (GetCanClimb() && CharacterMovementComponent.IsFalling()) // Only allow climb while airborne
		{
			FQHCharacterMovementClimbQuery Query = ClimbQuery;
			Query.QueryDirection = CharacterMovementComponent.Owner.GetActorForwardVector();

			FQHCharacterMovementClimbInfo Info;
			if (CharacterMovementComponent.QueryClimb(Info, Query))
			{
				const FVector ToClimb = (Info.ClimbSurfaceLocation - CharacterMovementComponent.UpdatedComponent.GetWorldLocation());

				// Start climbing only if moving or inputting towards the climb surface
				if ((ToClimb.DotProduct(CharacterMovementComponent.Velocity) > 0.0f) || (ToClimb.DotProduct(CharacterMovementComponent.GetLastInputVector()) > 0.0f))
				{
					CharacterMovementComponent.StartClimbing(ClimbQuery, Info);
				}
			}
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
asset INPUT_CTX_PlayerCharacterClimb of UInputMappingContext
{
	Initialize();

	// PlayerCharacterClimbStop
	{
		// Spacebar
		MapKey(INPUT_ACT_PlayerCharacterClimbStop, EKeys::SpaceBar);

		// Gamepad bottom face-button
		MapKey(INPUT_ACT_PlayerCharacterClimbStop, EKeys::Gamepad_FaceButton_Bottom);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerCharacterClimbStop of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
class UQHCameraShakePlayerCharacterClimbingHeadbob
	: ULegacyCameraShake
{
	default OscillationDuration = 0.5f;
	default OscillationBlendInTime = 0.2f;
	default OscillationBlendOutTime = 0.3f;

	default RotOscillation.Pitch.Amplitude = 0.4f;
	default RotOscillation.Pitch.Frequency = 2.0f;

	default RotOscillation.Yaw.Amplitude = 0.0f;
	default RotOscillation.Yaw.Frequency = 0.0f;

	default RotOscillation.Roll.Amplitude = 0.1f;
	default RotOscillation.Roll.Frequency = 2.0f;

	default bSingleInstance = true;
}

//______________________________________________________________________________
class UQHCameraShakePlayerCharacterClimbingStartHeadbob
	: ULegacyCameraShake
{
	default OscillationDuration = 0.5f;
	default OscillationBlendInTime = 0.05f;
	default OscillationBlendOutTime = 0.3f;

	default RotOscillation.Pitch.Amplitude = 2.0f;
	default RotOscillation.Pitch.Frequency = 2.0f;

	default RotOscillation.Yaw.Amplitude = 0.0f;
	default RotOscillation.Yaw.Frequency = 0.0f;

	default RotOscillation.Roll.Amplitude = 1.0f;
	default RotOscillation.Roll.Frequency = 2.0f;

	default bSingleInstance = true;
}

//______________________________________________________________________________
class UQHCameraShakePlayerCharacterClimbingEndHeadbob
	: ULegacyCameraShake
{
	default OscillationDuration = 0.5f;
	default OscillationBlendInTime = 0.05f;
	default OscillationBlendOutTime = 0.3f;

	default RotOscillation.Pitch.Amplitude = 2.0f;
	default RotOscillation.Pitch.Frequency = 2.0f;

	default RotOscillation.Yaw.Amplitude = 0.0f;
	default RotOscillation.Yaw.Frequency = 0.0f;

	default RotOscillation.Roll.Amplitude = 1.0f;
	default RotOscillation.Roll.Frequency = 2.0f;

	default bSingleInstance = true;
}

const FConsoleVariable CVar_QHCharacterClimbEnabled("char.move.climb.on", false);
