#include "QHSlice4.h"

////////////////////////////////////////////////////////////////////////////////
FQHSlice4::FQHSlice4(const FVector4& InOrigin4, const FVector& InOrigin3, const FEuler4D& InRotation4)
	: Origin4(InOrigin4)
	, Origin3(InOrigin3)
	, Rotation4(InRotation4)
	, Rotation4Matrix(InRotation4.RadiansToBasis())
	, Rotation4MatrixInverse(Rotation4Matrix.Inverse())
{
}

////////////////////////////////////////////////////////////////////////////////
void FQHSlice4::SetOrigin(const FVector4& InOrigin4, const FVector& InOrigin3)
{
	Origin4 = InOrigin4;
	Origin3 = InOrigin3;
}

////////////////////////////////////////////////////////////////////////////////
void FQHSlice4::SetRotation4(const FEuler4D& InRotation)
{
	Rotation4 = InRotation;
	Rotation4Matrix = Rotation4.RadiansToBasis();
	Rotation4MatrixInverse = Rotation4Matrix.Inverse();
}

FTransform4D FQHSlice4::GetSliceTransform4D() const
{
	const FVector4 sliceOrigin3As4 = FVector4(Origin3, 0.0);
	const FVector4 transformOrigin = sliceOrigin3As4 - Rotation4MatrixInverse.TransformFVector4(Origin4);
	// This single transform performs the equivalent of `Out = Rotation4MatrixInverse * (WorldPos - Origin4) + [Origin3, 0]` but with one origin instead of two.
	// `Rotation4MatrixInverse` is correct because this is what's used in `World4ToSlice3()`.
	// TODO: We may want to reconsider the `Rotation4MatrixInverse` name in the future to avoid confusion, the other one should be named inverse instead.
	return FTransform4D(Rotation4MatrixInverse, transformOrigin);
}

////////////////////////////////////////////////////////////////////////////////
FVector4 FQHSlice4::Slice3ToWorld4(const FVector& InSlice3Position) const
{
	return Origin4
		 + Rotation4Matrix.TransformFVector4(FVector4((InSlice3Position - Origin3), 0.0f));
}

////////////////////////////////////////////////////////////////////////////////
FVector FQHSlice4::Slice3ToWorld3(const FVector& InSlice3Position) const
{
	return (Origin3 + InSlice3Position);
}

////////////////////////////////////////////////////////////////////////////////
FVector FQHSlice4::World4ToSlice3(const FVector4& InWorld4Position) const
{
	return Origin3
		 + FVector(World4ToSlice4(InWorld4Position));
}

////////////////////////////////////////////////////////////////////////////////
FVector4 FQHSlice4::World4ToSlice4(const FVector4& InWorld4Position) const
{
	return Rotation4MatrixInverse.TransformFVector4(InWorld4Position - Origin4);
}

////////////////////////////////////////////////////////////////////////////////
bool FQHSlice4::Equals(const FQHSlice4& InOther) const
{
	return (Origin4 == InOther.Origin4)
		&& (Origin3 == InOther.Origin3)
		&& (Rotation4 == InOther.Rotation4);
}

////////////////////////////////////////////////////////////////////////////////
const FVector4& UQHSlice4Library::Slice4_GetOrigin4(const FQHSlice4& InSlice)
{
	return InSlice.GetOrigin4();
}

////////////////////////////////////////////////////////////////////////////////
const FVector& UQHSlice4Library::Slice4_GetOrigin3(const FQHSlice4& InSlice)
{
	return InSlice.GetOrigin3();
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Library::Slice4_SetOrigin(FQHSlice4& InSlice, const FVector4& InOrigin4, const FVector& InOrigin3)
{
	InSlice.SetOrigin(InOrigin4, InOrigin3);
}

////////////////////////////////////////////////////////////////////////////////
const FEuler4D& UQHSlice4Library::Slice4_GetRotation4(const FQHSlice4& InSlice)
{
	return InSlice.GetRotation4();
}

////////////////////////////////////////////////////////////////////////////////
const FMatrix& UQHSlice4Library::Slice4_GetRotation4Matrix(const FQHSlice4& InSlice)
{
	return InSlice.GetRotation4Matrix();
}

////////////////////////////////////////////////////////////////////////////////
void UQHSlice4Library::Slice4_SetRotation4(FQHSlice4& InSlice, const FEuler4D& InRotation)
{
	InSlice.SetRotation4(InRotation);
}

////////////////////////////////////////////////////////////////////////////////
FVector4 UQHSlice4Library::Slice4_Slice3ToWorld4(const FQHSlice4& InSlice4, const FVector& InSlice3Position)
{
	return InSlice4.Slice3ToWorld4(InSlice3Position);
}

////////////////////////////////////////////////////////////////////////////////
FVector UQHSlice4Library::Slice4_World4ToSlice3(const FQHSlice4& InSlice4, const FVector4& InWorld4Position)
{
	return InSlice4.World4ToSlice3(InWorld4Position);
}
