#include "Unreal4DCore/Math/Euler4D.h"

#include "AngelscriptBinds.h"

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_Euler4D(FAngelscriptBinds::EOrder::Late, []
{
	// FEuler4D
	{
		FAngelscriptBinds Binds_Euler4D = FAngelscriptBinds::ExistingClass("FEuler4D");
		// Constructor.
		Binds_Euler4D.Constructor("void f(const double InYZ, const double InZX, const double InXY, const double InXW, const double InWY, const double InZW)",
		[](FEuler4D* InThis, const double InYZ, const double InZX, const double InXY, const double InXW, const double InWY, const double InZW)
		{
			new (InThis) FEuler4D(InYZ, InZX, InXY, InXW, InWY, InZW);
		});

		// Operators.
		Binds_Euler4D.Method("bool opEquals(const FEuler4D& InOther) const", METHODPR_TRIVIAL(bool, FEuler4D, operator==, (const FEuler4D&) const));
		Binds_Euler4D.Method("FEuler4D opAdd(const FEuler4D& InOther) const", METHODPR_TRIVIAL(FEuler4D, FEuler4D, operator+, (const FEuler4D&) const));
		Binds_Euler4D.Method("void opAddAssign(const FEuler4D& InOther)",
			+[](FEuler4D* InThis, const FEuler4D& InOther) { InThis->FEuler4D::operator+=(InOther); });
		Binds_Euler4D.Method("FEuler4D opSub(const FEuler4D& InOther) const", METHODPR_TRIVIAL(FEuler4D, FEuler4D, operator-, (const FEuler4D&) const));
		Binds_Euler4D.Method("void opSubAssign(const FEuler4D& InOther)",
			+[](FEuler4D* InThis, const FEuler4D& InOther) { InThis->FEuler4D::operator-=(InOther); });
		Binds_Euler4D.Method("FEuler4D opMul(const FEuler4D& InChild) const", METHODPR_TRIVIAL(FEuler4D, FEuler4D, operator*, (const FEuler4D&) const));
		Binds_Euler4D.Method("FEuler4D opMul(double InScalar) const", METHODPR_TRIVIAL(FEuler4D, FEuler4D, operator*, (double) const));
		Binds_Euler4D.Method("void opMulAssign(const FEuler4D& InChild)",
			+[](FEuler4D* InThis, const FEuler4D& InChild) { InThis->FEuler4D::operator*=(InChild); });
	}
});
