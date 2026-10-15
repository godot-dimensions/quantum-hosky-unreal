#pragma once

#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "GameFramework/Actor.h"
#include "LevelInstance/LevelInstanceTypes.h"

#include "QHLevelInstanceEditorInstanceActor.generated.h"

class ILevelInstanceInterface;
class ULevel;

/**
 * Editor Only Actor that is spawned inside every LevelInstance Instance Level so that we can update its Actor Transforms through the ILevelInstanceInterface's (ULevelInstanceComponent)
 * @see ULevelInstanceComponent
 */
UCLASS(transient, notplaceable)
class QHGAME_API AQHLevelInstanceEditorInstanceActor : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UScene4DComponent> Root4DComponent = nullptr;

public:
	AQHLevelInstanceEditorInstanceActor(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	static AQHLevelInstanceEditorInstanceActor* Create(ILevelInstanceInterface* LevelInstance, ULevel* LoadedLevel);

	void SetLevelInstanceID(const FLevelInstanceID& InLevelInstanceID) { LevelInstanceID = InLevelInstanceID; }
	const FLevelInstanceID& GetLevelInstanceID() const { return LevelInstanceID; }

	virtual bool IsSelectionParentOfAttachedActors() const override { return true; }
	virtual bool IsSelectionChild() const override { return true; }
	virtual AActor* GetSelectionParent() const override;

private:
	virtual bool ActorTypeIsMainWorldOnly() const override { return true; }
	friend class UQHLevelInstanceComponent;
	void UpdateWorldTransform4D(const FTransform4D& InTransform4D);

	FLevelInstanceID LevelInstanceID;
#endif
};
