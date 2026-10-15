//______________________________________________________________________________
class AQHASPlayerCharacter
	: AQHCharacter
{
	default CharacterMovement.MaxWalkSpeed = 1000.0f;
	default CharacterMovement.MaxAcceleration = 5000.0f;
	default CharacterMovement.JumpZVelocity = 800.0f;
	default CharacterMovement.AirControl = 0.2f;

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent CameraComponent = nullptr;
	default CameraComponent.EnableFirstPersonFieldOfView = true;
	default CameraComponent.EnableFirstPersonScale = true;
	default CameraComponent.FirstPersonScale = 0.1f;

	UPROPERTY(DefaultComponent, Attach = CameraComponent, VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent HandMeshComponent = nullptr;
	default HandMeshComponent.FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	default HandMeshComponent.CollisionProfileName = CollisionProfile::NoCollision;

	UPROPERTY(DefaultComponent, Attach = HandMeshComponent, VisibleAnywhere, BlueprintReadOnly)
	USceneComponent HeldItemAttachComponent = nullptr;

	// TODO: Rename `UQHPlayerCharacterControlComponent` -> `UQHPlayerControlComponent` and move from `AQHASPlayerCharacter` -> `AQHPlayerController`.
	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerCharacterControlComponent PlayerCharacterControlComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerCharacterFootstepComponent PlayerCharacterFootstepComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerCharacterMantleComponent PlayerCharacterMantleComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHPlayerCharacterClimbComponent PlayerCharacterClimbComponent = nullptr;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHASInventoryComponent InventoryComponent = nullptr;
	default InventoryComponent.Def = GetInventoryDef();

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UQHItemCollectorComponent ItemCollectorComponent = nullptr;

// private:
	private FVector SpawnLocation = FVector::ZeroVector;
	private FRotator SpawnRotation = FRotator::ZeroRotator;

// public:
	void ResetPlayer()
	{
		// Teleport player to spawn
		SetActorLocationAndRotation(SpawnLocation, SpawnRotation, bTeleport = true);

		// Stop movement
		CharacterMovement.StopMovementImmediately();

		// Reset 3D camera
		QH::GetPlayerController().SetControlRotation(SpawnRotation);

		// Reset 4D camera
		QH::GetActiveSlice4Component().ResetSlice4(InBlendDuration = 0.0f);
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private FQHInventoryDef GetInventoryDef() const
	{
		FQHInventoryDef Result;

		// Storage
		{
			FQHInventorySlotDef SlotDef;
			SlotDef.Tags.AddTag(GameplayTags::inventory_slot_storage);
			SlotDef.Count = 16;

			Result.Slots.Add(SlotDef);
		}

		// Holdable
		{
			FQHInventorySlotDef SlotDef;
			SlotDef.Tags.AddTag(GameplayTags::inventory_slot_hold);
			SlotDef.Priority = 1; // Pickup items into holdable slots first
			SlotDef.Count = 10;

			Result.Slots.Add(SlotDef);
		}

		return Result;
	}

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		SpawnLocation = GetActorLocation();
		SpawnRotation = GetActorRotation();

		// Initialize inventory
		{
			InventoryComponent.SetHeldItemAttachment(HeldItemAttachComponent);

			// Hold the first slot
			InventoryComponent.SetHeldInventorySlot(InventoryComponent.FindSlot(GameplayTag::MakeGameplayTagQuery_MatchAnyTags(GameplayTags::inventory_slot_hold.SingleTagContainer)));
		}
	}

	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Out-of-bounds protection
		const float KillZ = -5000.0f;
		if (GetActorLocation().Z < KillZ)
		{
			ResetPlayer();
		}
	}

	//~ AActor END
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHASPlayerCharacter GetPlayerCharacter()
	{
		return Cast<AQHASPlayerCharacter>(Gameplay::GetPlayerCharacter(0));
	}

	////////////////////////////////////////////////////////////////////////////////
	UQHASInventoryComponent GetPlayerInventory()
	{
		AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();

		return (PlayerCharacter != nullptr)
			 ? PlayerCharacter.InventoryComponent
			 : nullptr;
	}
}
