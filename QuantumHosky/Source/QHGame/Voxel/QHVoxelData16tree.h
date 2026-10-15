// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#pragma once

#include "CoreMinimal.h"
#include "QHVoxelDefinitions.h"
#include "QHVoxel16treeId.h"
#include "QHVoxel16treeLeafData.h"
#include "Unreal4DCore/Math/IntRect4D.h"

class FQHVoxelData16treeParent;
class FQHVoxelData16treeLeaf;

namespace FQHVoxelData16treeUtilities
{
	FORCEINLINE FQHVoxelCellIndex IndexFromCoordinates(int32 X, int32 Y, int32 Z, int32 W)
	{
		return X + QH_DATA_CHUNK_SIZE * (Y + QH_DATA_CHUNK_SIZE * (Z + QH_DATA_CHUNK_SIZE * W));
	}
	FORCEINLINE FIntVector4 CoordinatesFromIndex(FQHVoxelCellIndex Index)
	{
		return
		{
			Index % QH_DATA_CHUNK_SIZE,
			(Index / QH_DATA_CHUNK_SIZE) % QH_DATA_CHUNK_SIZE,
			(Index / (QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE)) % QH_DATA_CHUNK_SIZE,
			(Index / (QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE))
		};
	}
	FORCEINLINE FQHVoxelCellIndex IndexFromGlobalCoordinates(const FIntVector4& Min, int32 X, int32 Y, int32 Z, int32 W)
	{
		X -= Min.X;
		Y -= Min.Y;
		Z -= Min.Z;
		W -= Min.W;
		return IndexFromCoordinates(X, Y, Z, W);
	}
	/**
	 * It is safe to create children in Apply
	 */
	template<typename T, typename F1>
	void IterateEntireTree(T& Tree, F1 Apply)
	{
		Apply(Tree);

		if (!Tree.IsLeaf() && Tree.AsParent().HasChildren())
		{
			for (auto& Child : Tree.AsParent().GetChildren())
			{
				IterateEntireTree(Child, Apply);
			}
		}
	}
}

class FQHVoxelData16treeBase
{
public:
	// Height of the 16tree (distance to smallest possible leaf)
	const uint8 Height;

	// Center of the 16tree
	const FIntVector4 Position;

	FQHVoxelData16treeBase(uint8 Height, const FIntVector4& Position)
		: Height(Height)
		, Position(Position)
	{
		check(Height < 32);
	}
	~FQHVoxelData16treeBase() = default;

public:
	FORCEINLINE uint32 GetSize() const
	{
		return QH_DATA_CHUNK_SIZE << Height;
	}
	FORCEINLINE uint32 GetHalfSize() const
	{
		return (QH_DATA_CHUNK_SIZE / 2) << Height;
	}
	FORCEINLINE FIntVector4 GetMin() const
	{
		return Position - FIntVector4(GetHalfSize());
	}
	FORCEINLINE FIntVector4 GetMax() const
	{
		return Position + FIntVector4(GetHalfSize());
	}
	FORCEINLINE FIntRect4D GetBounds() const
	{
		return FIntRect4D(GetMin(), GetMax());
	}
	FORCEINLINE bool IsIn16tree(int32 X, int32 Y, int32 Z, int32 W) const
	{
		return GetBounds().Contains(X, Y, Z, W);
	}
	FORCEINLINE bool IsIn16tree(const FIntVector4& P) const
	{
		return IsIn16tree(P.X, P.Y, P.Z, P.W);
	}
	FORCEINLINE bool IsLeaf() const
	{
		return Height == 0;
	}
	FORCEINLINE FQHVoxel16treeId GetId() const
	{
		return { Position, Height };
	}

protected:
	FORCEINLINE static FIntVector4 GetChildPosition(const FIntVector4& ParentPosition, uint32 ParentSize, uint8 ChildIndex)
	{
		return ParentPosition +
			FIntVector4(
				ParentSize / 4 * ((ChildIndex & 0x1) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x2) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x4) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x8) ? 1 : -1));
	}

public:
	FQHVoxelData16treeLeaf& GetBottomNode(int32 X, int32 Y, int32 Z, int32 W);

	FQHVoxelData16treeParent& AsParent();
	const FQHVoxelData16treeParent& AsParent() const;

	FQHVoxelData16treeLeaf& AsLeaf();
	const FQHVoxelData16treeLeaf& AsLeaf() const;

	bool IsLeafOrHasNoChildren() const;

	friend class FQHVoxelData16treeParent;
};

