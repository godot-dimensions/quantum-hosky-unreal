#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHMath.generated.h"

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHConvexHull3
{
	GENERATED_BODY()

	//------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Vertices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> Normals;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Indices;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> VertexRemap;
};

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "FVector4"))
class UQHMathLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	static FQHConvexHull3 CreateConvexHull3(const TArray<FVector>& InVertices, const bool InRemapVertices = false);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (CPP_Default_InClip = "BIG_NUMBER"))
	static FVector Project4To3(const FVector4& InVector4, const float InClip = BIG_NUMBER);
};
