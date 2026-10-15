#pragma once

#include "Unreal4DCore/Model/Mesh/Tetra/TetraMesh4D.h"
#include "QHVoxelData.h"

namespace FQHVoxelHypercubeMesher {
	UTetraMesh4D* GenerateMesh(FIntRect4D ChunkBounds, const TSharedPtr<FQHVoxelData> Data, float VoxelSize);
}
