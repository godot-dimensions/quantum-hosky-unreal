#include "QHGame/Level/QHLevelInstanceComponent.h"

#include "QHGame/Level/QHLevelInstanceEditorInstanceActor.h"

#include "LevelInstance/LevelInstanceInterface.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "UObject/ICookInfo.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/Package.h"

#if WITH_EDITORONLY_DATA
static const TCHAR* GLevelSpriteAssetName = TEXT("/Engine/EditorResources/LevelInstance");
#endif

UQHLevelInstanceComponent::UQHLevelInstanceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
#if WITH_EDITORONLY_DATA
	, bWasDirtyBeforeEditFilterChange(false)
#endif
{
#if WITH_EDITOR
	bWantsOnUpdateTransform = true;
#endif
}

#if WITH_EDITORONLY_DATA
void UQHLevelInstanceComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	if (Ar.IsSaving() && Ar.IsObjectReferenceCollector() && !Ar.IsCooking())
	{
		FSoftObjectPathSerializationScope EditorOnlyScope(ESoftObjectPathCollectType::EditorOnlyCollect);
		FSoftObjectPath SpriteAsset(GLevelSpriteAssetName);
		Ar << SpriteAsset;
	}
}
#endif

#if WITH_EDITOR
void UQHLevelInstanceComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	// Prevents USceneComponent from creating the SpriteComponent in OnRegister because we want to provide a different texture and condition
	bVisualizeComponent = false;
#endif

	Super::OnRegister();

#if WITH_EDITORONLY_DATA
	AActor* Owner = GetOwner();
	// Only show Sprite for non-instanced LevelInstances
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		// Re-enable before calling CreateSpriteComponent
		bVisualizeComponent = true;
		FCookLoadScope CookLoadScope(ECookLoadType::EditorOnly);
		CreateSpriteComponent(LoadObject<UTexture2D>(nullptr, GLevelSpriteAssetName), false);
		if (SpriteComponent)
		{
			SpriteComponent->bShowLockedLocation = false;
			SpriteComponent->SetVisibility(ShouldShowSpriteComponent());
			SpriteComponent->RegisterComponent();
		}
	}

	//Initialize the Shadow variable to the parents value. This is for the cases where the ILevelInstanceInterface child classes use a constant behavior
	if (ILevelInstanceInterface* LevelInstance = Cast<ILevelInstanceInterface>(GetOwner()))
	{
		ShadowDesiredRuntimeBehavior = LevelInstance->GetDesiredRuntimeBehavior();
	}
#endif //WITH_EDITORONLY_DATA
}

void UQHLevelInstanceComponent::SetFilter(const FWorldPartitionActorFilter& InFilter, bool bNotify)
{
	if (GetFilter() != InFilter)
	{
		Modify(!IsEditFilter());
		SetActiveFilter(InFilter);
		OnFilterChanged();

		if (bNotify)
		{
			FWorldPartitionActorFilter::GetOnWorldPartitionActorFilterChanged().Broadcast();
		}
	}
}

void UQHLevelInstanceComponent::SetActiveFilter(const FWorldPartitionActorFilter& InFilter)
{
	if (IsEditFilter())
	{
		EditFilter = InFilter;
	}
	else
	{
		Filter = InFilter;
	}
}

void UQHLevelInstanceComponent::OnFilterChanged()
{
	if (ILevelInstanceInterface* LevelInstance = Cast<ILevelInstanceInterface>(GetOwner()))
	{
		LevelInstance->OnFilterChanged();
	}
}

bool UQHLevelInstanceComponent::ShouldShowSpriteComponent() const
{
	return GetOwner() && GetOwner()->GetLevel() && (GetOwner()->GetLevel()->IsPersistentLevel() || !GetOwner()->GetLevel()->IsInstancedLevel());
}

void UQHLevelInstanceComponent::PreEditUndo()
{
	UndoRedoCachedFilter = GetFilter();
}

void UQHLevelInstanceComponent::PostEditUndo()
{
	Super::PostEditUndo();

	UpdateComponentToWorld();
	UpdateEditorInstanceActor();

	if (GetFilter() != UndoRedoCachedFilter)
	{
		OnFilterChanged();
		FWorldPartitionActorFilter::RequestFilterRefresh(false);
		FWorldPartitionActorFilter::GetOnWorldPartitionActorFilterChanged().Broadcast();
	}
	UndoRedoCachedFilter = FWorldPartitionActorFilter();
}

void UQHLevelInstanceComponent::PreEditChange(FProperty* PropertyAboutToChange)
{
	if (PropertyAboutToChange && PropertyAboutToChange->GetFName() == GET_MEMBER_NAME_CHECKED(UQHLevelInstanceComponent, EditFilter))
	{
		bWasDirtyBeforeEditFilterChange = GetPackage()->IsDirty();
	}

	Super::PreEditChange(PropertyAboutToChange);
}

void UQHLevelInstanceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	UpdateEditorInstanceActor();

	const bool FilterProperty = PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UQHLevelInstanceComponent, Filter);
	const bool EditFilterProperty = PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UQHLevelInstanceComponent, EditFilter);

	if (FilterProperty || EditFilterProperty)
	{
		OnFilterChanged();
	}

	if (EditFilterProperty && !bWasDirtyBeforeEditFilterChange)
	{
		GetPackage()->SetDirtyFlag(false);
	}
}

void UQHLevelInstanceComponent::OnTransform4DChanged()
{
	Super::OnTransform4DChanged();

	UpdateEditorInstanceActor();
}

void UQHLevelInstanceComponent::UpdateEditorInstanceActor()
{
	if (!CachedEditorInstanceActorPtr.IsValid())
	{
		if (ILevelInstanceInterface* LevelInstance = Cast<ILevelInstanceInterface>(GetOwner()))
		{
			if (ULevelInstanceSubsystem* LevelInstanceSubsystem = LevelInstance->GetLevelInstanceSubsystem())
			{
				if (LevelInstanceSubsystem->IsLoaded(LevelInstance))
				{
					LevelInstanceSubsystem->ForEachActorInLevelInstance(LevelInstance, [this, LevelInstance](AActor* LevelActor)
					{
						if (AQHLevelInstanceEditorInstanceActor* LevelInstanceEditorInstanceActor = Cast<AQHLevelInstanceEditorInstanceActor>(LevelActor))
						{
							check(LevelInstanceEditorInstanceActor->GetLevelInstanceID() == LevelInstance->GetLevelInstanceID());
							CachedEditorInstanceActorPtr = LevelInstanceEditorInstanceActor;
							return false;
						}
						return true;
					});
				}
			}
		}
	}

	// TODO: Change to 4D update
	if (AQHLevelInstanceEditorInstanceActor* EditorInstanceActor = Cast<AQHLevelInstanceEditorInstanceActor>(CachedEditorInstanceActorPtr.Get()))
	{
		EditorInstanceActor->UpdateWorldTransform4D(GetGlobalTransform4D());
	}
}

void UQHLevelInstanceComponent::OnEdit()
{
	if (SpriteComponent)
	{
		SpriteComponent->SetVisibility(false);
	}
}

void UQHLevelInstanceComponent::OnEditOverrides()
{
	OnEdit();
}

void UQHLevelInstanceComponent::OnCommit()
{
	if (SpriteComponent)
	{
		SpriteComponent->SetVisibility(ShouldShowSpriteComponent());
	}
}

void UQHLevelInstanceComponent::OnCommitOverrides()
{
	OnCommit();
}

void UQHLevelInstanceComponent::UpdateEditFilter()
{
	// @todo_ow: for child level instances setup the proper filter based on parent hierarchy filter
	EditFilter = Filter;
}

void UQHLevelInstanceComponent::ClearCachedFilter()
{
	CachedFilteredActorsPerContainer.Reset();
}

bool UQHLevelInstanceComponent::IsEditFilter() const
{
	if (ILevelInstanceInterface* LevelInstance = Cast<ILevelInstanceInterface>(GetOwner()))
	{
		return LevelInstance->IsEditing();
	}

	return false;
}

const TMap<FActorContainerID, TSet<FGuid>>& UQHLevelInstanceComponent::GetFilteredActorsPerContainer() const
{
	const FWorldPartitionActorFilter& CurrentFilter = GetFilter();
	if (CachedFilter != CurrentFilter)
	{
		CachedFilteredActorsPerContainer.Reset();
	}

	if (CachedFilteredActorsPerContainer.IsSet())
	{
		return CachedFilteredActorsPerContainer.GetValue();
	}

	TMap<FActorContainerID, TSet<FGuid>> FilteredActors;
	if (ILevelInstanceInterface* LevelInstance = Cast<ILevelInstanceInterface>(GetOwner()))
	{
		// Fill Container Instance Filter
		UWorldPartitionSubsystem* WorldPartitionSubsystem = GetWorld()->GetSubsystem<UWorldPartitionSubsystem>();
		check(WorldPartitionSubsystem);

		FilteredActors = WorldPartitionSubsystem->GetFilteredActorsPerContainer(LevelInstance->GetLevelInstanceID().GetContainerID(), LevelInstance->GetWorldAssetPackage(), CurrentFilter, LevelInstance->GetLoadingFilterTypes());
	}

	CachedFilteredActorsPerContainer = MoveTemp(FilteredActors);
	CachedFilter = CurrentFilter;
	return CachedFilteredActorsPerContainer.GetValue();
}

#endif
