#include "QHInput.h"

#include "InputAction.h"
#include "InputMappingContext.h"

////////////////////////////////////////////////////////////////////////////////
void UQHInputLibrary::InputAction_Initialize(UInputAction* InInputAction, const EInputActionValueType InValueType)
{
	if (InInputAction)
	{
		InInputAction->ValueType = InValueType;
		InInputAction->Triggers.Empty();
		InInputAction->Modifiers.Empty();
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHInputLibrary::InputMappingContext_Initialize(UInputMappingContext* InInputMappingContext)
{
	if (InInputMappingContext)
	{
		InInputMappingContext->UnmapAll();
	}
}
