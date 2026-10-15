#pragma once

struct FKConvexElem;
struct FTransform4D;

//------------------------------------------------------------------------------

#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHPhysics.generated.h"

//______________________________________________________________________________
namespace EQHCollisionMaskFilter
{
	enum
	{
		Item = (1 << 0)
	};
}

//______________________________________________________________________________
UCLASS()
class QHGAME_API UQHPhysicsLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	static bool SliceTetraMeshConvexHullW0(const int32 InNumTetra, const FVector4* InTetraVertices, const int32* InTetraIndices, const FTransform4D& InTetraToSliceTransform, FKConvexElem& OutConvex, const FTransform& InSliceToConvexTransform);
};
