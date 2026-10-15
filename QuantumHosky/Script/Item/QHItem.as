//______________________________________________________________________________
class AQHASItemActor
	: AQHItemActor
{
	default Scene4Component.LocalScale4D = FVector4(25.0f, 25.0f, 25.0f, 25.0f);

	default Mesh4Component.Mesh4D = QH::GetUnitBoxMesh4D();
	default Mesh4Component.bRenderInMainPass = false;
	default Mesh4Component.bRenderInDepthPass = false;
	default Mesh4Component.CastShadow = false;
	default Mesh4Component.EnableProjection = true; // Render 4D projections only
	default Mesh4Component.ProjectionDistance = 500.0f;

	default Slice4MemberComponent.Extents4 = 100.0f;

//==============================================================================
// public:

	UPROPERTY(OverrideComponent = Scene3Component, VisibleAnywhere, BlueprintReadOnly)
	UCapsuleComponent CapsuleComponent = nullptr;
	default CapsuleComponent.CapsuleRadius = 50.0f;
	default CapsuleComponent.CapsuleHalfHeight = 100.0f;
	default CapsuleComponent.CollisionProfileName = CollisionProfile::Item;
	default CapsuleComponent.SimulatePhysics = true;
	default CapsuleComponent.LinearDamping = 2.0f; 									// Cause items to maintain a low maximum speed and quickly come to rest
	default CapsuleComponent.BodyInstance.bOneWayInteraction = true; 				// Cause items to act as if they have no mass when colliding
	default CapsuleComponent.BodyInstance.bLockRotation = true; 					// Prevent items from rotating
	default CapsuleComponent.BodyInstance.bLockXRotation = true;
	default CapsuleComponent.BodyInstance.bLockYRotation = true;
	default CapsuleComponent.BodyInstance.bLockZRotation = true;
	default CapsuleComponent.BodyInstance.bOverrideMaxDepenetrationVelocity = true; // Cause items to gently depenetrate
	default CapsuleComponent.BodyInstance.MaxDepenetrationVelocity = 100.0f;

	UPROPERTY(DefaultComponent, VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent NameplateWidgetComponent = nullptr;
	default NameplateWidgetComponent.Space = EWidgetSpace::Screen;
	default NameplateWidgetComponent.bVisible = false; // Hide item nameplates by default
	default NameplateWidgetComponent.CollisionProfileName = CollisionProfile::NoCollision;

//==============================================================================
// private:

	private FTransform Mesh3RelativeTransformDefault;

	private float Mesh3SpinDirection = 0.0f;

	private UQHItemNameplateWidget NameplateWidget = nullptr;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void ShowNameplate(const bool InShow)
	{
		NameplateWidgetComponent.SetVisibility(InShow);
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnCrossedSlice(UQHSlice4MemberComponent InSlice4MemberComponent, bool InEnteredSlice)
	{
		// Allow item interactivity only while in-slice

		CapsuleComponent.SetSimulatePhysics(InEnteredSlice);
		CapsuleComponent.SetCollisionEnabled(InEnteredSlice ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);

		Mesh3Component.SetVisibility(InEnteredSlice, true);
	}

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// Store initial mesh relative transform
		Mesh3RelativeTransformDefault = Mesh3Component.GetRelativeTransform();

		// Pick a random direction to spin
		Mesh3SpinDirection = Math::RandBool()
						   ? 1.0f
						   : -1.0f;

		// Nameplate widget setup on the Blueprint
		NameplateWidget = Cast<UQHItemNameplateWidget>(NameplateWidgetComponent.GetWidget());

		if (NameplateWidget != nullptr)
		{
			NameplateWidget.SetName(Item.GetDef().ItemName);
		}

		Slice4MemberComponent.OnCrossedSlice.AddUFunction(this, n"OnCrossedSlice");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		Slice4MemberComponent.OnCrossedSlice.UnbindObject(this);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// NOTE: If this logic needs to be made modular, move it to an item component.

		// Oscillate item mesh vertically
		const float MeshOscillationPeriod = 7.0f;
		const float MeshOscillationDistance = 10.0f;
		Mesh3Component.SetRelativeLocation(Mesh3RelativeTransformDefault.GetLocation() + (FVector::UpVector * (Math::Sin(GetGameTimeSinceCreation() / MeshOscillationPeriod * TWO_PI) * MeshOscillationDistance)));

		// Spin item mesh in-place
		const float MeshSpinSpeed = 30.0f;
		Mesh3Component.AddRelativeRotation(FRotator(0.0f, (MeshSpinSpeed * Mesh3SpinDirection * InDeltaTime), 0.0f));

		// Scale item mesh by proximity to slice
		const float MeshScaleMin = 0.25f;
		const float MeshScale = Math::Min((Slice4MemberComponent.GetInsideOwnerSlice4Percent() + MeshScaleMin), 1.0f);
		Mesh3Component.SetRelativeScale3D(Mesh3RelativeTransformDefault.GetScale3D() * MeshScale);
	}

	//~ AActor END

	//~ AQHItemActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected bool CanBePickedUp(UQHInventoryComponent InInventoryComponent) const
	{
		const float CanBePickedUpItemAliveTime = 0.25f;

		return (GetGameTimeSinceCreation() >= CanBePickedUpItemAliveTime) 	// Prevent items from being immediately picked up (e.g. when dropping an item against a wall)
			 && Slice4MemberComponent.IsInsideOwnerSlice4();				// Only allow in-slice items to be picked up
	}

	//~ AQHItemActor END
}

//______________________________________________________________________________
class AQHItemSpawner
	: AActor
{

//==============================================================================
// public:

	UPROPERTY(DefaultComponent, RootComponent, VisibleAnywhere, BlueprintReadOnly)
	UScene4DComponent Root4DComponent = nullptr;

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UQHItemDef> Item = nullptr;

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		// TODO: Spawn item actor in 4D.

		QHItem::SpawnItemActorForDef(System::LoadAsset_Blocking(Item), Root4DComponent.GetWorldTransform());

		DestroyActor();
	}

	//~ AActor END
}

