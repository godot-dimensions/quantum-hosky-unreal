#pragma once

class AQHItemActor;
class UMaterialInterface;
class UScene4DComponent;
class UQHInventoryComponent;
class UQHItemComponent;
class UQHItemInst;
class UMesh4DComponent;
class UQHScene3To4Component;
class UQHSlice4MemberComponent;
class UStaticMesh;
class UStaticMeshComponent;

// -----------------------------------------------------------------------------

#include "QHGame/Utility/QHHandle.h"

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHItem.generated.h"

//------------------------------------------------------------------------------

// TODO: Add support for item stacking.
// TODO: Add support for default item components.
// TODO: Add support for item definition parenting / hierarchies.
// TODO: Add support for project-specific item definitions.
// TODO: Add support for project-specific item instances.
// TODO: Add support for project-specific item events.

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHItemMeshMaterialDef
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Slot = 0;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHItemMeshDef
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UStaticMesh> Asset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TOptional<FTransform> Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FQHItemMeshMaterialDef> Materials;
};

//______________________________________________________________________________
UCLASS(BlueprintType)
class QHGAME_API UQHItemDef
	: public UDataAsset
{

	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ScriptName = "ItemName"))
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UMaterialInterface> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftClassPtr<AQHItemActor> Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FQHItemMeshDef Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DestroyOnUse = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	TArray<TObjectPtr<UQHItemComponent>> Components;
};

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct QHGAME_API FQHItemUsageInfo
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	// TODO: Add support for item components to pass along generic data (e.g. instanced struct) via usage info for later processing and return to the caller.

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	//------------------------------------------------------------------------------

	bool IsValid() const
	{
		return (Instigator != nullptr);
	}
};

//______________________________________________________________________________
UENUM(BlueprintType)
enum class EQHItemUsageResultType
	: uint8
{
	Invalid,
	Success,
	Failure
};

//______________________________________________________________________________
/** Result of item usage by an item component. */
USTRUCT(BlueprintType)
struct QHGAME_API FQHItemComponentUsageResult
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQHItemUsageResultType Type = EQHItemUsageResultType::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Reason;

	/** If item usage processing should continue or stop on successful usage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Continue = true;

	//------------------------------------------------------------------------------

	FQHItemComponentUsageResult() = default;

	FQHItemComponentUsageResult(const EQHItemUsageResultType InType, const FText& InReason = FText())
		: Type(InType)
		, Reason(InReason)
	{
	}

	//------------------------------------------------------------------------------

	bool IsSuccess() const
	{
		return (Type == EQHItemUsageResultType::Success);
	}
};

//______________________________________________________________________________
/** Result of item usage. */
USTRUCT(BlueprintType)
struct QHGAME_API FQHItemUsageResult
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQHItemUsageResultType Type = EQHItemUsageResultType::Invalid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Reason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<const UQHItemComponent> Component = nullptr;

	//------------------------------------------------------------------------------

	FQHItemUsageResult() = default;

	FQHItemUsageResult(const FQHItemComponentUsageResult& InComponentResult, const UQHItemComponent* InComponent)
		: Type(InComponentResult.Type)
		, Reason(InComponentResult.Reason)
		, Component(InComponent)
	{
	}

	//------------------------------------------------------------------------------

	bool IsSuccess() const
	{
		return (Type == EQHItemUsageResultType::Success);
	}
};

//______________________________________________________________________________
UCLASS(BlueprintType)
class QHGAME_API UQHItemInst
	: public UObject
{
	GENERATED_BODY()

//==============================================================================
private:

	UPROPERTY()
	TObjectPtr<const UQHItemDef> Def = nullptr;

	UPROPERTY()
	FGameplayTagContainer Tags;

	UPROPERTY()
	TArray<TObjectPtr<UQHItemComponent>> Components;

	UPROPERTY()
	TObjectPtr<AQHItemActor> ItemActor = nullptr;

	UPROPERTY()
	TObjectPtr<UQHInventoryComponent> Inventory = nullptr;

	UPROPERTY()
	int32 InventorySlot = QH_HANDLE_INVALID;

	bool HasBegunPlay = false;
	int32 NumTickingComponents = 0;

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const UQHItemDef* GetDef() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	const FGameplayTagContainer& GetTags() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	AQHItemActor* GetItemActor() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UQHInventoryComponent* GetInventory() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	AActor* GetInventoryActor() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	int32 GetInventorySlot() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UObject* GetOwner() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	FQHItemUsageResult Use(const FQHItemUsageInfo& InInfo, const bool InTest = false);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool IsValidItem() const;

//==============================================================================
private:

	void Initialize(const UQHItemDef* InDef, const bool InDeferred = false);
	void Uninitialize();
	void BeginPlay();
	void EndPlay();
	void Tick(const float InDeltaTime);
	void AddedToInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void RemovedFromInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void ActorSpawned(AQHItemActor* InItemActor);
	void ActorDestroyed(AQHItemActor* InItemActor);
	void PickedUp(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);
	void Dropped(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);

	void NotifyTick(UWorld* InWorld, const ELevelTick InTickType, const float InDeltaTime);
	void NotifyComponentTickChanged(UQHItemComponent* InItemComponent);

//==============================================================================
protected:

	//~ UObject BEGIN

	UWorld* GetWorld() const override;

	//~ UObject END

//==============================================================================
public:

	friend class AQHItemActor;
	friend class UQHInventoryComponent;
	friend class UQHItemComponent;
	friend class UQHItemLibrary;
};

