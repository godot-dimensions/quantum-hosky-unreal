// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#pragma once

#include "CoreMinimal.h"
#include "QHVoxelDefinitions.h"
#include "QHVoxelData16tree.h"
#include "Unreal4DCore/Math/IntRect4D.h"

class QHGAME_API FQHVoxelData
{
public:
	const int32 Depth;
	const FIntRect4D WorldBounds;

private:
	TUniquePtr<FQHVoxelData16treeParent> Tree;

	FQHVoxelData(int32 InDepth, const FIntRect4D& InWorldBounds);

public:
	static TSharedRef<FQHVoxelData> Create(int32 InDepth, const FIntRect4D& InWorldBounds);
	~FQHVoxelData();

	int32 Size() const
	{
		return QH_DATA_CHUNK_SIZE << Depth;
	}
	FQHVoxelData16treeBase& Get16tree() const
	{
		return *Tree;
	}

	// NOTE: what if we query between WorldBounds.Max - 1 and WorldBounds.Max?
	template<typename T>
	bool IsInWorld(T X, T Y, T Z, T W) const
	{
		return WorldBounds.ContainsTemplate(X, Y, Z, W);
	}
	template<typename T>
	bool IsInWorld(const T& P) const
	{
		return WorldBounds.ContainsTemplate(P);
	}

	template<typename T>
	void ClampToWorld(T& X, T& Y, T& Z, T& W) const
	{
		WorldBounds.Clamp(X, Y, Z, W);
	}
	template<typename T>
	T ClampToWorld(const T& P) const
	{
		return WorldBounds.Clamp(P);
	}

	void Set(int32 X, int32 Y, int32 Z, int32 W, const FQHVoxelValue& Value);

	void Set(const FIntVector4& P, const FQHVoxelValue& Value)
	{
		Set(P.X, P.Y, P.Z, P.W, Value);
	}

	FQHVoxelValue Get(int32 X, int32 Y, int32 Z, int32 W) const;

	FQHVoxelValue Get(const FIntVector4& P) const
	{
		return Get(P.X, P.Y, P.Z, P.W);
	}
};
