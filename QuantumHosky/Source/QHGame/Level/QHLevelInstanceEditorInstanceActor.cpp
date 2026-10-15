#include "QHGame/Level/QHLevelInstanceEditorInstanceActor.h"

#include "LevelInstance/LevelInstanceInterface.h"
#include "LevelInstance/LevelInstanceSubsystem.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Engine/LevelStreaming.h"
#include "LevelUtils.h"
#include "GameFramework/WorldSettings.h"
#include "WorldPartition/WorldPartition.h"

AQHLevelInstanceEditorInstanceActor::AQHLevelInstanceEditorInstanceActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Root4DComponent = CreateDefaultSubobject<UScene4DComponent>(TEXT("Root4DComponent"));
	Root4DComponent->Mobility = EComponentMobility::Static;

	// To keep the behavior of any calls to USceneComponent::GetActorPositionForRenderer() consistent between Editor and Game modes
	// we need to flag the root component so that it isn't considered as an AttachmentRoot.
	Root4DComponent->bIsNotRenderAttachmentRoot = true;

	SetRootComponent(Root4DComponent);
}

#if WITH_EDITOR
AActor* AQHLevelInstanceEditorInstanceActor::GetSelectionParent() const
{
	if (ULevelInstanceSubsystem* LevelInstanceSubsystem = GetWorld()->GetSubsystem<ULevelInstanceSubsystem>())
	{
		if (ILevelInstanceInterface* LevelInstance = LevelInstanceSubsystem->GetLevelInstance(LevelInstanceID))
		{
			return CastChecked<AActor>(LevelInstance);
		}
	}

	return nullptr;
}

AQHLevelInstanceEditorInstanceActor* AQHLevelInstanceEditorInstanceActor::Create(ILevelInstanceInterface* LevelInstance, ULevel* LoadedLevel)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = LoadedLevel;
	SpawnParams.bHideFromSceneOutliner = true;
	SpawnParams.bCreateActorPackage = false;
	SpawnParams.ObjectFlags = RF_Transient;
	SpawnParams.bNoFail = true;

	AActor* LevelInstanceActor = CastChecked<AActor>(LevelInstance);
	UScene4DComponent* LevelInstance4DComponent = LevelInstanceActor->GetComponentByClass<UScene4DComponent>();
	AQHLevelInstanceEditorInstanceActor* InstanceActor = LevelInstanceActor->GetWorld()->SpawnActor<AQHLevelInstanceEditorInstanceActor>(SpawnParams);
	InstanceActor->Root4DComponent->SetGlobalTransform4D(LevelInstance4DComponent->GetGlobalTransform4D());
	InstanceActor->SetLevelInstanceID(LevelInstance->GetLevelInstanceID());

	for (AActor* LevelActor : LoadedLevel->Actors)
	{
		if (LevelActor && LevelActor->GetAttachParentActor() == nullptr && !LevelActor->IsChildActor() && LevelActor != InstanceActor)
		{
			// TODO: QHLevelInstance; need to override `UScene4DComponent::AttachToComponent` and handle attachment transform rules

			UScene4DComponent* Root4DComponent = Cast<UScene4DComponent>(LevelActor->GetRootComponent());
			FTransform4D WorldTransform4D;

			if (Root4DComponent)
			{
				WorldTransform4D = Root4DComponent->GetGlobalTransform4D();
			}

			LevelActor->AttachToActor(InstanceActor, FAttachmentTransformRules::KeepWorldTransform);

			if (Root4DComponent)
			{
				Root4DComponent->SetGlobalTransform4D(WorldTransform4D);
			}
		}
	}

	InstanceActor->PushSelectionToProxies();

	return InstanceActor;
}

void AQHLevelInstanceEditorInstanceActor::UpdateWorldTransform4D(const FTransform4D& InTransform4D)
{
	Root4DComponent->SetGlobalTransform4D(InTransform4D);

	const ULevel* Level = GetLevel();

	ULevelStreaming* LevelStreaming = FLevelUtils::FindStreamingLevel(Level);
	check(LevelStreaming);

	// TODO: QHLevelInstance
	AWorldSettings* WorldSettings = Level->GetWorldSettings();
	check(WorldSettings);
	LevelStreaming->LevelTransform = FTransform(WorldSettings->LevelInstancePivotOffset) * Root4DComponent->GetComponentTransform();

	if (UWorldPartition* WorldPartition = WorldSettings->GetWorldPartition())
	{
		WorldPartition->SetInstanceTransform(LevelStreaming->LevelTransform);
	}
}

#endif
