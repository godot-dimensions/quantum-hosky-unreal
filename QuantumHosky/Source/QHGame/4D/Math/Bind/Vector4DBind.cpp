#include "Unreal4DCore/Math/VectorFuncs4D.h"

#include "AngelscriptBinds.h"

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_Vector4D(FAngelscriptBinds::EOrder::Late, []
{
	// FVector4
	{
		FAngelscriptBinds Binds_Vector4 = FAngelscriptBinds::ExistingClass("FVector4");

		Binds_Vector4.Constructor("void f(const float32 InV)",
		[](FVector4* InThis, const float InV)
		{
			return new (InThis) FVector4(InV, InV, InV, InV);
		});

		Binds_Vector4.Constructor("void f(const float32 InXYZ, const float32 InW)",
		[](FVector4* InThis, const float InXYZ, const float InW)
		{
			return new (InThis) FVector4(InXYZ, InXYZ, InXYZ, InW);
		});

		Binds_Vector4.Method("FVector4 opMul(const FVector4& InOther) const", METHODPR_TRIVIAL(FVector4, FVector4, operator*, (const FVector4&) const));

		Binds_Vector4.Method("float64 Size() const", METHOD_TRIVIAL(FVector4, Size));
		Binds_Vector4.Method("float64 SizeSquared() const", METHOD_TRIVIAL(FVector4, SizeSquared));
		Binds_Vector4.Method("FVector4 GetSafeReciprocal() const", FUNC_TRIVIAL(UVectorFuncs4D::Vector4_SafeReciprocal));

		{
			FAngelscriptBinds::FNamespace Namespace_FVector4("FVector4");

			FAngelscriptBinds::BindGlobalVariable("const FVector4 Zero", &FVector4_Zero);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 One", &FVector4_One);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Forward", &FVector4_Forward);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Back", &FVector4_Back);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Left", &FVector4_Left);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Right", &FVector4_Right);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Up", &FVector4_Up);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Down", &FVector4_Down);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Ana", &FVector4_Ana);
			FAngelscriptBinds::BindGlobalVariable("const FVector4 Kata", &FVector4_Kata);
		}
	}
});
