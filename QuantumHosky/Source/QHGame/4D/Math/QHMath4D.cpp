#include "QHMath4D.h"

#include "Unreal4DCore/Math/Transform4D.h"
#include "QHGame/Utility/QHProfiling.h"

const int32* UQHMath4D::GetTetraEdgeToVertexLUT()
{
	static const int32 TetraEdgeToVertexLUT[12] =
	{
		0, 1,
		0, 2,
		0, 3,
		1, 2,
		1, 3,
		2, 3
	};

	return TetraEdgeToVertexLUT;
}

const int32* UQHMath4D::GetTetraCrossSectionLUT()
{
	static const int32 TetraCrossSectionLUT[16 * 6] =
	{
		-1, -1, -1, -1, -1, -1, // 0000
		 0,  1,  2, -1, -1, -1, // 0001
		 0,  4,  3, -1, -1, -1, // 0010
		 2,  4,  1,  3,  1,  4, // 0011
		 1,  3,  5, -1, -1, -1, // 0100
		 0,  3,  2,  5,  2,  3, // 0101
		 1,  0,  5,  4,  5,  0, // 0110
		 2,  4,  5, -1, -1, -1, // 0111
		 2,  5,  4, -1, -1, -1, // 1000
		 1,  5,  0,  4,  0,  5, // 1001
		 0,  2,  3,  3,  2,  5, // 1010
		 1,  5,  3, -1, -1, -1, // 1011
		 2,  1,  4,  4,  1,  3, // 1100
		 0,  3,  4, -1, -1, -1, // 1101
		 0,  2,  1, -1, -1, -1, // 1110
		-1, -1, -1, -1, -1, -1  // 1111
	};

	return TetraCrossSectionLUT;
}

int32 UQHMath4D::GetTetraSliceMask(const FVector4 InVertices[4])
{
	int32 Result = 0;

	for (int32 i = 0; i < 4; ++i)
	{
		if (InVertices[i].W < 0.0f)
		{
			Result |= (1 << i);
		}
	}

	return Result;
}

#define QH_SLICE_TETRA_DEBUG 0

#if QH_SLICE_TETRA_DEBUG
#define QH_SLICE_TETRA_ENSURE(InPredicate) ensureAlways(InPredicate)
#else
#define QH_SLICE_TETRA_ENSURE(...)
#endif

float UQHMath4D::SliceTetraEdgeW0(const FVector4& InA, const FVector4& InB)
{
	QH_SLICE_TETRA_ENSURE((InA.W < 0.0) != (InB.W < 0.0)); // This should be guaranteed by how the vertices are selected.
	const double denom = (InA.W - InB.W);
	// The following invariants follow from the precondition.
	QH_SLICE_TETRA_ENSURE(denom != 0.0);
	QH_SLICE_TETRA_ENSURE(InA.W == 0.0 || ((denom > 0.0) == (InA.W > 0.0)));
	QH_SLICE_TETRA_ENSURE(FMath::Abs(denom) >= FMath::Abs(InA.W));
	// It's fine if denom is *nearly* 0. The output vertex position will be imprecise, but in that case the problem is genuinely ill-conditioned. It's still important to output a triangle otherwise the mesh may have holes.

	// Perform linear interpolation between the two tetrahedral vertices.
	// This determines how far along the W-axis we slice between the two vertices.
	// Formula equivalent to GLSL: mix_weight = v1.w / (v1.w - v2.w)
	return InA.W / denom;
}

