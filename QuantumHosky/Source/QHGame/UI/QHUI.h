#pragma once

#include "Components/Widget.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "QHUI.generated.h"

//______________________________________________________________________________
DECLARE_DYNAMIC_DELEGATE_RetVal(UWidget*, FQHGetWidget);

//______________________________________________________________________________
UCLASS(meta = (ScriptMixin = "UWidget"))
class QHGAME_API UQHUILibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = false, meta = (ScriptName = "SetTooltipWidgetDelegate"))
	static void Widget_SetTooltipWidgetDelegate(UWidget* InWidget, const FQHGetWidget& InDelegate);
};
