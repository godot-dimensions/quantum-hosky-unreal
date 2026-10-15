#pragma once

#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WorldPartition/Filter/WorldPartitionActorFilter.h"
#include "WorldPartition/WorldPartitionActorContainerID.h"
#include "LevelInstance/LevelInstanceTypes.h"

#include "LevelInstance/LevelInstanceComponentInterface.h"

#include "QHLevelInstanceComponent.generated.h"

/**
 * ULevelInstanceComponent subclasses USceneComponent for Editing purposes so that we can have a proxy to the LevelInstanceActor's RootComponent transform without attaching to it.
 *
 * It is responsible for updating the transform of the ALevelInstanceEditorInstanceActor which is created when loading a LevelInstance Instance Level
 *
 * We use this method to avoid attaching the Instance Level Actors to the ILevelInstanceInterface. (Cross level attachment and undo/redo pain)
 *
 * The LevelInstance Level Actors are attached to this ALevelInstanceEditorInstanceActor keeping the attachment local to the Instance Level and shielded from the transaction buffer.
 *
 * Avoiding those Level Actors from being part of the Transaction system allows us to unload that level without clearing the transaction buffer. It also allows BP Reinstancing without having to update attachements.
 */
UCLASS()
class QHGAME_API UQHLevelInstanceComponent : public UScene4DComponent, public ILevelInstanceComponentInterface
{
	GENERATED_BODY()
public:
	UQHLevelInstanceComponent(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITORONLY_DATA
	virtual void Serialize(FArchive& Ar) override;
#endif
#if WITH_EDITOR
	// Those are the methods that need overriding to be able to properly update the AttachComponent
	virtual void OnRegister() override;
	virtual void PreEditUndo() override;
	virtual void PostEditUndo() override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnTransform4DChanged() override;

	void UpdateEditorInstanceActor();
	void AbandonEditorInstanceActor() { CachedEditorInstanceActorPtr.Reset(); }
	void OnEdit();
	void OnEditOverrides();
	void OnCommit();
	void OnCommitOverrides();

	const FWorldPartitionActorFilter& GetFilter() const { return IsEditFilter() ? EditFilter : Filter; }
	void SetFilter(const FWorldPartitionActorFilter& InFilter, bool bNotify = true);
	const TMap<FActorContainerID, TSet<FGuid>>& GetFilteredActorsPerContainer() const;
	void UpdateEditFilter();
	void ClearCachedFilter();
private:
	bool ShouldShowSpriteComponent() const;
	void OnFilterChanged();
	void SetActiveFilter(const FWorldPartitionActorFilter& InFilter);
	bool IsEditFilter() const;

	friend class FLevelInstanceActorImpl;
	TWeakObjectPtr<AActor> CachedEditorInstanceActorPtr;
#endif

#if WITH_EDITORONLY_DATA
public:
	//Filters are only allowed on Embedded Level Instances Editor\Private LI per design.
	UPROPERTY()
	ELevelInstanceRuntimeBehavior ShadowDesiredRuntimeBehavior;

private:
	//, EditCondition="bFiltersAllowed"))
	UPROPERTY(EditAnywhere, Category = Filter, meta=(LevelInstanceFilter, DisableLevelInstancePropertyOverride, EditCondition = "ShadowDesiredRuntimeBehavior==ELevelInstanceRuntimeBehavior::Partitioned"))
	FWorldPartitionActorFilter Filter;

	UPROPERTY(EditAnywhere, Transient, Category = Filter, meta=(LevelInstanceEditFilter))
	FWorldPartitionActorFilter EditFilter;

	FWorldPartitionActorFilter UndoRedoCachedFilter;

	mutable FWorldPartitionActorFilter CachedFilter;
	mutable TOptional<TMap<FActorContainerID, TSet<FGuid>>> CachedFilteredActorsPerContainer;

	// Used to cancel the package getting dirty when editing the EditFilter which is transient
	bool bWasDirtyBeforeEditFilterChange;
#endif
};
