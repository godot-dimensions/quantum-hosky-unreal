#include "QHGame/UI/QHUI.h"

////////////////////////////////////////////////////////////////////////////////
void UQHUILibrary::Widget_SetTooltipWidgetDelegate(UWidget* InWidget, const FQHGetWidget& InDelegate)
{
	if (InWidget)
	{
		InWidget->ToolTipWidgetDelegate.BindUFunction(const_cast<UObject*>(InDelegate.GetUObject()), InDelegate.GetFunctionName());
	}
}
