#pragma once

class UInputAction;
class UInputMappingContext;

//------------------------------------------------------------------------------

#include "QHInput.generated.h"

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "UInputAction UInputMappingContext"))
class UQHInputLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "Initialize"))
	static void InputAction_Initialize(UInputAction* InInputAction, const EInputActionValueType InValueType);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "Initialize"))
	static void InputMappingContext_Initialize(UInputMappingContext* InInputMappingContext);
};
