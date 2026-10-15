#pragma once

#include "../../QHDefines.h"

#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"

struct FTransform4D;

// QH CPU tetrahedral mesh slicing helpers.
// General math helpers are provided by Unreal4DCore.
class QHGAME_API UQHMath4D
{
public:
	// CPU 4D -> 3D tetrahedral mesh slicing, used by QH's sliced 3D Chaos collision hulls.
	static const int32* GetTetraEdgeToVertexLUT();
	static const int32* GetTetraCrossSectionLUT();
	static int32 GetTetraSliceMask(const FVector4 InVertices[4]);
	static float SliceTetraEdgeW0(const FVector4& InA, const FVector4& InB);
	static int32 SliceTetraW0(const FVector4 InVertices4D[4], FVector (&OutVertices3D)[6], int32 (&OutIndices)[6]);
	static int32 SliceTetraMeshW0(const int32 InNumTetra, const FVector4* InTetraVertices, const int32* InTetraIndices, const FTransform4D& InTetraToSliceTransform, TArray<FVector>& OutVertices3D, TArray<int32>& OutIndices);
};
