#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QHVoxelWorldComponent.h"
#include "QHVoxelWorld.generated.h"

/**
 * The actor, of which there should usually be one per scene, containing all the voxel data and associated settings.
 */
UCLASS()
class QHGAME_API AQHVoxelWorld : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(Category = "Voxel", VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UQHVoxelWorldComponent> WorldRoot;

public:

	AQHVoxelWorld(const FObjectInitializer& Init);
};
