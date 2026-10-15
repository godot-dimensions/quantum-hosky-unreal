#include "Unreal4DCore/Math/Transform4D.h"

#include "AngelscriptBinds.h"

AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_Transform4D(FAngelscriptBinds::EOrder::Late, []
{
	// FTransform4D
	{
		FAngelscriptBinds Binds_Transform4D = FAngelscriptBinds::ExistingClass("FTransform4D");
		// Constructor.
		Binds_Transform4D.Constructor("void f(const FMatrix& InBasis, const FVector4& InOrigin)",
		[](FTransform4D* InThis, const FMatrix& InBasis, const FVector4& InOrigin)
		{
			new (InThis) FTransform4D(InBasis, InOrigin);
		});
	}
});