int32 UQHMath4D::SliceTetraW0(const FVector4 InVertices4D[4], FVector (&OutVertices3D)[6], int32 (&OutIndices)[6])
{
	int32 NumTris = 0;

	const int32* TetraEdgeToVertexLUT = UQHMath4D::GetTetraEdgeToVertexLUT();
	const int32* TetraCrossSectionLUT = UQHMath4D::GetTetraCrossSectionLUT();

	const int32 mask = UQHMath4D::GetTetraSliceMask(InVertices4D);
	const int32 lookupBase = mask * 6;

	// Two possible triangles: [0,1,2] and [3,4,5]
	for (int32 triangleIndex = 0; triangleIndex < 2; triangleIndex++)
	{
		const int32 edge0 = TetraCrossSectionLUT[lookupBase + (triangleIndex * 3) + 0];
		const int32 edge1 = TetraCrossSectionLUT[lookupBase + (triangleIndex * 3) + 1];
		const int32 edge2 = TetraCrossSectionLUT[lookupBase + (triangleIndex * 3) + 2];

		if (edge0 < 0 || edge1 < 0 || edge2 < 0)
		{
			continue;
		}

		const int32 vertexA0 = TetraEdgeToVertexLUT[(edge0 * 2) + 0];
		const int32 vertexB0 = TetraEdgeToVertexLUT[(edge0 * 2) + 1];
		const int32 vertexA1 = TetraEdgeToVertexLUT[(edge1 * 2) + 0];
		const int32 vertexB1 = TetraEdgeToVertexLUT[(edge1 * 2) + 1];
		const int32 vertexA2 = TetraEdgeToVertexLUT[(edge2 * 2) + 0];
		const int32 vertexB2 = TetraEdgeToVertexLUT[(edge2 * 2) + 1];

		const float t0 = UQHMath4D::SliceTetraEdgeW0(InVertices4D[vertexA0], InVertices4D[vertexB0]);
		const float t1 = UQHMath4D::SliceTetraEdgeW0(InVertices4D[vertexA1], InVertices4D[vertexB1]);
		const float t2 = UQHMath4D::SliceTetraEdgeW0(InVertices4D[vertexA2], InVertices4D[vertexB2]);

		const FVector4 pos4D0 = FMath::Lerp(InVertices4D[vertexA0], InVertices4D[vertexB0], t0);
		const FVector4 pos4D1 = FMath::Lerp(InVertices4D[vertexA1], InVertices4D[vertexB1], t1);
		const FVector4 pos4D2 = FMath::Lerp(InVertices4D[vertexA2], InVertices4D[vertexB2], t2);

		const int32 i0 = (NumTris * 3) + 0;
		const int32 i1 = (NumTris * 3) + 1;
		const int32 i2 = (NumTris * 3) + 2;

		++NumTris;

		OutVertices3D[i0] = pos4D0;
		OutVertices3D[i1] = pos4D1;
		OutVertices3D[i2] = pos4D2;

		OutIndices[i0] = i0;
		OutIndices[i1] = i1;
		OutIndices[i2] = i2;
	}

	return NumTris;
}

int32 UQHMath4D::SliceTetraMeshW0(const int32 InNumTetra, const FVector4* InTetraVertices, const int32* InTetraIndices, const FTransform4D& InTetraToSliceTransform, TArray<FVector>& OutVertices3D, TArray<int32>& OutIndices)
{
	// TODO: Attempt optimization of 4D CPU mesh slicing via vectorization (past what UE already performs) or compute shaders;
	// https://dev.epicgames.com/community/learning/tutorials/WkwJ/unreal-engine-simple-compute-shader-with-cpu-readback

	QH_PROFILE_FUNCTION();

	int32 NumTris = 0;

	OutVertices3D.Reserve(InNumTetra * 2 * 3); // Conservative reserve; 2 possible tris per-tetra; 3 verts per-tri
	OutIndices.Reserve(InNumTetra * 2 * 3);

	FVector4 TetraVertices[4];
	FVector TriVertices[6];
	int32 TriIndices[6];

	for (int32 i = 0; i < InNumTetra; ++i)
	{
		// Transform tetra to slice-space
		TetraVertices[0] = InTetraToSliceTransform.TransformPosition(InTetraVertices[InTetraIndices[(i * 4) + 0]]);
		TetraVertices[1] = InTetraToSliceTransform.TransformPosition(InTetraVertices[InTetraIndices[(i * 4) + 1]]);
		TetraVertices[2] = InTetraToSliceTransform.TransformPosition(InTetraVertices[InTetraIndices[(i * 4) + 2]]);
		TetraVertices[3] = InTetraToSliceTransform.TransformPosition(InTetraVertices[InTetraIndices[(i * 4) + 3]]);

		// Slice tetra
		const int32 NumTetraTris = UQHMath4D::SliceTetraW0(TetraVertices, TriVertices, TriIndices);

		// Accumulate sliced tris

		OutVertices3D.Append(TriVertices, (NumTetraTris * 3));

		const int32 IndexBase = (NumTris * 3);

		for (int32 j = 0; j < NumTetraTris; ++j)
		{
			OutIndices.Add(TriIndices[(j * 3) + 0] + IndexBase);
			OutIndices.Add(TriIndices[(j * 3) + 1] + IndexBase);
			OutIndices.Add(TriIndices[(j * 3) + 2] + IndexBase);
		}

		NumTris += NumTetraTris;
	}

	return NumTris;
}

#undef QH_SLICE_TETRA_DEBUG
#undef QH_SLICE_TETRA_ENSURE
