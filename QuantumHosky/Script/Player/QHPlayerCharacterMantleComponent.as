//______________________________________________________________________________
class UQHPlayerCharacterMantleComponent
	: UActorComponent
{
	default PrimaryComponentTick.bStartWithTickEnabled = true;

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQHCharacterMovementMantleQuery MantleQuery;
	default MantleQuery.QueryDistance = 10.0f;
	default MantleQuery.QueryRadius = 100.0f;
	default MantleQuery.QueryHeight = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> MantleSound = nullptr;

//==============================================================================
// private:

	private UQHCharacterMovementComponent CharacterMovementComponent = nullptr;

	private int32 CanMantleCounter = 1;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	bool GetCanMantle() const
	{
		return (CanMantleCounter > 0)
			&& CVar_QHCharacterAllowMantle.GetBool();
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetCanMantle(const bool InCanMantle)
	{
		CanMantleCounter += (InCanMantle ? 1 : -1);

		SetComponentTickEnabled(GetCanMantle());
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnMovementModeChanged(ACharacter InCharacter, EMovementMode InMovementModePrevious, uint8 InCustomModePrevious)
	{
		if (CharacterMovementComponent != nullptr)
		{
			// Mantle started feedback
			if ((CharacterMovementComponent.MovementMode == EMovementMode::MOVE_Custom) && (CharacterMovementComponent.CustomMovementMode == uint8(EQHCharacterMovementMode::Mantling)))
			{
				Gameplay::PlaySoundAtLocation(Sound = System::LoadAsset_Blocking(MantleSound),
											  Location = Owner.GetActorLocation(),
											  Rotation = Owner.GetActorRotation(),
											  OwningActor = Owner);

				APlayerController PlayerController = QH::GetPlayerController();

				PlayerController.ClientStartCameraShake(UQHCameraShakePlayerCharacterMantlingHeadbob);
			}
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
		if (GetCanMantle() && (CharacterMovementComponent != nullptr) && !CharacterMovementComponent.IsMovingOnGround()) // Only allow mantle while airborne
		{
			FQHCharacterMovementMantleInfo Info;
			if (CharacterMovementComponent.QueryMantle(Info, MantleQuery))
			{
				const FVector ToMantle = (Info.MantleTargetLocation - CharacterMovementComponent.UpdatedComponent.GetWorldLocation());

				// Start mantling only if moving or inputting towards the mantle target
				if ((ToMantle.DotProduct(CharacterMovementComponent.Velocity) > 0.0f) || (ToMantle.DotProduct(CharacterMovementComponent.GetLastInputVector()) > 0.0f))
				{
					CharacterMovementComponent.StartMantling(Info);
				}
			}
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
class UQHCameraShakePlayerCharacterMantlingHeadbob
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
const FConsoleVariable CVar_QHCharacterAllowMantle("char.move.mantle.allow", true);
