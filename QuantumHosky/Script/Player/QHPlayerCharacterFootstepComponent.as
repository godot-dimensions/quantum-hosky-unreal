//______________________________________________________________________________
class UQHPlayerCharacterFootstepComponent
	: UActorComponent
{
	default PrimaryComponentTick.bStartWithTickEnabled = true;

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FootstepCooldown = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> FootstepSound = nullptr;

//==============================================================================
// private:

	private float FootstepTimer = 0.0f;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void TriggerFootstep()
	{
		Gameplay::PlaySoundAtLocation(Sound = System::LoadAsset_Blocking(FootstepSound),
									  Location = Owner.GetActorLocation(),
									  Rotation = Owner.GetActorRotation(),
									  OwningActor = Owner);

		FootstepTimer = 0.0f;
	}

	////////////////////////////////////////////////////////////////////////////////
	private void TriggerHeadbob(const TSubclassOf<UCameraShakeBase>& InHeadbobCameraShake)
	{
		APlayerController PlayerController = QH::GetPlayerController();

		PlayerController.ClientStartCameraShake(InHeadbobCameraShake);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnCharacterLanded(const FHitResult&in Hit)
	{
		TriggerFootstep();
		TriggerHeadbob(UQHCameraShakePlayerCharacterLandedHeadbob);
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		ACharacter Character = Cast<ACharacter>(Owner);

		Character.LandedDelegate.AddUFunction(this, n"OnCharacterLanded");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason EndPlayReason)
	{
		ACharacter Character = Cast<ACharacter>(Owner);

		Character.LandedDelegate.UnbindObject(this);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(float DeltaSeconds)
	{
		ACharacter Character = Cast<ACharacter>(Owner);

		if (Character.CharacterMovement.IsMovingOnGround() && !Character.GetLastMovementInputVector().IsNearlyZero())
		{
			FootstepTimer += DeltaSeconds;

			if (FootstepTimer >= FootstepCooldown)
			{
				TriggerFootstep();
				TriggerHeadbob(UQHCameraShakePlayerCharacterWalkingHeadbob);
			}
		}
	}
}

//______________________________________________________________________________
class UQHCameraShakePlayerCharacterWalkingHeadbob
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
class UQHCameraShakePlayerCharacterLandedHeadbob
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
