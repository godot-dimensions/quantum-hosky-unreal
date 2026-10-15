//______________________________________________________________________________
event void FQHOnInventoryItemHeld(UQHASInventoryComponent InInventoryComponent, const int32 InNewSlot, UQHItemInst InNewItem, const int32 InOldSlot, UQHItemInst InOldItem);

//______________________________________________________________________________
class UQHASInventoryComponent
	: UQHInventoryComponent
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> OnItemPickedUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> OnItemDroppedSound;

	UPROPERTY()
	FQHOnInventoryItemHeld OnItemHeld;

//==============================================================================
// private:

	private int32 HeldInventorySlot = QH_HANDLE_INVALID;
	private UQHItemInst HeldItem = nullptr;
	private USceneComponent HeldItemAttachComponent = nullptr;
	private FName HeldItemAttachSocket = NAME_None;
	private UStaticMeshComponent HeldItemMeshComponent = nullptr;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	int32 GetHeldInventorySlot() const
	{
		return HeldInventorySlot;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetHeldInventorySlot(const int32 InSlot)
	{
		if (HeldInventorySlot != InSlot)
		{
			const FQHInventorySlotInst Slot = GetSlot(InSlot);

			if (Slot.IsValid() && Slot.Tags.HasTagExact(GameplayTags::inventory_slot_hold))
			{
				const int32 OldHeldInventorySlot = HeldInventorySlot;
				const FQHInventorySlotInst OldSlot = GetSlot(OldHeldInventorySlot);

				HeldInventorySlot = InSlot;

				OnItemHeldChanged(HeldInventorySlot, Slot.Item, OldHeldInventorySlot, OldSlot.Item);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UQHItemInst GetHeldItem() const
	{
		return HeldItem;
	}

	////////////////////////////////////////////////////////////////////////////////
	UStaticMeshComponent GetHeldItemMesh() const
	{
		return (HeldItem != nullptr)
			 ? HeldItemMeshComponent
			 : nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetHeldItemAttachment(USceneComponent InAttachComponent, const FName InAttachSocket = NAME_None)
	{
		HeldItemAttachComponent = InAttachComponent;
		HeldItemAttachSocket = InAttachSocket;
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void OnItemHeldChanged(const int32 InNewSlot, UQHItemInst InNewItem, const int32 InOldSlot, UQHItemInst InOldItem)
	{
		// TODO: Add support for script-specific item events that can be handled by item components:
		// Option a): Add support for dynamic item events (e.g. `UQHItemInst::NotifyItemEvent(GameplayTags::item_event_held, FInstancedStruct::Make(FQHItemEvent_ItemHeld(this)))`).
		// Option b): Similar to `AQHASItemActor`, add support for `UQHASItemInst` that we can add custom events to (e.g. `UQHASItemInst::OnItemHeld`) that could then be handled by `UQHASItemComponents`.

		HeldItem = InNewItem;

		SetupHeldItemMesh(InNewItem);

		OnItemHeld.Broadcast(this, InNewSlot, InNewItem, InOldSlot, InOldItem);
	}

	////////////////////////////////////////////////////////////////////////////////
	private void SetupHeldItemMesh(UQHItemInst InItem)
	{
		// TODO: Add `UQHItemComponent_HeldSettings` for parameterization of held item settings (e.g. hold transform, etc.).

		if (HeldItemAttachComponent != nullptr)
		{
			if (InItem != nullptr)
			{
				// Ensure held item mesh component created
				if (HeldItemMeshComponent == nullptr)
				{
					HeldItemMeshComponent = UStaticMeshComponent::Create(Owner);
					HeldItemMeshComponent.FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
					HeldItemMeshComponent.SetCollisionProfileName(CollisionProfile::NoCollision);
					HeldItemMeshComponent.AttachToComponent(HeldItemAttachComponent, HeldItemAttachSocket);
				}

				HeldItemMeshComponent.SetVisibility(true, true);

				FVector ItemScale = FVector::OneVector;

				// Configure the held item mesh by the item's actor
				{
					UClass ItemActorType = QHItem::GetItemActorType(InItem.Def);

					if (ItemActorType != nullptr)
					{
						AQHItemActor ItemActorCDO = Cast<AQHItemActor>(ItemActorType.GetDefaultObject());

						if (ItemActorCDO != nullptr)
						{
							HeldItemMeshComponent.SetStaticMesh(ItemActorCDO.Mesh3Component.StaticMesh);

							// Reset materials

							const TArray<UMaterialInterface> ItemMaterials = ItemActorCDO.Mesh3Component.GetMaterials();

							for (int32 i = 0, end = ItemMaterials.Num(); i < end; ++i)
							{
								HeldItemMeshComponent.SetMaterial(i, ItemMaterials[i]);
							}

							ItemScale *= ItemActorCDO.Mesh3Component.GetRelativeScale3D();
						}
					}
				}

				// Configure the held item mesh by the item's def
				{
					UStaticMesh ItemMesh = System::LoadAsset_Blocking(InItem.Def.Mesh.Asset);

					if (ItemMesh != nullptr)
					{
						HeldItemMeshComponent.SetStaticMesh(ItemMesh);

						// Reset materials
						for (int32 i = 0, end = HeldItemMeshComponent.GetNumMaterials(); i < end; ++i)
						{
							HeldItemMeshComponent.SetMaterial(i, nullptr);
						}
					}

					for (const FQHItemMeshMaterialDef& ItemMaterialDef : InItem.Def.Mesh.Materials)
					{
						UMaterialInterface ItemMaterial = System::LoadAsset_Blocking(ItemMaterialDef.Asset);

						if (ItemMaterial != nullptr)
						{
							HeldItemMeshComponent.SetMaterial(ItemMaterialDef.Slot, ItemMaterial);
						}
					}

					if (InItem.Def.Mesh.Transform.IsSet())
					{
						const FTransform ItemTransform = InItem.Def.Mesh.Transform.GetValue();

						ItemScale *= ItemTransform.Scale3D;
					}
				}

				// Setup held item mesh relative attach transform
				{
					// Retrieve held item attach transform on mesh
					FTransform HeldItemMeshTransform = HeldItemMeshComponent.GetSocketTransform(n"Held", ERelativeTransformSpace::RTS_Component);
					HeldItemMeshTransform.SetScale3D(HeldItemMeshTransform.Scale3D.Reciprocal()); // Scale is stored inversely on the mesh socket

					// Retrieve held item attach transform on instigator
					const FTransform HeldItemAttachTransform = HeldItemAttachComponent.GetSocketTransform(HeldItemAttachSocket);

					FTransform HeldItemRelativeTransform;

					// Maintain constant scale of held item mesh relative to attach socket;
					// scale-down the item's mesh when held
					const float HeldItemScale = 0.2f;
					HeldItemRelativeTransform.SetScale3D(HeldItemAttachTransform.Scale3D.Reciprocal() * ItemScale * HeldItemScale);

					// Position held item mesh such that it is attached at the mesh socket with the specified scale relative to the attach socket
					HeldItemMeshComponent.SetRelativeTransform(HeldItemMeshTransform.Inverse() * HeldItemRelativeTransform);
				}
			}
			else if (HeldItemMeshComponent != nullptr)
			{
				HeldItemMeshComponent.SetVisibility(false, true);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void HandleOnItemAdded(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem)
	{
		Gameplay::PlaySoundAtLocation(Sound = System::LoadAsset_Blocking(OnItemPickedUpSound),
									  Location = Owner.GetActorLocation(),
									  Rotation = Owner.GetActorRotation(),
									  OwningActor = Owner);

		// Update held item
		if (InSlot == HeldInventorySlot)
		{
			OnItemHeldChanged(HeldInventorySlot, InItem, HeldInventorySlot, nullptr);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void HandleOnItemRemoved(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem)
	{
		// Update held item
		if (InSlot == HeldInventorySlot)
		{
			OnItemHeldChanged(HeldInventorySlot, nullptr, HeldInventorySlot, InItem);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void HandleOnItemPickedUp(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem, AQHItemActor InItemActor)
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void HandleOnItemDropped(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem, AQHItemActor InItemActor)
	{
		if (InItemActor.Slice4MemberComponent.GetOwnerSlice4() == nullptr)
		{
			UQHSlice4MemberComponent Slice4MemberComponent = UQHSlice4MemberComponent::Get(InInventoryComponent.Owner);

			if (Slice4MemberComponent != nullptr)
			{
				// Put the dropped item in the same slice as its instigator
				InItemActor.Slice4MemberComponent.SetOwnerSlice4(Slice4MemberComponent.GetOwnerSlice4());
			}
		}

		Gameplay::PlaySoundAtLocation(Sound = System::LoadAsset_Blocking(OnItemDroppedSound),
									  Location = Owner.GetActorLocation(),
									  Rotation = Owner.GetActorRotation(),
									  OwningActor = Owner);
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		OnItemAdded.AddUFunction(this, n"HandleOnItemAdded");
		OnItemRemoved.AddUFunction(this, n"HandleOnItemRemoved");
		OnItemPickedUp.AddUFunction(this, n"HandleOnItemPickedUp");
		OnItemDropped.AddUFunction(this, n"HandleOnItemDropped");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason EndPlayReason)
	{
		OnItemAdded.UnbindObject(this);
		OnItemRemoved.UnbindObject(this);
		OnItemPickedUp.UnbindObject(this);
		OnItemDropped.UnbindObject(this);

		if (HeldItemMeshComponent != nullptr)
		{
			HeldItemMeshComponent.DestroyComponent(Owner);
		}
	}
}

//______________________________________________________________________________
UCLASS(Abstract)
class UQHInventorySlotWidget
	: UUserWidget
{

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UBorder Border_Item = nullptr;

	UPROPERTY(BindWidget)
	private UImage Image_ItemIcon = nullptr;

	private int32 InventorySlot = QH_HANDLE_INVALID;

	private UQHItemInst Item = nullptr;

	private int32 Selected = 0;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	int32 GetInventorySlot() const
	{
		return InventorySlot;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetInventorySlot(const int32 InSlot)
	{
		InventorySlot = InSlot;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetItem(UQHItemInst InItem)
	{
		Item = InItem;

		if (QHItem::IsValidItem(InItem))
		{
			Image_ItemIcon.SetVisibility(ESlateVisibility::Visible);
			Image_ItemIcon.SetBrushFromSoftMaterial(InItem.Def.Icon);
		}
		else
		{
			Image_ItemIcon.SetVisibility(ESlateVisibility::Hidden);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetSelected(const bool InSelected)
	{
		Selected += (InSelected ? 1 : -1);

		const FLinearColor SelectedColor(1.0f, 0.7f, 0.0f);
		const FLinearColor UnselectedColor(0.01f, 0.01f, 0.01f);

		Border_Item.SetBrushColor((Selected > 0) ? SelectedColor : UnselectedColor);
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private UWidget GetTooltipWidget()
	{
		UWidget Result = nullptr;

		if (QHItem::IsValidItem(Item))
		{
			const UQHItemDef ItemDef = Item.GetDef();

			Result = QH::GetTooltipWidget(ItemDef.ItemName, ItemDef.Description);
		}

		return Result;
	}

//==============================================================================
// protected:

	//~ UUserWidget BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInitialized()
	{
		SetTooltipWidgetDelegate(FQHGetWidget(this, n"GetTooltipWidget"));
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnMouseEnter(FGeometry MyGeometry, FPointerEvent MouseEvent)
	{
		SetSelected(true);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void OnMouseLeave(FPointerEvent MouseEvent)
	{
		SetSelected(false);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	FEventReply OnMouseButtonDown(FGeometry MyGeometry, FPointerEvent MouseEvent)
	{
		// Drop the item on mouse right-click

		FEventReply Result = FEventReply::Unhandled();

		if (MouseEvent.IsMouseButtonDown(EKeys::RightMouseButton))
		{
			if (QHItem::IsValidItem(Item))
			{
				Item.GetInventory().DropItem(InventorySlot);

				Result = FEventReply::Handled();
			}
		}

		return Result;
	}

	//~ UUserWidget END
}

//______________________________________________________________________________
UCLASS(Abstract)
class UQHInventoryWidget
	: UUserWidget
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	TSubclassOf<UQHInventorySlotWidget> InventorySlotWidgetType;

//==============================================================================
// protected:

	protected UQHASInventoryComponent Inventory = nullptr;

	protected TArray<UQHInventorySlotWidget> InventorySlotWidgets;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void InitializeInventory(UQHASInventoryComponent InInventory)
	{
		UninitializeInventory();

		Inventory = InInventory;

		if (Inventory != nullptr)
		{
			// Listen for the inventory to change
			Inventory.OnItemAdded.AddUFunction(this, n"OnInventoryItemAdded");
			Inventory.OnItemRemoved.AddUFunction(this, n"OnInventoryItemRemoved");

			OnInventoryInitialized();
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void UninitializeInventory()
	{
		if (Inventory != nullptr)
		{
			Inventory.OnItemAdded.UnbindObject(this);
			Inventory.OnItemRemoved.UnbindObject(this);

			OnInventoryUninitialized();
		}

		Inventory = nullptr;
	}

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	protected void OnInventoryInitialized()
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	protected void OnInventoryUninitialized()
	{
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	protected void OnInventoryItemAdded(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem)
	{
		UQHInventorySlotWidget InventorySlotWidget = FindInventorySlotWidget(InSlot);

		if (InventorySlotWidget != nullptr)
		{
			InventorySlotWidget.SetItem(InItem);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintEvent)
	protected void OnInventoryItemRemoved(UQHInventoryComponent InInventoryComponent, int InSlot, UQHItemInst InItem)
	{
		UQHInventorySlotWidget InventorySlotWidget = FindInventorySlotWidget(InSlot);

		if (InventorySlotWidget != nullptr)
		{
			InventorySlotWidget.SetItem(nullptr);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	protected UQHInventorySlotWidget CreateInventorySlotWidget(const FQHInventorySlotInst& InSlot)
	{
		check(InSlot.IsValid());

		check(InventorySlotWidgetType.IsValid());

		UQHInventorySlotWidget InventorySlotWidget = Cast<UQHInventorySlotWidget>(WidgetBlueprint::CreateWidget(InventorySlotWidgetType, OwningPlayer));
		check(InventorySlotWidget != nullptr);

		InventorySlotWidget.SetInventorySlot(InSlot.Handle);
		InventorySlotWidget.SetItem(InSlot.Item);

		InventorySlotWidgets.Add(InventorySlotWidget);

		return InventorySlotWidget;
	}

	////////////////////////////////////////////////////////////////////////////////
	protected UQHInventorySlotWidget FindInventorySlotWidget(const int32 InSlot) const
	{
		UQHInventorySlotWidget Result = nullptr;

		for (UQHInventorySlotWidget InventorySlotWidget : InventorySlotWidgets)
		{
			if (InventorySlotWidget.GetInventorySlot() == InSlot)
			{
				Result = InventorySlotWidget;

				break;
			}
		}

		return Result;
	}
}

//______________________________________________________________________________
UCLASS(Abstract)
class UQHInventoryStorageWidget
	: UQHInventoryWidget
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	int32 NumInventorySlotRows = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	int32 NumInventorySlotColumns = 4;

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UUniformGridPanel Panel_InventorySlots = nullptr;

//==============================================================================
// protected:

	//~ UQHInventoryWidget BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInventoryInitialized()
	{
		Super::OnInventoryInitialized();

		// Create a widget for each `storage` inventory slot

		const TArray<int32> InventorySlots = Inventory.FindSlots(GameplayTag::MakeGameplayTagQuery_MatchAllTags(GameplayTags::inventory_slot_storage.SingleTagContainer));

		for (int32 i = 0, end = InventorySlots.Num(); i < end; ++i)
		{
			UQHInventorySlotWidget InventorySlotWidget = CreateInventorySlotWidget(Inventory.GetSlot(InventorySlots[i]));

			const int32 SlotRow = Math::IntegerDivisionTrunc(i, NumInventorySlotRows);
			const int32 SlotColumn = (i % NumInventorySlotColumns);

			Panel_InventorySlots.AddChildToUniformGrid(InventorySlotWidget, SlotRow, SlotColumn);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInventoryUninitialized()
	{
		Super::OnInventoryUninitialized();

		Panel_InventorySlots.ClearChildren();
	}

	//~ UQHInventoryWidget END
}

//______________________________________________________________________________
UCLASS(Abstract)
class UQHInventoryQuickBarWidget
	: UQHInventoryWidget
{

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UHorizontalBox Container_InventorySlots = nullptr;

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void SetHeldInventorySlot(const int32 InNewSlot, const int32 InOldSlot)
	{
		UQHInventorySlotWidget OldInventorySlotWidget = FindInventorySlotWidget(InOldSlot);

		if (OldInventorySlotWidget != nullptr)
		{
			OldInventorySlotWidget.SetSelected(false);
		}

		UQHInventorySlotWidget NewInventorySlotWidget = FindInventorySlotWidget(InNewSlot);

		if (NewInventorySlotWidget != nullptr)
		{
			NewInventorySlotWidget.SetSelected(true);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnItemHeld(UQHASInventoryComponent InInventoryComponent, const int32 InNewSlot, UQHItemInst InNewItem, const int32 InOldSlot, UQHItemInst InOldItem)
	{
		SetHeldInventorySlot(InNewSlot, InOldSlot);
	}

//==============================================================================
// protected:

	//~ UQHInventoryWidget BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInventoryInitialized()
	{
		Super::OnInventoryInitialized();

		// Create a widget for each `holdable` inventory slot

		const TArray<int32> InventorySlots = Inventory.FindSlots(GameplayTag::MakeGameplayTagQuery_MatchAllTags(GameplayTags::inventory_slot_hold.SingleTagContainer));

		for (const int32 InventorySlot : InventorySlots)
		{
			UQHInventorySlotWidget InventorySlotWidget = CreateInventorySlotWidget(Inventory.GetSlot(InventorySlot));

			Container_InventorySlots.AddChildToHorizontalBox(InventorySlotWidget);
		}

		// Listen for the held item to change
		{
			Inventory.OnItemHeld.AddUFunction(this, n"OnItemHeld");

			SetHeldInventorySlot(Inventory.GetHeldInventorySlot(), QH_HANDLE_INVALID);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInventoryUninitialized()
	{
		Super::OnInventoryUninitialized();

		Container_InventorySlots.ClearChildren();
	}

	//~ UQHInventoryWidget END
}