class FQHVoxelData16treeLeaf : public FQHVoxelData16treeBase
{
public:
	FQHVoxelData16treeLeaf(const FQHVoxelData16treeBase& Parent, uint8 ChildIndex)
		: FQHVoxelData16treeBase(Parent.Height - 1, this->GetChildPosition(Parent.Position, Parent.GetSize(), ChildIndex))
	{
		check(0 <= ChildIndex && ChildIndex < 16);
	}

	FQHVoxel16treeLeafData Values;

public:
	FORCEINLINE       FQHVoxel16treeLeafData& GetData()       { return Values; }
	FORCEINLINE const FQHVoxel16treeLeafData& GetData() const { return Values; }

	FQHVoxelValue Get(int32 X, int32 Y, int32 Z, int32 W) {
		return Values.Get(FQHVoxelData16treeUtilities::IndexFromGlobalCoordinates(GetMin(), X, Y, Z, W));
	}

	void Set(int32 X, int32 Y, int32 Z, int32 W, FQHVoxelValue Value) {
		FQHVoxelCellIndex Index = FQHVoxelData16treeUtilities::IndexFromGlobalCoordinates(GetMin(), X, Y, Z, W);
		if (Values.Get(Index) != Value) {
			Values.PrepareForWrite();
			Values.Set(Index, Value);
		}
	}
};


class FQHVoxelData16treeParent : public FQHVoxelData16treeBase
{
public:
	template<typename T>
	struct FChildrenIterator
	{
		void* const Children;
		bool const bIsLeaf;

		FChildrenIterator(void* InChildren, bool bInIsLeaf)
			: Children(InChildren)
			, bIsLeaf(bInIsLeaf)
		{
		}

		struct FIterator
		{
			void* const VoidPtr;
			bool const bIsLeaf;
			uint32 Index = 0;

			FIterator(void* Ptr, bool bIsLeaf)
				: VoidPtr(Ptr)
				, bIsLeaf(bIsLeaf)
			{
			}

			T& operator*() const
			{
				if (bIsLeaf)
				{
					FQHVoxelData16treeLeaf* Ptr = reinterpret_cast<FQHVoxelData16treeLeaf*>(VoidPtr);
					return static_cast<T&>(Ptr[Index]);
				}
				else
				{
					FQHVoxelData16treeParent* Ptr = reinterpret_cast<FQHVoxelData16treeParent*>(VoidPtr);
					return static_cast<T&>(Ptr[Index]);
				}
			}
			void operator++()
			{
				Index++;
			}
			bool operator!=(const FIterator&) const
			{
				return Index != 16;
			}
		};

		FIterator begin() { return FIterator(Children, bIsLeaf); }
		FIterator end() { return FIterator(nullptr, false); }
	};

public:
	FQHVoxelData16treeParent(uint8 Height)
		: FQHVoxelData16treeBase(Height, FIntVector4(0, 0, 0, 0))
	{
	}
	~FQHVoxelData16treeParent()
	{
		if (HasChildren())
		{
			DestroyChildren();
		}
	}

public:
	inline bool HasChildren() const
	{
		return Children != nullptr;
	}

public:
	inline const FQHVoxelData16treeBase& GetChild(int32 X, int32 Y, int32 Z, int32 W) const
	{
		return GetChild(GetChildIndex(X, Y, Z, W));
	}
	inline FQHVoxelData16treeBase& GetChild(int32 X, int32 Y, int32 Z, int32 W)
	{
		return GetChild(GetChildIndex(X, Y, Z, W));
	}

	inline const FQHVoxelData16treeBase& GetChild(const FIntVector4& P) const
	{
		return GetChild(P.X, P.Y, P.Z, P.W);
	}
	inline FQHVoxelData16treeBase& GetChild(const FIntVector4& P)
	{
		return GetChild(P.X, P.Y, P.Z, P.W);
	}

