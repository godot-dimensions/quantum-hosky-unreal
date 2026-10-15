#pragma once

class AQHItemActor;
class UQHItemDef;
class UQHItemInst;

//------------------------------------------------------------------------------

#include "QHGame/Utility/QHHandle.h"

#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHInventoryComponent.generated.h"

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHInventorySlotDef
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery ItemQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHInventoryItemDef
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<const UQHItemDef> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count = 1;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHInventoryDef
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQHInventorySlotDef> Slots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQHInventoryItemDef> Items;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHInventorySlotInst
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Handle = QH_HANDLE_INVALID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery ItemQuery;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UQHItemInst> Item = nullptr;

	//------------------------------------------------------------------------------

	bool HasItem() const;
	bool MatchesItem(UQHItemInst* InItem) const;

	bool IsValid() const
	{
		return UQHHandleLibrary::IsValid(Handle);
	}

	//------------------------------------------------------------------------------

	bool operator==(const int32 InHandle) const
	{
		return (Handle == InHandle);
	}

	bool operator<(const FQHInventorySlotInst& InOther) const
	{
		return (Priority > InOther.Priority);
	}
};

//______________________________________________________________________________
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnInventoryItemAdded, UQHInventoryComponent*, InInventoryComponent, const int32, InSlot, UQHItemInst*, InItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FQHOnInventoryItemRemoved, UQHInventoryComponent*, InInventoryComponent, const int32, InSlot, UQHItemInst*, InItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FQHOnInventoryItemPickedUp, UQHInventoryComponent*, InInventoryComponent, const int32, InSlot, UQHItemInst*, InItem, AQHItemActor*, InItemActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FQHOnInventoryItemDropped, UQHInventoryComponent*, InInventoryComponent, const int32, InSlot, UQHItemInst*, InItem, AQHItemActor*, InItemActor);

//______________________________________________________________________________
UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent = "true"))
class QHGAME_API UQHInventoryComponent
	: public UActorComponent
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	FQHInventoryDef Def;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnInventoryItemAdded OnItemAdded;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnInventoryItemRemoved OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnInventoryItemPickedUp OnItemPickedUp;

	UPROPERTY(BlueprintAssignable, Transient, DuplicateTransient, NonTransactional, SkipSerialization)
	FQHOnInventoryItemDropped OnItemDropped;

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	int32 AddItem(UQHItemInst* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	UQHItemInst* RemoveItem(const int32 InSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	int32 PickUpItem(AQHItemActor* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	AQHItemActor* DropItem(const int32 InSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	TArray<int32> AddSlots(const FQHInventorySlotDef& InDef);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	FQHInventorySlotInst GetSlot(const int32 InHandle) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const TArray<FQHInventorySlotInst>& GetSlots() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	int32 FindSlot(const FGameplayTagQuery& InQuery) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	TArray<int32> FindSlots(const FGameplayTagQuery& InQuery) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	int32 FindEmptySlotForItem(UQHItemInst* InItem) const;

//==============================================================================
private:

	UPROPERTY(Transient)
	TArray<FQHInventorySlotInst> Slots;

	int32 SlotHandleCounter = UQHHandleLibrary::NewCounter();

//==============================================================================
public:

	UQHInventoryComponent(const FObjectInitializer& InInit);

//==============================================================================
private:

	int32 FindSlotByHandle(const int32 InHandle) const;

	void RemoveItemFromSlot(FQHInventorySlotInst& InSlot);

//==============================================================================
protected:

	//~ UActorComponent BEGIN

	void InitializeComponent() override;
	void EndPlay(const EEndPlayReason::Type InReason) override;

	//~ UActorComponent END
};

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "FQHInventorySlotInst"))
class QHGAME_API UQHInventoryLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "HasItem"))
	static bool InventorySlotInst_HasItem(const FQHInventorySlotInst& InSlot);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "IsValid"))
	static bool InventorySlotInst_IsValid(const FQHInventorySlotInst& InSlot);
};
