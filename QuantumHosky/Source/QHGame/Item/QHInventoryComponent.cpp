#include "QHGame/Item/QHInventoryComponent.h"

#include "QHGame/Item/QHItem.h"

////////////////////////////////////////////////////////////////////////////////
bool FQHInventorySlotInst::HasItem() const
{
	return UQHItemLibrary::IsValidItem(Item);
}

////////////////////////////////////////////////////////////////////////////////
bool FQHInventorySlotInst::MatchesItem(UQHItemInst* InItem) const
{
	return (InItem != nullptr)
		&& (ItemQuery.IsEmpty() || ItemQuery.Matches(InItem->GetTags()));
}

////////////////////////////////////////////////////////////////////////////////
UQHInventoryComponent::UQHInventoryComponent(const FObjectInitializer& InInit)
	: Super(InInit)
{
	bWantsInitializeComponent = true;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHInventoryComponent::AddItem(UQHItemInst* InItem)
{
	int32 Result = QH_HANDLE_INVALID;

	const int32 SlotIndex = FindSlotByHandle(FindEmptySlotForItem(InItem));

	if (SlotIndex != INDEX_NONE)
	{
		FQHInventorySlotInst& Slot = Slots[SlotIndex];

		const int32 SlotHandle = Slot.Handle;

		Slot.Item = InItem;

		// Setup item ownership following internal setup to avoid external re-entrancy
		InItem->AddedToInventory(this, SlotHandle);

		OnItemAdded.Broadcast(this, SlotHandle, InItem);

		Result = SlotHandle;
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
UQHItemInst* UQHInventoryComponent::RemoveItem(const int32 InSlot)
{
	UQHItemInst* Result = nullptr;

	const int32 SlotIndex = FindSlotByHandle(InSlot);

	if (SlotIndex != INDEX_NONE)
	{
		FQHInventorySlotInst& Slot = Slots[SlotIndex];

		if (Slot.HasItem())
		{
			Result = Slot.Item;

			RemoveItemFromSlot(Slot);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHInventoryComponent::PickUpItem(AQHItemActor* InItem)
{
	int32 Result = QH_HANDLE_INVALID;

	if (UQHItemLibrary::IsValidItemActor(InItem) && InItem->CanBePickedUp(this))
	{
		UQHItemInst* Item = InItem->GetItem();

		// Add the item to the inventory
		const int32 Slot = AddItem(Item);

		if (UQHHandleLibrary::IsValid(Slot))
		{
			Item->PickedUp(this, Slot, InItem);

			OnItemPickedUp.Broadcast(this, Slot, Item, InItem);

			// Destroy picked-up item actor
			InItem->Destroy();

			Result = Slot;
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
AQHItemActor* UQHInventoryComponent::DropItem(const int32 InSlot)
{
	AQHItemActor* Result = nullptr;

	// Remove the item from the inventory
	if (UQHItemInst* Item = RemoveItem(InSlot))
	{
		// Drop the item as an actor;
		// NOTE: It is possible for dropped items to be picked up before UQHItemLibrary::DropItem returns (e.g. if dropping an item when standing against a wall), resulting in out-of-order item picked-up/dropped events.
		// Since items are generally picked up via overlaps, a potential solution could be to add support for ignoring item actor initial overlaps.
		AQHItemActor* ItemActor = UQHItemLibrary::DropItem(GetOwner(), Item);

		if (UQHItemLibrary::IsValidItemActor(ItemActor))
		{
			Item->Dropped(this, InSlot, ItemActor);

			OnItemDropped.Broadcast(this, InSlot, Item, ItemActor);

			Result = ItemActor;
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
TArray<int32> UQHInventoryComponent::AddSlots(const FQHInventorySlotDef& InDef)
{
	TArray<int32> Results;

	Slots.Reserve(Slots.Num() + InDef.Count);

	FQHInventorySlotInst SlotInst;
	SlotInst.Tags = InDef.Tags;
	SlotInst.ItemQuery = InDef.ItemQuery;
	SlotInst.Priority = InDef.Priority;

	for (int32 i = 0; i < InDef.Count; ++i)
	{
		SlotInst.Handle = UQHHandleLibrary::NewHandle(SlotHandleCounter);

		Slots.Add(SlotInst);

		Slots.StableSort();
	}

	return Results;
}

////////////////////////////////////////////////////////////////////////////////
FQHInventorySlotInst UQHInventoryComponent::GetSlot(const int32 InHandle) const
{
	const int32 SlotIndex = FindSlotByHandle(InHandle);

	return (SlotIndex != INDEX_NONE)
		 ? Slots[SlotIndex]
		 : FQHInventorySlotInst();
}

////////////////////////////////////////////////////////////////////////////////
const TArray<FQHInventorySlotInst>& UQHInventoryComponent::GetSlots() const
{
	return Slots;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHInventoryComponent::FindSlot(const FGameplayTagQuery& InQuery) const
{
	int32 Result = QH_HANDLE_INVALID;

	if (!InQuery.IsEmpty())
	{
		for (const FQHInventorySlotInst& Slot : Slots)
		{
			if (!Slot.Tags.IsEmpty() && InQuery.Matches(Slot.Tags))
			{
				Result = Slot.Handle;

				break;
			}
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
TArray<int32> UQHInventoryComponent::FindSlots(const FGameplayTagQuery& InQuery) const
{
	TArray<int32> Results;

	if (!InQuery.IsEmpty())
	{
		for (const FQHInventorySlotInst& Slot : Slots)
		{
			if (!Slot.Tags.IsEmpty() && InQuery.Matches(Slot.Tags))
			{
				Results.Add(Slot.Handle);
			}
		}
	}

	return Results;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHInventoryComponent::FindEmptySlotForItem(UQHItemInst* InItem) const
{
	int32 Result = QH_HANDLE_INVALID;

	if (UQHItemLibrary::IsValidItem(InItem))
	{
		for (const FQHInventorySlotInst& Slot : Slots)
		{
			if (!Slot.HasItem() && Slot.MatchesItem(InItem))
			{
				Result = Slot.Handle;

				break;
			}
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHInventoryComponent::FindSlotByHandle(const int32 InHandle) const
{
	return UQHHandleLibrary::IsValid(InHandle)
		 ? Slots.IndexOfByKey(InHandle)
		 : INDEX_NONE;
}

////////////////////////////////////////////////////////////////////////////////
void UQHInventoryComponent::RemoveItemFromSlot(FQHInventorySlotInst& InSlot)
{
	check(InSlot.HasItem());

	UQHItemInst* Item = InSlot.Item;

	InSlot.Item = nullptr;

	const int32 SlotHandle = InSlot.Handle;

	// Setup item ownership following internal setup to avoid external re-entrancy
	Item->RemovedFromInventory(this, SlotHandle);

	OnItemRemoved.Broadcast(this, SlotHandle, Item);
}

////////////////////////////////////////////////////////////////////////////////
void UQHInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Add default slots
	for (const FQHInventorySlotDef& SlotDef : Def.Slots)
	{
		AddSlots(SlotDef);
	}

	// Add default items
	for (const FQHInventoryItemDef& InventoryItemDef : Def.Items)
	{
		if (const UQHItemDef* ItemDef = InventoryItemDef.Item.LoadSynchronous())
		{
			for (int32 i = 0; i < InventoryItemDef.Count; ++i)
			{
				if (UQHItemInst* Item = UQHItemLibrary::CreateItem(ItemDef))
				{
					AddItem(Item);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHInventoryComponent::EndPlay(const EEndPlayReason::Type InReason)
{
	Super::EndPlay(InReason);

	// Clear items from slots;
	// TODO: `UQHInventoryComponent::RemoveAllItems/DropAllItems`.
	for (FQHInventorySlotInst& Slot : Slots)
	{
		if (Slot.HasItem())
		{
			RemoveItemFromSlot(Slot);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
bool UQHInventoryLibrary::InventorySlotInst_HasItem(const FQHInventorySlotInst& InSlot)
{
	return InSlot.HasItem();
}

////////////////////////////////////////////////////////////////////////////////
bool UQHInventoryLibrary::InventorySlotInst_IsValid(const FQHInventorySlotInst& InSlot)
{
	return InSlot.IsValid();
}
