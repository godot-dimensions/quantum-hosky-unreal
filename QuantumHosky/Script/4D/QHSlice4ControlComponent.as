//______________________________________________________________________________
event void FQHOnActiveSlice4Changed(UQHSlice4Component InNewActiveSlice4, UQHSlice4Component InOldActiveSlice4);

//______________________________________________________________________________
class UQHSlice4ControlComponent
	: UActorComponent
{
	default PrimaryComponentTick.bStartWithTickEnabled = false;

//==============================================================================
// public:

	UPROPERTY()
	FQHOnActiveSlice4Changed OnActiveSlice4Changed;

//==============================================================================
// private:

	private UQHASSlice4Component ActiveSlice4 = nullptr;

	private TArray<UQHASSlice4Component> ActiveSlice4Stack;

	private uint32 Input_Slice4RotateDigital;
	private uint32 Input_Slice4RotateAxis;
	private uint32 Input_Slice4NextRotationAxis;
	private uint32 Input_Slice4Reset;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void PushActiveSlice4(UQHASSlice4Component InSlice4Component)
	{
		if (InSlice4Component != nullptr)
		{
			ActiveSlice4Stack.Add(InSlice4Component);

			UpdateActiveSlice4();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void PopActiveSlice4(UQHASSlice4Component InSlice4Component)
	{
		if (InSlice4Component != nullptr)
		{
			for (int32 i = (ActiveSlice4Stack.Num() - 1); i >= 0; --i)
			{
				if (ActiveSlice4Stack[i] == InSlice4Component)
				{
					ActiveSlice4Stack.RemoveAt(i);

					UpdateActiveSlice4();

					break;
				}
			}
		}
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void UpdateActiveSlice4() const
	{
		UQHASSlice4Component NewActiveSlice4 = !ActiveSlice4Stack.IsEmpty()
										   	 ? ActiveSlice4Stack.Last()
										   	 : nullptr;

		QH::SetActiveSlice4Component(NewActiveSlice4);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void SetSlice4Origin(FQHSlice4& InSlice4) const
	{
		if ((ActiveSlice4 != nullptr) && ActiveSlice4.Slice4OriginStatic)
		{
			InSlice4.SetOrigin(FVector4(), FVector());
		}
		else
		{
			AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();
			InSlice4.SetOrigin(PlayerCharacter.Scene4Component.GetTransform4().Origin, PlayerCharacter.GetActorLocation());
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnPlayerOwnerSlice4Changed(UQHSlice4MemberComponent InSlice4MemberComponent, UQHSlice4Component InNewOwner, UQHSlice4Component InOldOwner)
	{
		if (ActiveSlice4 != nullptr)
		{
			ActiveSlice4.DeactivateSlice4_Private();
		}

		ActiveSlice4 = Cast<UQHASSlice4Component>(InNewOwner);

		if (ActiveSlice4 != nullptr)
		{
			ActiveSlice4.ActivateSlice4_Private();
		}

		OnActiveSlice4Changed.Broadcast(InNewOwner, InOldOwner);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_Slice4RotateDigital(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		if (ActiveSlice4 == nullptr)
		{
			return;
		}

		if (!QH::IsSliceDigitalRotationEnabled())
		{
			return;
		}

		FQHSlice4 Slice4 = ActiveSlice4.GetSlice4();

		SetSlice4Origin(Slice4);

		FEuler4D Slice4Rotation = Slice4.GetRotation4();

		const float Slice4RotationSpeed = (ActiveSlice4 != nullptr)
										? ActiveSlice4.Slice4RotationSpeed
										: QH::DefaultSlice4RotationSpeed;

		const float Slice4RotationChange = Math::DegreesToRadians(ActionValue.GetAxis1D() * Slice4RotationSpeed * Gameplay::GetWorldDeltaSeconds());

		// The legacy axis enum names the complementary plane; keep its input behavior.
		switch (ActiveSlice4.Slice4RotationAxis)
		{
			// XY
			case EQHSlice4RotationAxis::XY:
			{
				Slice4Rotation.ZW -= Slice4RotationChange;
			}

			break;

			// XZ
			case EQHSlice4RotationAxis::XZ:
			{
				Slice4Rotation.WY += Slice4RotationChange;
			}

			break;

			// YZ
			case EQHSlice4RotationAxis::YZ:
			{
				Slice4Rotation.XW -= Slice4RotationChange;
			}

			break;

			// XW
			case EQHSlice4RotationAxis::XW:
			{
				Slice4Rotation.YZ -= Slice4RotationChange;
			}

			break;

			// YW
			case EQHSlice4RotationAxis::YW:
			{
				Slice4Rotation.ZX += Slice4RotationChange;
			}

			break;

			// ZW
			case EQHSlice4RotationAxis::ZW:
			{
				Slice4Rotation.XY -= Slice4RotationChange;
			}

			break;
		}

		Slice4.SetRotation4(Slice4Rotation);

		ActiveSlice4.SetSlice4(Slice4);
	}

	UFUNCTION()
	private void OnInput_Slice4RotateAxis(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		if (ActiveSlice4 == nullptr)
		{
			return;
		}

		// TODO: Use chorded input to detect slice mouse rotation
		if (!QH::IsSliceRotationInputActive())
		{
			return;
		}

		FQHSlice4 Slice4 = ActiveSlice4.GetSlice4();

		SetSlice4Origin(Slice4);

		const FVector2D Slice4RotationInputs = ActionValue.GetAxis2D();
		// The player rotation and slice rotation are logically part of the same thing, so this has to be taken into account.
		const float PlayerYaw = QH::GetPlayerController().GetControlRotation().Yaw;

		// Preserve the existing mouse-control frame: map 3D Z to 4D W.
		// FRotator uses degrees; the slice Euler angles are now radians.
		FRotator RotationChange =
			FRotator(0, PlayerYaw, 0) // The fact that PlayerYaw goes in the yaw place here is a coincidence.
			.Compose(FRotator(Slice4RotationInputs.Y, 0, Slice4RotationInputs.X)) // The order in which these two rotations are applied doesn't matter. They don't commute, but neither order is more logical.
			.Compose(FRotator(0, -PlayerYaw, 0));

		FEuler4D Slice4Rotation = Slice4.GetRotation4();
		FRotator SliceRotation3D = FRotator(-Math::RadiansToDegrees(Slice4Rotation.XW), -Math::RadiansToDegrees(Slice4Rotation.XY), Math::RadiansToDegrees(Slice4Rotation.WY));
		FRotator NewRotation = RotationChange.Compose(SliceRotation3D);
		Slice4Rotation.XW = -Math::DegreesToRadians(NewRotation.Pitch);
		Slice4Rotation.XY = -Math::DegreesToRadians(NewRotation.Yaw);
		Slice4Rotation.WY = Math::DegreesToRadians(NewRotation.Roll);

		Slice4.SetRotation4(Slice4Rotation);

		ActiveSlice4.SetSlice4(Slice4);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_Slice4NextRotationAxis(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		if (ActiveSlice4 != nullptr)
		{
			EQHSlice4RotationAxis NewRotationAxis = QH::DefaultSlice4RotationAxis;

			if (CVar_QHSliceAllowAnyRotation.GetBool())
			{
				const int32 CurrentRotationAxis = int32(ActiveSlice4.Slice4RotationAxis);

				const int32 NavigationDir = Input::ModifierKeysState_IsShiftDown(Input::GetModifierKeysState())
										? int32(-ActionValue.GetAxis1D())
										: int32(ActionValue.GetAxis1D());

				const int32 NextRotationAxis = Math::WrapIndex((CurrentRotationAxis + NavigationDir), 0, int32(EQHSlice4RotationAxis::MAX));

				NewRotationAxis = EQHSlice4RotationAxis(NextRotationAxis);
			}
			else
			{
				// Only allow rotation in XZ and YZ;
				// don't allow rotation of the up-axis/gravity;
				// rotation in W does not produce a noticeable change to the user when starting from the origin

				switch (ActiveSlice4.Slice4RotationAxis)
				{
					// XZ
					case EQHSlice4RotationAxis::XZ:
					{
						NewRotationAxis = EQHSlice4RotationAxis::YZ;
					}

					break;

					// YZ
					case EQHSlice4RotationAxis::YZ:
					{
						NewRotationAxis = EQHSlice4RotationAxis::XZ;
					}

					break;

					//default
					default:
					{
						NewRotationAxis = QH::DefaultSlice4RotationAxis;
					}

					break;
				}
			}

			ActiveSlice4.Slice4RotationAxis = NewRotationAxis;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_Slice4Reset(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		APlayerController PlayerController = QH::GetPlayerController();

		if (PlayerController.IsInputKeyDown(EKeys::LeftShift))
		{
			if (PlayerController.IsInputKeyDown(EKeys::LeftControl))
			{
				if (ActiveSlice4 != nullptr)
				{
					ActiveSlice4.ResetSlice4();
				}
			}
			else
			{
				QH::GetPlayerCharacter().ResetPlayer();
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
		// Listen for the active slice to change
		{
			UQHSlice4MemberComponent PlayerSlice4MemberComponent = UQHSlice4MemberComponent::Get(QH::GetPlayerCharacter());

			if (PlayerSlice4MemberComponent != nullptr)
			{
				PlayerSlice4MemberComponent.OnOwnerSlice4Changed.AddUFunction(this, n"OnPlayerOwnerSlice4Changed");
			}
		}

		// Initialize default slice
		{
			UQHASSlice4Component DefaultSlice4Component = QH::GetDefaultSlice4Component();

			if (DefaultSlice4Component != nullptr)
			{
				PushActiveSlice4(DefaultSlice4Component);
			}
		}

		// Setup input
		{
			UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

			if (InputSystem != nullptr)
			{
				InputSystem.AddMappingContext(INPUT_CTX_Slice4Control, 0, FModifyContextOptions());
			}

			UEnhancedInputComponent InputComponent = QH::GetInputComponent();

			if (InputComponent != nullptr)
			{
				Input_Slice4RotateDigital = InputComponent.BindAction(INPUT_ACT_Slice4RotateDigital, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_Slice4RotateDigital")).GetHandle();
				Input_Slice4RotateAxis = InputComponent.BindAction(INPUT_ACT_Slice4RotateAxis, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_Slice4RotateAxis")).GetHandle();
				Input_Slice4NextRotationAxis = InputComponent.BindAction(INPUT_ACT_Slice4NextRotationAxis, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_Slice4NextRotationAxis")).GetHandle();
				Input_Slice4Reset = InputComponent.BindAction(INPUT_ACT_Slice4Reset, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_Slice4Reset")).GetHandle();
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason EndPlayReason)
	{
		{
			UQHSlice4MemberComponent PlayerSlice4MemberComponent = UQHSlice4MemberComponent::Get(QH::GetPlayerCharacter());

			if (PlayerSlice4MemberComponent != nullptr)
			{
				PlayerSlice4MemberComponent.OnOwnerSlice4Changed.UnbindObject(this);
			}
		}

		{
			UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

			if (InputSystem != nullptr)
			{
				InputSystem.RemoveMappingContext(INPUT_CTX_Slice4Control, FModifyContextOptions());
			}

			UEnhancedInputComponent InputComponent = QH::GetInputComponent();

			if (InputComponent != nullptr)
			{
				InputComponent.RemoveBindingByHandle(Input_Slice4RotateDigital);
				InputComponent.RemoveBindingByHandle(Input_Slice4RotateAxis);
				InputComponent.RemoveBindingByHandle(Input_Slice4NextRotationAxis);
				InputComponent.RemoveBindingByHandle(Input_Slice4Reset);
			}
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
asset INPUT_CTX_Slice4Control of UInputMappingContext
{
	Initialize();

	// Slice4RotateDigital
	{
		// E
		MapKey(INPUT_ACT_Slice4RotateDigital, EKeys::E);

		// Q
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_Slice4RotateDigital, EKeys::Q);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}

		// Mouse wheel
		/*
		// TODO: Mouse wheel for slice rotation has been disabled as it currently conflicts with item slot selection,
		// and adding modifier key support (e.g. ctrl + mouse wheel) is not straightforward if we additionally want to block any other bound inputs or support rebindable keys.
		// A long-term solution is to modify the `EnhancedInput` plugin to work with `FInputChord` rather than `FKey`, allowing us to treat a key+modifier as a truly separate input event;
		// see https://www.reddit.com/r/unrealengine/comments/1irv0jz/using_advanced_input_there_does_not_seem_to_be_a/
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_Slice4Rotate, EKeys::MouseWheelAxis);

			{
				UInputModifierScalar ModifierScalar = NewObject(this, UInputModifierScalar);
				ModifierScalar.Scalar = FVector(5.0f);

				Mapping.Modifiers.Add(ModifierScalar);
			}
		}
		*/

		// Gamepad right-trigger
		MapKey(INPUT_ACT_Slice4RotateDigital, EKeys::Gamepad_RightTriggerAxis);

		// Gamepad left-trigger
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_Slice4RotateDigital, EKeys::Gamepad_LeftTriggerAxis);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}
	}

	// Slice4RotateAxis
	{
		MapKey(INPUT_ACT_Slice4RotateAxis, EKeys::Mouse2D);
	}

	// Slice4NextRotationAxis
	{
		// Tab
		MapKey(INPUT_ACT_Slice4NextRotationAxis, EKeys::Tab);

		// Gamepad dpad right
		MapKey(INPUT_ACT_Slice4NextRotationAxis, EKeys::Gamepad_DPad_Right);

		// Gamepad dpad left
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_Slice4NextRotationAxis, EKeys::Gamepad_DPad_Left);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}
	}

	// Slice4Reset
	{
		// R
		MapKey(INPUT_ACT_Slice4Reset, EKeys::R);

		// Gamepad right-stick click
		MapKey(INPUT_ACT_Slice4Reset, EKeys::Gamepad_RightThumbstick);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_Slice4RotateDigital of UInputAction
{
	Initialize(EInputActionValueType::Axis1D);

	{
		UInputTriggerDown TriggerDown = NewObject(this, UInputTriggerDown);
		TriggerDown.ActuationThreshold = 0.0f;

		Triggers.Add(TriggerDown);
	}
}

asset INPUT_ACT_Slice4RotateAxis of UInputAction
{
	Initialize(EInputActionValueType::Axis2D);

	bConsumeInput = false;

	{
		UInputTriggerDown TriggerDown = NewObject(this, UInputTriggerDown);
		TriggerDown.ActuationThreshold = 0.0f;

		Triggers.Add(TriggerDown);
	}

	{
		UInputModifierNegate ModifierNegate = NewObject(this, UInputModifierNegate);
		ModifierNegate.bX = true;
		ModifierNegate.bY = true;

		Modifiers.Add(ModifierNegate);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_Slice4NextRotationAxis of UInputAction
{
	Initialize(EInputActionValueType::Axis1D);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_Slice4Reset of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	UQHASSlice4Component GetDefaultSlice4Component()
	{
		AQHASWorldSettings WorldSettings = QH::GetWorldSettings();

		return Cast<UQHASSlice4Component>(WorldSettings.GetDefaultSlice4Component());
	}

	////////////////////////////////////////////////////////////////////////////////
	UQHASSlice4Component GetActiveSlice4Component()
	{
		UQHSlice4MemberComponent Slice4MemberComponent = UQHSlice4MemberComponent::Get(QH::GetPlayerCharacter());

		return (Slice4MemberComponent != nullptr)
			 ? Cast<UQHASSlice4Component>(Slice4MemberComponent.GetOwnerSlice4())
			 : nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetActiveSlice4Component(UQHASSlice4Component InSlice4Component)
	{
		UQHSlice4MemberComponent Slice4MemberComponent = UQHSlice4MemberComponent::Get(QH::GetPlayerCharacter());

		if (Slice4MemberComponent != nullptr)
		{
			Slice4MemberComponent.SetOwnerSlice4(InSlice4Component);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void PushActiveSlice4Component(UQHASSlice4Component InSlice4Component)
	{
		UQHSlice4ControlComponent Slice4ControlComponent = UQHSlice4ControlComponent::Get(QH::GetPlayerController());

		if (Slice4ControlComponent != nullptr)
		{
			Slice4ControlComponent.PushActiveSlice4(InSlice4Component);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void PopActiveSlice4Component(UQHASSlice4Component InSlice4Component)
	{
		UQHSlice4ControlComponent Slice4ControlComponent = UQHSlice4ControlComponent::Get(QH::GetPlayerController());

		if (Slice4ControlComponent != nullptr)
		{
			Slice4ControlComponent.PopActiveSlice4(InSlice4Component);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	FQHSlice4 GetActiveSlice4()
	{
		UQHSlice4Component Slice4Component = QH::GetActiveSlice4Component();

		return (Slice4Component != nullptr)
			 ? Slice4Component.GetSlice4()
			 : FQHSlice4();
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetActiveSlice4(const FQHSlice4& InSlice)
	{
		UQHSlice4Component Slice4Component = QH::GetActiveSlice4Component();

		if (Slice4Component != nullptr)
		{
			Slice4Component.SetSlice4(InSlice);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void BindOnActiveSlice4Change(UObject InObject, const FName InFunction)
	{
		UQHSlice4ControlComponent Slice4ControlComponent = UQHSlice4ControlComponent::Get(QH::GetPlayerController());

		if (Slice4ControlComponent != nullptr)
		{
			Slice4ControlComponent.OnActiveSlice4Changed.AddUFunction(InObject, InFunction);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void UnbindOnActiveSlice4Change(UObject InObject, const FName InFunction)
	{
		UQHSlice4ControlComponent Slice4ControlComponent = UQHSlice4ControlComponent::Get(QH::GetPlayerController());

		if (Slice4ControlComponent != nullptr)
		{
			Slice4ControlComponent.OnActiveSlice4Changed.Unbind(InObject, InFunction);
		}
	}

	bool IsSliceRotationInputActive()
	{
		// TODO: Use chorded input to detect slice mouse rotation
		return QH::GetPlayerController().IsInputKeyDown(EKeys::RightMouseButton);
	}

	bool IsSliceDigitalRotationEnabled()
	{
		return CVar_QHSliceAllowDigitalRotation.GetBool();
	}
}

//______________________________________________________________________________
const FConsoleVariable CVar_QHSliceAllowAnyRotation("slice.rotate.any", false);
const FConsoleVariable CVar_QHSliceAllowDigitalRotation("slice.rotate.digi.on", false);
