//______________________________________________________________________________
// TODO: Rename `UQHPlayerCharacterControlComponent` -> `UQHPlayerControlComponent` and move from `AQHASPlayerCharacter` -> `AQHPlayerController`.
class UQHPlayerCharacterControlComponent
	: UActorComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookSpeed = 100.0f;

//==============================================================================
// private:

	private UQHASInventoryComponent PlayerInventory = nullptr;
	private TArray<int32> PlayerHoldableInventorySlots;

	private uint32 Input_PlayerCharacterMove;
	private uint32 Input_PlayerCharacterLook;
	private uint32 Input_PlayerCharacterJump;
	private uint32 Input_PlayerOpenInventory;
	private uint32 Input_PlayerHoldNextInventorySlot;
	private TArray<UInputAction> InputAction_PlayerHoldInventorySlotN;
	private TArray<uint32> Input_PlayerHoldInventorySlotN;
	private uint32 Input_PlayerUse;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerCharacterMove(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		ACharacter Character = Cast<ACharacter>(Owner);

		const FVector2D ControllerInput = ActionValue.GetAxis2D();
		const FVector MovementInput = (Character.ActorRotation.GetForwardVector().GetSafeNormal2D() * ControllerInput.Y)
							  		+ (Character.ActorRotation.GetRightVector().GetSafeNormal2D() * ControllerInput.X);

		Character.AddMovementInput(MovementInput);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerCharacterLook(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		// Do not rotate the 3D camera when rotating the 4D camera;
		// TODO: Use chorded input to detect slice mouse rotation
		if (QH::IsSliceRotationInputActive())
		{
			return;
		}

		APlayerController Controller = QH::GetPlayerController();

		const FVector2D ControllerInput = ActionValue.GetAxis2D();
		const float LookVelocity = (LookSpeed * World.GetDeltaSeconds());

		Controller.AddYawInput(ControllerInput.X * LookVelocity);
		Controller.AddPitchInput(ControllerInput.Y * LookVelocity);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerCharacterJump(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		ACharacter Character = Cast<ACharacter>(Owner);

		Character.Jump();
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerOpenInventory(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		AQHASHUD HUD = QH::GetHUD();

		if (HUD != nullptr)
		{
			HUD.SetInventoryOpen(!HUD.GetInventoryOpen());
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerHoldNextInventorySlot(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		const int32 CurrentHeldInventorySlot = PlayerHoldableInventorySlots.FindIndex(PlayerInventory.GetHeldInventorySlot());
		const int32 NextHeldInventorySlot = Math::WrapIndex((CurrentHeldInventorySlot + int32(ActionValue.Axis1D)), 0, PlayerHoldableInventorySlots.Num());

		if (PlayerHoldableInventorySlots.IsValidIndex(NextHeldInventorySlot))
		{
			PlayerInventory.SetHeldInventorySlot(PlayerHoldableInventorySlots[NextHeldInventorySlot]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerHoldInventorySlotN(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		const int32 HoldInventorySlot = InputAction_PlayerHoldInventorySlotN.FindIndex(SourceAction);

		if ((HoldInventorySlot != -1) && PlayerHoldableInventorySlots.IsValidIndex(HoldInventorySlot))
		{
			PlayerInventory.SetHeldInventorySlot(PlayerHoldableInventorySlots[HoldInventorySlot]);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInput_PlayerUse(FInputActionValue ActionValue, float32 ElapsedTime, float32 TriggeredTime, const UInputAction SourceAction)
	{
		UQHInteractableComponent Interactable = QH::GetPlayerInteractable();

		// If we have an interactable, interact with it
		if (Interactable != nullptr)
		{
			Interactable.Interact(Owner);
		}
		// Otherwise use our held item
		else
		{
			UQHItemInst Item = PlayerInventory.GetHeldItem();

			if (Item != nullptr)
			{
				FQHItemUsageInfo UsageInfo;
				UsageInfo.Instigator = Owner;

				Item.Use(UsageInfo);
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
		PlayerInventory = QH::GetPlayerInventory();
		PlayerHoldableInventorySlots = PlayerInventory.FindSlots(GameplayTag::MakeGameplayTagQuery_MatchAnyTags(GameplayTags::inventory_slot_hold.SingleTagContainer));

		// Setup input
		{
			UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

			if (InputSystem != nullptr)
			{
				InputSystem.AddMappingContext(INPUT_CTX_PlayerCharacterControl, 0, FModifyContextOptions());
			}

			UEnhancedInputComponent InputComponent = QH::GetInputComponent();

			if (InputComponent != nullptr)
			{
				Input_PlayerCharacterMove = InputComponent.BindAction(INPUT_ACT_PlayerCharacterMove, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerCharacterMove")).GetHandle();
				Input_PlayerCharacterLook = InputComponent.BindAction(INPUT_ACT_PlayerCharacterLook, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerCharacterLook")).GetHandle();
				Input_PlayerCharacterJump = InputComponent.BindAction(INPUT_ACT_PlayerCharacterJump, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerCharacterJump")).GetHandle();
				Input_PlayerOpenInventory = InputComponent.BindAction(INPUT_ACT_PlayerOpenInventory, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerOpenInventory")).GetHandle();
				Input_PlayerHoldNextInventorySlot = InputComponent.BindAction(INPUT_ACT_PlayerHoldNextInventorySlot, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerHoldNextInventorySlot")).GetHandle();

				// Hold inventory slot N
				{
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot0);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot1);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot2);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot3);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot4);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot5);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot6);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot7);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot8);
					InputAction_PlayerHoldInventorySlotN.Add(INPUT_ACT_PlayerHoldInventorySlot9);

					for (int32 i = 0, end = InputAction_PlayerHoldInventorySlotN.Num(); i < end; ++i)
					{
						const uint32 InputBinding = InputComponent.BindAction(InputAction_PlayerHoldInventorySlotN[i], ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerHoldInventorySlotN")).GetHandle();

						Input_PlayerHoldInventorySlotN.Add(InputBinding);
					}
				}

				Input_PlayerUse = InputComponent.BindAction(INPUT_ACT_PlayerUse, ETriggerEvent::Triggered, FEnhancedInputActionHandlerDynamicSignature(this, n"OnInput_PlayerUse")).GetHandle();
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason EndPlayReason)
	{
		// Teardown input
		{
			UEnhancedInputLocalPlayerSubsystem InputSystem = UEnhancedInputLocalPlayerSubsystem::Get(QH::GetPlayerController());

			if (InputSystem != nullptr)
			{
				InputSystem.RemoveMappingContext(INPUT_CTX_PlayerCharacterControl, FModifyContextOptions());
			}

			UEnhancedInputComponent InputComponent = QH::GetInputComponent();

			if (InputComponent != nullptr)
			{
				InputComponent.RemoveBindingByHandle(Input_PlayerCharacterMove);
				InputComponent.RemoveBindingByHandle(Input_PlayerCharacterLook);
				InputComponent.RemoveBindingByHandle(Input_PlayerCharacterJump);
				InputComponent.RemoveBindingByHandle(Input_PlayerOpenInventory);
				InputComponent.RemoveBindingByHandle(Input_PlayerHoldNextInventorySlot);

				InputAction_PlayerHoldInventorySlotN.Empty();

				for (const uint32 InputBinding : Input_PlayerHoldInventorySlotN)
				{
					InputComponent.RemoveBindingByHandle(InputBinding);
				}

				Input_PlayerHoldInventorySlotN.Empty();

				InputComponent.RemoveBindingByHandle(Input_PlayerUse);
			}
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
asset INPUT_CTX_PlayerCharacterControl of UInputMappingContext
{
	Initialize();

	// PlayerCharacterMove
	{
		// D
		MapKey(INPUT_ACT_PlayerCharacterMove, EKeys::D);

		// A
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterMove, EKeys::A);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}

		// W
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterMove, EKeys::W);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierSwizzleAxis));
		}

		// S
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterMove, EKeys::S);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierSwizzleAxis));
			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}

		// Gamepad left-stick
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterMove, EKeys::Gamepad_Left2D);

			{
				UInputModifierDeadZone ModifierDeadzone = NewObject(this, UInputModifierDeadZone);
				ModifierDeadzone.Type = EDeadZoneType::Axial;
				ModifierDeadzone.LowerThreshold = 0.2f;
				ModifierDeadzone.UpperThreshold = 0.8f;

				Mapping.Modifiers.Add(ModifierDeadzone);
			}
		}
	}

	// PlayerCharacterLook
	{
		// Mouse movement
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterLook, EKeys::Mouse2D);

			{
				UInputModifierNegate ModifierNegate = NewObject(this, UInputModifierNegate);
				ModifierNegate.bX = false;
				ModifierNegate.bY = true;

				Mapping.Modifiers.Add(ModifierNegate);
			}
		}

		// Gamepad right-stick
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerCharacterLook, EKeys::Gamepad_Right2D);

			{
				UInputModifierDeadZone ModifierDeadzone = NewObject(this, UInputModifierDeadZone);
				ModifierDeadzone.Type = EDeadZoneType::Axial;
				ModifierDeadzone.LowerThreshold = 0.2f;
				ModifierDeadzone.UpperThreshold = 0.8f;

				Mapping.Modifiers.Add(ModifierDeadzone);
			}
		}
	}

	// PlayerCharacterJump
	{
		// Spacebar
		MapKey(INPUT_ACT_PlayerCharacterJump, EKeys::SpaceBar);

		// Gamepad bottom face-button
		MapKey(INPUT_ACT_PlayerCharacterJump, EKeys::Gamepad_FaceButton_Bottom);
	}

	// PlayerOpenInventory
	{
		// I
		MapKey(INPUT_ACT_PlayerOpenInventory, EKeys::I);

		// Gamepad top face-button
		MapKey(INPUT_ACT_PlayerOpenInventory, EKeys::Gamepad_FaceButton_Top);
	}

	// PlayerHoldNextInventorySlot
	{
		// Mouse scroll-down
		MapKey(INPUT_ACT_PlayerHoldNextInventorySlot, EKeys::MouseScrollDown);

		// Mouse scroll-up
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerHoldNextInventorySlot, EKeys::MouseScrollUp);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}

		// Gamepad dpad-right
		MapKey(INPUT_ACT_PlayerHoldNextInventorySlot, EKeys::Gamepad_DPad_Right);

		// Gamepad dpad-left
		{
			FEnhancedActionKeyMapping& Mapping = MapKey(INPUT_ACT_PlayerHoldNextInventorySlot, EKeys::Gamepad_DPad_Left);

			Mapping.Modifiers.Add(NewObject(this, UInputModifierNegate));
		}
	}

	// PlayerHoldInventorySlotN
	{
		// 1
		MapKey(INPUT_ACT_PlayerHoldInventorySlot0, EKeys::One);

		// 2
		MapKey(INPUT_ACT_PlayerHoldInventorySlot1, EKeys::Two);

		// 3
		MapKey(INPUT_ACT_PlayerHoldInventorySlot2, EKeys::Three);

		// 4
		MapKey(INPUT_ACT_PlayerHoldInventorySlot3, EKeys::Four);

		// 5
		MapKey(INPUT_ACT_PlayerHoldInventorySlot4, EKeys::Five);

		// 6
		MapKey(INPUT_ACT_PlayerHoldInventorySlot5, EKeys::Six);

		// 7
		MapKey(INPUT_ACT_PlayerHoldInventorySlot6, EKeys::Seven);

		// 8
		MapKey(INPUT_ACT_PlayerHoldInventorySlot7, EKeys::Eight);

		// 9
		MapKey(INPUT_ACT_PlayerHoldInventorySlot8, EKeys::Nine);

		// 0
		MapKey(INPUT_ACT_PlayerHoldInventorySlot9, EKeys::Zero);
	}

	// PlayerUse
	{
		// Mouse left-click
		MapKey(INPUT_ACT_PlayerUse, EKeys::LeftMouseButton);

		// F
		MapKey(INPUT_ACT_PlayerUse, EKeys::F);

		// Gamepad right-shoulder
		MapKey(INPUT_ACT_PlayerUse, EKeys::Gamepad_RightShoulder);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerCharacterMove of UInputAction
{
	Initialize(EInputActionValueType::Axis2D);

	{
		UInputTriggerDown TriggerDown = NewObject(this, UInputTriggerDown);
		TriggerDown.ActuationThreshold = 0.0f;

		Triggers.Add(TriggerDown);
	}

	// Clamp input magnitude from exceeding 1 (e.g. if pressing W and D at the same time)
	{
		UInputModifierDeadZone ModifierDeadzone = NewObject(this, UInputModifierDeadZone);
		ModifierDeadzone.LowerThreshold = 0.0f;
		ModifierDeadzone.UpperThreshold = 1.0f;

		Modifiers.Add(ModifierDeadzone);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerCharacterLook of UInputAction
{
	Initialize(EInputActionValueType::Axis2D);

	bConsumeInput = false;

	{
		UInputTriggerDown TriggerDown = NewObject(this, UInputTriggerDown);
		TriggerDown.ActuationThreshold = 0.0f;

		Triggers.Add(TriggerDown);
	}
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerCharacterJump of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerDown));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerOpenInventory of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldNextInventorySlot of UInputAction
{
	Initialize(EInputActionValueType::Axis1D);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot0 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot1 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot2 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot3 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot4 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot5 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot6 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot7 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot8 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerHoldInventorySlot9 of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}

//______________________________________________________________________________
asset INPUT_ACT_PlayerUse of UInputAction
{
	Initialize(EInputActionValueType::Boolean);

	Triggers.Add(NewObject(this, UInputTriggerPressed));
}
