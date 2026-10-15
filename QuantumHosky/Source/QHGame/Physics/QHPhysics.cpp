#include "QHGame/Physics/QHPhysics.h"

#include "QHGame/4D/Math/QHMath4D.h"
#include "Unreal4DCore/Math/Transform4D.h"
#include "QHGame/Utility/QHProfiling.h"

#include "PhysicsEngine/ConvexElem.h"

////////////////////////////////////////////////////////////////////////////////
bool UQHPhysicsLibrary::SliceTetraMeshConvexHullW0(const int32 InNumTetra, const FVector4* InTetraVertices, const int32* InTetraIndices, const FTransform4D& InTetraToSliceTransform, FKConvexElem& OutConvex, const FTransform& InSliceToConvexTransform)
{
	QH_PROFILE_FUNCTION();

	// Slice tetra-mesh
	const int32 NumTris = UQHMath4D::SliceTetraMeshW0(InNumTetra, InTetraVertices, InTetraIndices, InTetraToSliceTransform, OutConvex.VertexData, OutConvex.IndexData);

	// Transform sliced tris to convex-space
	for (FVector& Vertex : OutConvex.VertexData)
	{
		Vertex = InSliceToConvexTransform.TransformPosition(Vertex);
	}

	return (NumTris > 0);
}