	inline const FQHVoxelData16treeBase& GetChild(int32 Index) const
	{
		check((Children != nullptr) & (0 <= Index) & (Index < 16));
		if (this->Height == 1)
		{
			const FQHVoxelData16treeLeaf* Ptr = reinterpret_cast<const FQHVoxelData16treeLeaf*>(Children);
			return static_cast<const FQHVoxelData16treeBase&>(Ptr[Index]);
		}
		else
		{
			const FQHVoxelData16treeParent* Ptr = reinterpret_cast<const FQHVoxelData16treeParent*>(Children);
			return static_cast<const FQHVoxelData16treeBase&>(Ptr[Index]);
		}
	}
	inline FQHVoxelData16treeBase& GetChild(int32 Index)
	{
		check((Children != nullptr) & (0 <= Index) & (Index < 16));
		if (this->Height == 1)
		{
			FQHVoxelData16treeLeaf* Ptr = reinterpret_cast<FQHVoxelData16treeLeaf*>(Children);
			return static_cast<FQHVoxelData16treeBase&>(Ptr[Index]);
		}
		else
		{
			FQHVoxelData16treeParent* Ptr = reinterpret_cast<FQHVoxelData16treeParent*>(Children);
			return static_cast<FQHVoxelData16treeBase&>(Ptr[Index]);
		}
	}

	inline FChildrenIterator<const FQHVoxelData16treeBase> GetChildren() const
	{
		check(Children);
		return { Children, this->Height == 1 };
	}
	inline FChildrenIterator<FQHVoxelData16treeBase> GetChildren()
	{
		check(Children);
		return { Children, this->Height == 1 };
	}

protected:
	FQHVoxelData16treeParent(const FQHVoxelData16treeParent& Parent, uint8 ChildIndex)
		: FQHVoxelData16treeBase(Parent.Height - 1, this->GetChildPosition(Parent.Position, Parent.GetSize(), ChildIndex))
	{
		check(0 <= ChildIndex && ChildIndex < 16);
	}

public:
	template<typename... TArgs>
	void CreateChildren(TArgs&&... Args)
	{
		check(!HasChildren() && this->Height > 0);

		Children = FMemory::Malloc(16 * (this->Height == 1 ? sizeof(FQHVoxelData16treeLeaf) : sizeof(FQHVoxelData16treeParent)));

		for (int32 Index = 0; Index < 16 ; Index++)
		{
			if (this->Height == 1)
			{
				FQHVoxelData16treeLeaf* Ptr = static_cast<FQHVoxelData16treeLeaf*>(Children);
				new (&Ptr[Index]) FQHVoxelData16treeLeaf (static_cast<const FQHVoxelData16treeParent&>(*this), Index, Forward<TArgs>(Args)...);
			}
			else
			{
				FQHVoxelData16treeParent* Ptr = static_cast<FQHVoxelData16treeParent*>(Children);
				new (&Ptr[Index]) FQHVoxelData16treeParent(static_cast<const FQHVoxelData16treeParent&>(*this), Index, Forward<TArgs>(Args)...);
			}
		}
	}
	inline void DestroyChildren()
	{
		check(HasChildren());

		for (auto& Child : GetChildren())
		{
			if (this->Height == 1)
			{
				static_cast<FQHVoxelData16treeLeaf&>(Child).~FQHVoxelData16treeLeaf();
			}
			else
			{
				static_cast<FQHVoxelData16treeParent&>(Child).~FQHVoxelData16treeParent();
			}
		}

		FMemory::Free(Children);
		Children = nullptr;
	}

private:
	void* Children = nullptr;

	inline uint32 GetChildIndex(int32 X, int32 Y, int32 Z, int32 W) const
	{
		return (X >= this->Position.X) + 2 * (Y >= this->Position.Y) + 4 * (Z >= this->Position.Z) + 8 * (W >= this->Position.W);
	}
};

FORCEINLINE FQHVoxelData16treeParent& FQHVoxelData16treeBase::AsParent()
{
	check(!IsLeaf());
	return static_cast<FQHVoxelData16treeParent&>(*this);
}

FORCEINLINE const FQHVoxelData16treeParent& FQHVoxelData16treeBase::AsParent() const
{
	check(!IsLeaf());
	return static_cast<const FQHVoxelData16treeParent&>(*this);
}

FORCEINLINE FQHVoxelData16treeLeaf& FQHVoxelData16treeBase::AsLeaf()
{
	check(IsLeaf());
	return static_cast<FQHVoxelData16treeLeaf&>(*this);
}

FORCEINLINE const FQHVoxelData16treeLeaf& FQHVoxelData16treeBase::AsLeaf() const
{
	check(IsLeaf());
	return static_cast<const FQHVoxelData16treeLeaf&>(*this);
}

FORCEINLINE FQHVoxelData16treeLeaf& FQHVoxelData16treeBase::GetBottomNode(int32 X, int32 Y, int32 Z, int32 W) {
	auto* Tree = this;
	while (!Tree->IsLeaf()) {
		Tree = &Tree->AsParent().GetChild(X, Y, Z, W);
	}
	return Tree->AsLeaf();
}