//______________________________________________________________________________
UCLASS(Abstract)
class UQHItemNameplateWidget
	: UUserWidget
{

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UTextBlock Text_Name = nullptr;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void SetName(const FText& InName)
	{
		Text_Name.SetText(InName);
	}
}

//______________________________________________________________________________
class UQHItemCollectorComponent
	: UActorComponent
{

//==============================================================================
// private:

	private UQHInventoryComponent InventoryComponent = nullptr;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnOwnerBeginOverlap(AActor InThisActor, AActor InOtherActor)
	{
		// Attempt to pick-up items when overlapping them

		AQHItemActor ItemActor = Cast<AQHItemActor>(InOtherActor);

		if (ItemActor != nullptr)
		{
			if (InventoryComponent != nullptr)
			{
				InventoryComponent.PickUpItem(ItemActor);
			}
		}
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		InventoryComponent = UQHInventoryComponent::Get(Owner);

		if (InventoryComponent != nullptr)
		{
			Owner.OnActorBeginOverlap.AddUFunction(this, n"OnOwnerBeginOverlap");
		}
	}


	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(const EEndPlayReason InReason)
	{
		Owner.OnActorBeginOverlap.UnbindObject(this);
	}
}

////////////////////////////////////////////////////////////////////////////////
UFUNCTION()
void CCmd_SpawnItem_Handler(const TArray<FString>& InArgs)
{
	// TODO: Add auto-complete to `item.spawn` debug command.

	if (!InArgs.IsEmpty())
	{
		const UQHItemDef ItemDef = Cast<UQHItemDef>(LoadObject(nullptr, InArgs[0]));

		const int32 NumItems = (InArgs.Num() > 1)
							 ? String::Conv_StringToInt(InArgs[1])
							 : 1;

		if (ItemDef != nullptr)
		{
			AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();

			if (PlayerCharacter != nullptr)
			{
				for (int32 i = 0; i < NumItems; ++i)
				{
					// Spawn the item from the player character
					AQHItemActor Item = QHItem::DropItemForDef(PlayerCharacter, ItemDef);

					if (Item != nullptr)
					{
						// Put the item in the player character's slice
						Item.Slice4MemberComponent.SetOwnerSlice4(PlayerCharacter.Slice4MemberComponent.GetOwnerSlice4());
					}
				}
			}
		}
		else
		{
			Error(f"item.spawn - failed to find item def \"{InArgs[0]}\"");
		}
	}
	else
	{
		Log("item.spawn - usage - `item.spawn <PATH_TO_ITEM_DEF> <NUM>`");
	}
}
const FConsoleCommand CCmd_SpawnItem("item.spawn", n"CCmd_SpawnItem_Handler");

////////////////////////////////////////////////////////////////////////////////
UFUNCTION()
void CCmd_GiveItem_Handler(const TArray<FString>& InArgs)
{
	// TODO: Add auto-complete to `item.give` debug command.

	if (!InArgs.IsEmpty())
	{
		const UQHItemDef ItemDef = Cast<UQHItemDef>(LoadObject(nullptr, InArgs[0]));

		const int32 NumItems = (InArgs.Num() > 1)
							 ? String::Conv_StringToInt(InArgs[1])
							 : 1;

		if (ItemDef != nullptr)
		{
			AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();

			if (PlayerCharacter != nullptr)
			{
				for (int32 i = 0; i < NumItems; ++i)
				{
					// Create and give the item to the player character
					PlayerCharacter.InventoryComponent.AddItem(QHItem::CreateItem(ItemDef));
				}
			}
		}
		else
		{
			Error(f"item.give - failed to find item def \"{InArgs[0]}\"");
		}
	}
	else
	{
		Log("item.give - usage - `item.give <PATH_TO_ITEM_DEF> <NUM>`");
	}
}
const FConsoleCommand CCmd_GiveItem("item.give", n"CCmd_GiveItem_Handler");

//______________________________________________________________________________
asset MAT_PHYS_Item of UPhysicalMaterial
{
	// Disable item friction; limit item velocity via damping to more universally limit item maximum speed
	Friction = 0.0f;
	StaticFriction = 0.0f;
	bOverrideFrictionCombineMode = true;
	FrictionCombineMode = EFrictionCombineMode::Multiply;

	// Disable item restitution; cause items to push against surfaces/other items rather than bounce
	Restitution = 0.0f;
	bOverrideRestitutionCombineMode = true;
	RestitutionCombineMode = EFrictionCombineMode::Multiply;
}
