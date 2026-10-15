// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#include "QHVoxelData.h"
#include "QHVoxelData16tree.h"

FQHVoxelData::FQHVoxelData(int32 InDepth, const FIntRect4D& InWorldBounds) :
	Depth(InDepth),
	WorldBounds(InWorldBounds),
	Tree(MakeUnique<FQHVoxelData16treeParent>(InDepth))
	{}

// An arbitrary simple lumpy landscape, just so there's something to display.
static FQHVoxelValue FixedTestData(const FIntVector4& Position) {
	return 0.0 >
		Position.Z
		+ 5.0 * FMath::Sin(Position.X / 5.0)
		+ 6.0 * FMath::Sin(Position.Y / 6.0)
		+ 9.0 * FMath::Sin(Position.W / 9.0);
}

TSharedRef<FQHVoxelData> FQHVoxelData::Create(int32 InDepth, const FIntRect4D& InWorldBounds)
{
	FQHVoxelData* Data = new FQHVoxelData(InDepth, InWorldBounds);
	FQHVoxelData16treeUtilities::IterateEntireTree(Data->Get16tree(), [&](FQHVoxelData16treeBase& Chunk)
	{
		// Later, this will be restricted to only create a few initial nodes, but for now the system to create new chunks dynamically hasn't been set up yet so just create them all at once.
		if (!Chunk.IsLeaf())
		{
			Chunk.AsParent().CreateChildren();
		}
		else
		{
			FQHVoxelData16treeLeaf& Leaf = Chunk.AsLeaf();
			Leaf.GetData().CreateData([&](FQHVoxelValue* RESTRICT DataPtr)
			{
				for (int32 Index = 0; Index < QH_VOXELS_PER_DATA_CHUNK; Index++) {
					DataPtr[Index] = FixedTestData(FQHVoxelData16treeUtilities::CoordinatesFromIndex(Index) + Leaf.GetMin());
				}
			});
		}
	});
	return MakeShareable(Data);
}

FQHVoxelData::~FQHVoxelData()
{
	// Voxel Plugin has ClearData here. It doesn't look like that's actually necessary, but I'm not totally sure.
}

FQHVoxelValue FQHVoxelData::Get(int32 X, int32 Y, int32 Z, int32 W) const
{
	// Clamp to world, to avoid un-editable border
	ClampToWorld(X, Y, Z, W);

	FQHVoxelData16treeLeaf& Node = Get16tree().GetBottomNode(X, Y, Z, W);
	return Node.Get(X, Y, Z, W);
}

void FQHVoxelData::Set(int32 X, int32 Y, int32 Z, int32 W, const FQHVoxelValue& Value)
{
	if (IsInWorld(X, Y, Z, W))
	{
		FQHVoxelData16treeLeaf& Node = Get16tree().GetBottomNode(X, Y, Z, W);
		Node.Set(X, Y, Z, W, Value);
	}
}
