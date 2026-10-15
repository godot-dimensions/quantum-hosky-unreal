#pragma once

#include "Unreal4DCore/Math/Transform4D.h"
#include "Unreal4DCore/Math/Euler4D.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHSlice4.generated.h"

//______________________________________________________________________________
USTRUCT(BlueprintType)
struct FQHSlice4
{
	GENERATED_BODY()

//==============================================================================
private:

	// Origin of the slice in 4D world-space
	UPROPERTY()
	FVector4 Origin4 = FVector4::Zero();

	// Origin of the slice in 3D world-space
	UPROPERTY()
	FVector Origin3 = FVector::ZeroVector;

	UPROPERTY()
	FEuler4D Rotation4;

	UPROPERTY()
	FMatrix Rotation4Matrix = FMatrix::Identity;

	UPROPERTY()
	FMatrix Rotation4MatrixInverse = FMatrix::Identity;

//==============================================================================
public:

	FQHSlice4() = default;

	FQHSlice4(const FVector4& InOrigin4, const FVector& InOrigin3, const FEuler4D& InRotation4);

	//------------------------------------------------------------------------------

	const FVector4& GetOrigin4() const
	{
		return Origin4;
	}

	const FVector& GetOrigin3() const
	{
		return Origin3;
	}

	void SetOrigin(const FVector4& InOrigin4, const FVector& InOrigin3);

	const FEuler4D& GetRotation4() const
	{
		return Rotation4;
	}

	const FMatrix& GetRotation4Matrix() const
	{
		return Rotation4Matrix;
	}

	const FMatrix& GetRotation4MatrixInverse() const
	{
		return Rotation4MatrixInverse;
	}

	void SetRotation4(const FEuler4D& InRotation);

	// Returns the transform that converts the global transforms of 4D objects in this slice
	// into the displayed slice that the camera sees. Note that each object's global transform
	// does not include the slice's transform, because the slice is only needed for rendering.
	FTransform4D GetSliceTransform4D() const;

	FVector4 Slice3ToWorld4(const FVector& InSlice3Position) const;
	FVector Slice3ToWorld3(const FVector& InSlice3Position) const;

	// NOTE: For locations 'outside' of the slice, this will snap the location to be 'inside' the slice (i.e. the W-component will be zeroed).
	FVector World4ToSlice3(const FVector4& InWorld4Position) const;
	FVector4 World4ToSlice4(const FVector4& InWorld4Position) const;

	bool Equals(const FQHSlice4& InOther) const;

	//------------------------------------------------------------------------------

	bool operator==(const FQHSlice4& InOther) const
	{
		return Equals(InOther);
	}

	bool operator!=(const FQHSlice4& InOther) const
	{
		return !(*this == InOther);
	}
};

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "FQHSlice4"))
class UQHSlice4Library
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "GetOrigin4"))
	static const FVector4& Slice4_GetOrigin4(const FQHSlice4& InSlice);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "GetOrigin3"))
	static const FVector& Slice4_GetOrigin3(const FQHSlice4& InSlice);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "SetOrigin"))
	static void Slice4_SetOrigin(UPARAM(ref) FQHSlice4& InSlice, const FVector4& InOrigin4, const FVector& InOrigin3);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "GetRotation4"))
	static const FEuler4D& Slice4_GetRotation4(const FQHSlice4& InSlice);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "GetRotation4Matrix"))
	static const FMatrix& Slice4_GetRotation4Matrix(const FQHSlice4& InSlice);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "SetRotation4"))
	static void Slice4_SetRotation4(UPARAM(ref) FQHSlice4& InSlice, const FEuler4D& InRotation);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "Slice3ToWorld4"))
	static FVector4 Slice4_Slice3ToWorld4(const FQHSlice4& InSlice4, const FVector& InSlice3Position);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (ScriptName = "World4ToSlice3"))
	static FVector Slice4_World4ToSlice3(const FQHSlice4& InSlice4, const FVector4& InWorld4Position);
};
