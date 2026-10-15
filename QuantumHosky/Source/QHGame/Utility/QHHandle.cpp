#include "QHGame/Utility/QHHandle.h"

#include "AngelscriptBinds.h"

////////////////////////////////////////////////////////////////////////////////
int32 UQHHandleLibrary::NewCounter()
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
int32 UQHHandleLibrary::NewHandle(int32& InCounter)
{
	return ++InCounter; // Pre-increment as 0 == QH_HANDLE_INVALID
}

////////////////////////////////////////////////////////////////////////////////
bool UQHHandleLibrary::IsValid(const int32 InHandle)
{
	return (InHandle > 0);
}

////////////////////////////////////////////////////////////////////////////////
AS_FORCE_LINK const FAngelscriptBinds::FBind Bind_QHHandle(FAngelscriptBinds::EOrder::Late, []
{
	FAngelscriptBinds::BindGlobalVariable("const int32 QH_HANDLE_INVALID", &QH_HANDLE_INVALID);
});
