// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#pragma once

#include "CoreMinimal.h"

struct FQHVoxel16treeId
{
	FIntVector4 Position;
	uint8 Height;

	FORCEINLINE bool operator==(const FQHVoxel16treeId& Other) const
	{
		return Position == Other.Position && Height == Other.Height;
	}
	FORCEINLINE bool operator!=(const FQHVoxel16treeId& Other) const
	{
		return Position != Other.Position || Height != Other.Height;
	}
};
