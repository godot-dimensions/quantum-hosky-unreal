#include "Math/Matrix.h"

#include "AngelscriptBinds.h"

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_Matrix4D(FAngelscriptBinds::EOrder::Late, []
{
	FAngelscriptBinds Binds_Matrix = FAngelscriptBinds::ExistingClass("FMatrix");
	Binds_Matrix.Method("FVector4 TransformVector4(const FVector4& InVector) const", METHODPR_TRIVIAL(FVector4, FMatrix, TransformFVector4, (const FVector4&) const));
});
