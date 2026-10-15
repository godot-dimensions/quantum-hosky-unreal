// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#pragma once

#include "CoreMinimal.h"
#include "QHVoxelDefinitions.h"
#include "Unreal4DCore/Math/IntRect4D.h"

namespace FQHVoxelUtilities
{
	FORCEINLINE FIntVector4 Abs(const FIntVector4& Vector)
	{
		return FIntVector4(
			FMath::Abs(Vector.X),
			FMath::Abs(Vector.Y),
			FMath::Abs(Vector.Z),
			FMath::Abs(Vector.W));
	}

	template<uint32 ChunkSize>
	inline FIntRect4D GetBoundsFromDepth(int32 Depth)
	{
		const int32 Size = (ChunkSize << Depth) / 2;
		return FIntRect4D(-Size, Size);
	}

	// Get required depth such that ChunkSize << Depth >= Size
	template<uint32 ChunkSize>
	inline int32 GetDepthFromSize(uint32 Size)
	{
		if (Size <= 0)
		{
			return 0;
		}
		else
		{
			const int32 Depth = 31 - FPlatformMath::CountLeadingZeros(Size / ChunkSize);
			if (ChunkSize << Depth == Size)
			{
				return Depth;
			}
			else
			{
				return Depth + 1;
			}
		}
	}

	// Valid for root node only
	template<uint32 ChunkSize>
	inline int32 Get16treeDepthContainingBounds(const FIntRect4D& Bounds)
	{
		const uint32 Max = FMath::Max(Abs(Bounds.Min).GetMax(), Abs(Bounds.Max).GetMax());
		return GetDepthFromSize<ChunkSize>(2 * Max); // 2x: 16tree doesn't start at 0 0 0
	}
}
