#include "QHMath.h"

#include "Chaos/CollisionConvexMesh.h"
#include "Misc/MemStack.h"

////////////////////////////////////////////////////////////////////////////////
FQHConvexHull3 UQHMathLibrary::CreateConvexHull3(const TArray<FVector>& InVertices, const bool InRemapVertices /*= false*/)
{
	// TODO: Author optimized, stack-allocated hull-creation routine.

	FQHConvexHull3 Result;

	TArray<Chaos::FConvexBuilder::FVec3Type> ChaosInVertices;
	ChaosInVertices.SetNumUninitialized(InVertices.Num());

	for (int32 i = 0, end = InVertices.Num(); i < end; ++i)
	{
		ChaosInVertices[i] = InVertices[i];
	}

	// Build hull

	TArray<Chaos::FConvexBuilder::FPlaneType> Planes;
	TArray<TArray<int32>> Faces;
	TArray<Chaos::FConvexBuilder::FVec3Type> ChaosOutVertices;
	Chaos::FConvexBuilder::FAABB3Type Bounds;
	Chaos::FConvexBuilder::Build(ChaosInVertices, Planes, Faces, ChaosOutVertices, Bounds, Chaos::FConvexBuilder::EBuildMethod::ConvexHull3Simplified); // Simplify faces for vertex stability

	ensureAlways(Planes.Num() == Faces.Num());

	// Output vertices

	Result.Vertices.SetNumUninitialized(ChaosOutVertices.Num());

	for (int32 i = 0, end = ChaosOutVertices.Num(); i < end; ++i)
	{
		const Chaos::FConvexBuilder::FVec3Type& Vertex = ChaosOutVertices[i];

		Result.Vertices[i] = FVector(Vertex[0], Vertex[1], Vertex[2]);
	}

	Result.Normals.SetNumZeroed(ChaosOutVertices.Num());
	Result.Indices.Reserve(Faces.Num() * 2 * 3); // Minimum 2-triangles per-face; 3 vertices per-triangle

	for (int32 i = 0, iEnd = Faces.Num(); i < iEnd ; ++i)
	{
		const TArray<int32>& Face = Faces[i];

		// Triangulate face
		for (int32 j = 0, jEnd = (Face.Num() - 2); j < jEnd; ++j)
		{
			const int32 Tri0 = Face[0];
			const int32 Tri1 = Face[j + 1];
			const int32 Tri2 = Face[j + 2];

			Result.Indices.Add(Tri0);
			Result.Indices.Add(Tri1);
			Result.Indices.Add(Tri2);
		}

		// Accumulate per-face vertex normals

		const Chaos::FConvexBuilder::FPlaneType& Plane = Planes[i];
		const Chaos::FVec3& PlaneNormal = Plane.Normal();
		const FVector FaceNormal = FVector(PlaneNormal[0], PlaneNormal[1], PlaneNormal[2]);

		for (int32 j = 0, jEnd = Face.Num(); j < jEnd; ++j)
		{
			Result.Normals[Face[j]] += FaceNormal;
		}
	}

	// Normalize accumulated vertex normals
	for (FVector& Normal : Result.Normals)
	{
		Normal.Normalize();
	}

	// Map input vertices to output vertices
	if (InRemapVertices)
	{
		Result.VertexRemap.SetNumUninitialized(ChaosOutVertices.Num());

		for (int32 i = 0, end = ChaosOutVertices.Num(); i < end; ++i)
		{
			Result.VertexRemap[i] = ChaosInVertices.Find(ChaosOutVertices[i]);
		}
	}

	return Result;
}

////////////////////////////////////////////////////////////////////////////////
FVector UQHMathLibrary::Project4To3(const FVector4& InVector4, const float InClip /*= BIG_NUMBER*/)
{
	// https://math.stackexchange.com/a/2286226

	return (InVector4.W < InClip)
		 ? (FVector(InVector4.X, InVector4.Y, InVector4.Z) * (InClip / (InClip - InVector4.W)))
		 : FVector::ZeroVector;
}
