#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "Containers/Array.h"
#include "LevelInstance/LevelInstanceTypes.h"
#include "LevelInstance/LevelInstanceInterface.h"
#include "LevelInstance/LevelInstanceActorImpl.h"
#include "LevelInstance/LevelInstanceActorGuid.h"
#include "LevelInstance/LevelInstancePropertyOverrideAsset.h"
#include "WorldPartition/WorldPartitionActorDesc.h"

#include "QHLevelInstanceActor.generated.h"

class UQHLevelInstanceComponent;
struct FTransform4D;

UCLASS()
class QHGAME_API AQHLevelInstance : public AActor, public ILevelInstanceInterface
{
	GENERATED_BODY()

public:
	AQHLevelInstance();

protected:
#if WITH_EDITORONLY_DATA
	/** Level LevelInstance */
	UPROPERTY(EditAnywhere, Category = Level, Meta = (NoCreate, DisplayName="Level", DisableLevelInstancePropertyOverride))
	TSoftObjectPtr<UWorld> WorldAsset;

	UPROPERTY(VisibleAnywhere, Category = Override, meta = (EditInline, NoResetToDefault, EditCondition="PropertyOverrides != nullptr", EditConditionHides))
	TObjectPtr<ULevelInstancePropertyOverrideAsset> PropertyOverrides;
#endif

	UPROPERTY(VisibleAnywhere, Category = Default)
	TObjectPtr<UQHLevelInstanceComponent> LevelInstanceComponent;

	UPROPERTY(Replicated)
	TSoftObjectPtr<UWorld> CookedWorldAsset;

	UPROPERTY(Transient, Replicated)
	FGuid LevelInstanceSpawnGuid;

	FLevelInstanceActorGuid LevelInstanceActorGuid;
	FLevelInstanceActorImpl LevelInstanceActorImpl;

#if WITH_EDITORONLY_DATA
//Force the usage of SetDesiredRuntimeBehavior to change the behavior
private:
	UPROPERTY(EditAnywhere, Category = Level, AdvancedDisplay, Meta = (DisplayName="Level Behavior", DisableLevelInstancePropertyOverride))
	ELevelInstanceRuntimeBehavior DesiredRuntimeBehavior;
#endif

public:
	// Begin ILevelInstanceInterface
	const FLevelInstanceID& GetLevelInstanceID() const override;
	bool HasValidLevelInstanceID() const override;
	virtual const FGuid& GetLevelInstanceGuid() const override;
	virtual const TSoftObjectPtr<UWorld>& GetWorldAsset() const override;

	virtual void OnLevelInstanceLoaded() override;
	virtual bool IsLoadingEnabled() const override;
	// End ILevelInstanceInterface

	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostNetInit() override;

#if WITH_EDITOR
	// Begin ILevelInstanceInterface
	virtual ILevelInstanceComponentInterface* GetLevelInstanceComponent() const override;
	virtual AActor* CreateLevelInstanceEditorInstance(ILevelInstanceInterface* LevelInstance, ULevel* LoadedLevel) const override;
	virtual bool SetWorldAsset(TSoftObjectPtr<UWorld> WorldAsset) override;
	virtual ELevelInstanceRuntimeBehavior GetDesiredRuntimeBehavior() const override { return DesiredRuntimeBehavior; }
	void SetDesiredRuntimeBehavior(ELevelInstanceRuntimeBehavior NewBehavior) override;
	virtual ELevelInstanceRuntimeBehavior GetDefaultRuntimeBehavior() const override { return ELevelInstanceRuntimeBehavior::Partitioned; }
	virtual TSubclassOf<AActor> GetEditorPivotClass() const override;
	virtual void AdjustPivotOnCreation() override;
	virtual bool SupportsPartialEditorLoading() const override;

	virtual bool SupportsPropertyOverrides() const override;
	ULevelInstancePropertyOverrideAsset* GetPropertyOverrideAsset() const override;
	// End ILevelInstanceInterface

	// UObject overrides
	virtual void PreEditUndo() override;
	virtual void PostEditUndo() override;
	virtual void PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation) override;
	virtual void PostLoad() override;
	virtual void OnCookEvent(UE::Cook::ECookEvent CookEvent, UE::Cook::FCookEventContext& Context) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditImport() override;
	virtual bool ResolveSubobject(const TCHAR* SubObjectPath, UObject*& OutObject, bool bLoadIfExists) override;

	// AActor overrides
	virtual bool CanEditChangeComponent(const UActorComponent* InComponent, const FProperty* InProperty) const override;
	virtual void CheckForErrors() override;
	virtual TUniquePtr<class FWorldPartitionActorDesc> CreateClassActorDesc() const override;
	virtual bool CanDeleteSelectedActor(FText& OutReason) const override;
	virtual void SetIsTemporarilyHiddenInEditor(bool bIsHidden) override;
	virtual bool SetIsHiddenEdLayer(bool bIsHiddenEdLayer) override;
	virtual void EditorGetUnderlyingActors(TSet<AActor*>& OutUnderlyingActors) const override;
	virtual void PushSelectionToProxies() override;
	virtual void PushLevelInstanceEditingStateToProxies(bool bInEditingState) override;
	virtual FBox GetComponentsBoundingBox(bool bNonColliding = false, bool bIncludeFromChildActors = false) const override;
	virtual void GetStreamingBounds(FBox& OutRuntimeBounds, FBox& OutEditorBounds) const override;
	virtual bool IsLockLocation() const override;
	virtual bool IsActorLabelEditable() const override;
	virtual bool GetReferencedContentObjects(TArray<UObject*>& Objects) const override;
	virtual bool GetSoftReferencedContentObjects(TArray<FSoftObjectPath>& SoftObjects) const override;
	virtual bool OpenAssetEditor() override;
	virtual bool EditorCanAttachFrom(const AActor* InChild, FText& OutReason) const override;
	virtual bool IsUserManaged() const override;
	virtual bool ShouldExport() override;
	virtual bool SupportsSubRootSelection() const override { return true; }
	virtual bool IsHLODRelevant() const override;
	virtual bool HasHLODRelevantComponents() const override;
	// End of AActor interface

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLevelInstanceActorPostLoad, AQHLevelInstance*);
	static FOnLevelInstanceActorPostLoad OnLevelInstanceActorPostLoad;

private:
	virtual void SetPropertyOverrideAsset(ULevelInstancePropertyOverrideAsset* InPropertyOverrideAsset) override;
	virtual bool ShouldCookWorldAsset() const;
#endif

	void ApplyLevelTransform4D(ULevel* InLevel, const FTransform4D& InTransform4D) const;
};

//DEFINE_ACTORDESC_TYPE(AQHLevelInstance, FLevelInstanceActorDesc);
