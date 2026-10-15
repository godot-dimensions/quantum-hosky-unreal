// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#pragma once

#include "CoreMinimal.h"
#include "QHVoxelDefinitions.h"

class FQHVoxel16treeLeafData
{
	FQHVoxelValue* RESTRICT DataPtr = nullptr;
	FQHVoxelValue SingleValue;
	bool bIsSingleValue = false;

	static constexpr int32 MemorySize = QH_VOXELS_PER_DATA_CHUNK * sizeof(FQHVoxelValue);

public:
	FQHVoxel16treeLeafData() = default;
	~FQHVoxel16treeLeafData()
	{
		// In VoxelPlugin, there's an assertion here that DataPtr is already empty, but I don't see where it's meant to be cleared in that case, and however it works there, it's often the case here that the destructor is the only thing to deallocate the data.
		if (DataPtr)
		{
			ClearData();
		}
	}

	UE_NONCOPYABLE(FQHVoxel16treeLeafData);

public:
	void CreateData()
	{
		check(!HasData());
		Allocate();
		CheckState();
	}
	template<typename TLambda>
	void CreateData(TLambda Init)
	{
		CreateData();
		check(DataPtr);
		Init(static_cast<FQHVoxelValue* RESTRICT>(DataPtr));
	}

	void ClearData()
	{
		CheckState();
		if (DataPtr)
		{
			Deallocate();
		}
		bIsSingleValue = false;
		check(!HasData());
	}

public:
	// Used to determine if it's worth compressing or clearing the cache
	FORCEINLINE bool HasAllocation() const
	{
		return DataPtr != nullptr;
	}
	FORCEINLINE bool HasData() const
	{
		return DataPtr || bIsSingleValue;
	}

public:
	FORCEINLINE FQHVoxelValue Get(int32 Index) const
	{
		check(HasData());
		CheckBounds(Index);
		if (bIsSingleValue)
		{
			return SingleValue;
		}
		else
		{
			return DataPtr[Index];
		}
	}

public:
	FORCEINLINE void PrepareForWrite()
	{
		check(HasData());
		if (bIsSingleValue)
		{
			ExpandSingleValue();
		}
	}
	FORCEINLINE void Set(int32 Index, FQHVoxelValue Value)
	{
		check(DataPtr);
		CheckBounds(Index);
		DataPtr[Index] = Value;
	}

public:
	FORCEINLINE bool IsSingleValue() const
	{
		return bIsSingleValue;
	}
	FORCEINLINE FQHVoxelValue GetSingleValue() const
	{
		check(IsSingleValue());
		return SingleValue;
	}

	void SetSingleValue(FQHVoxelValue InSingleValue)
	{
		check(!DataPtr && !bIsSingleValue);
		bIsSingleValue = true;
		SingleValue = InSingleValue;
	}
	void ExpandSingleValue()
	{
		check(bIsSingleValue);

		bIsSingleValue = false;
		Allocate();
		for (int32 Index = 0; Index < QH_VOXELS_PER_DATA_CHUNK; Index++)
		{
			DataPtr[Index] = SingleValue;
		}
	}
	void TryCompressToSingleValue()
	{
		check(!bIsSingleValue);

		if (!DataPtr)
		{
			return;
		}

		const FQHVoxelValue NewSingleValue = DataPtr[0];
		for (int32 Index = 1; Index < QH_VOXELS_PER_DATA_CHUNK; Index++)
		{
			if (DataPtr[Index] != NewSingleValue) return;
		}

		Deallocate();
		SingleValue = NewSingleValue;
		bIsSingleValue = true;

		CheckState();
	}

private:
	FORCEINLINE void CheckState() const
	{
		check(!(DataPtr && bIsSingleValue));
	}
	FORCEINLINE static void CheckBounds(int32 Index)
	{
		check(0 <= Index && Index < QH_VOXELS_PER_DATA_CHUNK);
	}

private:
	void Allocate()
	{
		check(!DataPtr && !bIsSingleValue);
		DataPtr = static_cast<FQHVoxelValue*>(FMemory::Malloc(MemorySize));
	}
	void Deallocate()
	{
		check(DataPtr);
		FMemory::Free(DataPtr);
		DataPtr = nullptr;
	}
};