//______________________________________________________________________________
UCLASS(BlueprintType, Blueprintable, Abstract, EditInlineNew)
class QHGAME_API UQHItemComponent
	: public UObject
{
	GENERATED_BODY()

//==============================================================================
private:

	UPROPERTY(Transient)
	TObjectPtr<UQHItemInst> Owner = nullptr;

	bool TickEnabled = false;

//==============================================================================
protected:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UQHItemInst* GetOwner() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void SetTickEnabled(const bool InEnabled);

//==============================================================================
private:

	void BeginPlay(UQHItemInst* InOwner);
	void EndPlay();
	void Tick(const float InDeltaTime);
	void AddedToInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void RemovedFromInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void ActorSpawned(AQHItemActor* InItemActor);
	void ActorDestroyed(AQHItemActor* InItemActor);
	void PickedUp(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);
	void Dropped(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);
	TOptional<FQHItemComponentUsageResult> Use(const FQHItemUsageInfo& InInfo, const bool InTest);

//==============================================================================
protected:

	UFUNCTION(BlueprintNativeEvent)
	void OnBeginPlay();
	virtual void OnBeginPlay_Implementation()
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnEndPlay();
	virtual void OnEndPlay_Implementation()
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnTick(const float InDeltaTime);
	virtual void OnTick_Implementation(const float InDeltaTime)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnAddedToInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void OnAddedToInventory_Implementation(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnRemovedFromInventory(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot);
	void OnRemovedFromInventory_Implementation(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnActorSpawned(AQHItemActor* InItemActor);
	void OnActorSpawned_Implementation(AQHItemActor* InItemActor)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnActorDestroyed(AQHItemActor* InItemActor);
	void OnActorDestroyed_Implementation(AQHItemActor* InItemActor)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnPickedUp(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);
	virtual void OnPickedUp_Implementation(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor)
	{
	}

	UFUNCTION(BlueprintNativeEvent)
	void OnDropped(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor);
	virtual void OnDropped_Implementation(UQHInventoryComponent* InInventoryComponent, const int32 InInventorySlot, AQHItemActor* InItemActor)
	{
	}

	/** Use the item given the provided context.
	  * @param InTest If this usage is a test. If true, the item should verify that usage is valid but not commit any side-effects of usage.
	  * @return If this item component considers usage, return an FQHItemComponentUsageResult. If this item component does not consider usage, return empty.
	  */
	UFUNCTION(BlueprintNativeEvent)
	TOptional<FQHItemComponentUsageResult> OnUse(const FQHItemUsageInfo& InInfo, const bool InTest);
	virtual TOptional<FQHItemComponentUsageResult> OnUse_Implementation(const FQHItemUsageInfo& InInfo, const bool InTest)
	{
		return TOptional<FQHItemComponentUsageResult>();
	}

//==============================================================================
public:

	friend class UQHItemInst;
};

//______________________________________________________________________________
UCLASS(BlueprintType)
class QHGAME_API AQHItemActor
	: public AActor
{
	GENERATED_BODY()

//==============================================================================
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> Scene3Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UScene4DComponent> Scene4Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> Mesh3Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMesh4DComponent> Mesh4Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHScene3To4Component> Scene3To4Component = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHSlice4MemberComponent> Slice4MemberComponent = nullptr;

//==============================================================================
private:

	UPROPERTY(Transient)
	TObjectPtr<UQHItemInst> Item = nullptr;

//==============================================================================
public:

	AQHItemActor(const FObjectInitializer& InInit);

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	UQHItemInst* GetItem() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	bool IsValidItem() const;

	//~ TODO: Modularize `AQHItemActor::CanBePickedUp` via item components.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure = true)
	bool CanBePickedUp(UQHInventoryComponent* InInventoryComponent) const;
	virtual bool CanBePickedUp_Implementation(UQHInventoryComponent* InInventoryComponent) const
	{
		return true;
	}

//==============================================================================
private:

	void Initialize(UQHItemInst* InItem);

//==============================================================================
protected:

	//~ UObject BEGIN

	void PostInitProperties() override;

	//~ UObject END

	//~ AActor BEGIN

	void EndPlay(const EEndPlayReason::Type InReason) override;

	//~ AActor END

//==============================================================================
public:

	friend class UQHItemLibrary;
};

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "FQHItemComponentUsageResult FQHItemUsageResult"))
class QHGAME_API UQHItemLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static UQHItemInst* CreateItem(const UQHItemDef* InDef, const bool InDeferred = false);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static void DestroyItem(UQHItemInst* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (WorldContext = "InWorldContext"))
	static AQHItemActor* SpawnItemActor(const UObject* InWorldContext, UQHItemInst* InItem, const FTransform& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (WorldContext = "InWorldContext"))
	static AQHItemActor* SpawnItemActorForDef(const UObject* InWorldContext, const UQHItemDef* InItem, const FTransform& InTransform);

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static TSubclassOf<AQHItemActor> GetItemActorType(const UQHItemDef* InDef);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (WorldContext = "InWorldContext"))
	static AQHItemActor* DropItem(const AActor* InInstigator, UQHItemInst* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (WorldContext = "InWorldContext"))
	static AQHItemActor* DropItemForDef(const AActor* InInstigator, const UQHItemDef* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FVector GetItemDropLocation(const AActor* InInstigator, const UQHItemInst* InItem = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FVector GetItemDropLocationForDef(const AActor* InInstigator, const UQHItemDef* InItem = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static bool IsValidItem(const UQHItemInst* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static bool IsValidItemActor(const AQHItemActor* InItem);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "IsSuccess"))
	static bool ItemComponentUsageResult_IsSuccess(const FQHItemComponentUsageResult& InResult);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "IsSuccess"))
	static bool ItemUsageResult_IsSuccess(const FQHItemUsageResult& InResult);
};
