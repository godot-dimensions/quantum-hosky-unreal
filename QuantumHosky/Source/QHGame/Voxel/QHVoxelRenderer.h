#pragma once

#include "CoreMinimal.h"
#include "QHVoxelWorldComponent.h"

class AQHVoxelWorld;

// Later on, there will be a struct here to keep track of rendering state (which chunks are loaded or need loading/unloading, etc.), but for now there is no state necessary.
namespace FQHVoxelRenderer {
	void CreateMeshes(UQHVoxelWorldComponent* World, const TSharedPtr<FQHVoxelData> Data);
};
