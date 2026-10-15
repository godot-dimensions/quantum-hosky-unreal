//______________________________________________________________________________
UCLASS(Abstract)
class UQHTooltipWidget
	: UUserWidget
{

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UTextBlock Text_Title = nullptr;

	UPROPERTY(BindWidget)
	private UTextBlock Text_Description = nullptr;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	void SetTitle(const FText& InTitle)
	{
		Text_Title.SetText(InTitle);
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetDescription(const FText& InDescription)
	{
		Text_Description.SetText(InDescription);
	}
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	UQHTooltipWidget GetTooltipWidget(const FText& InTitle, const FText& InDescription)
	{
		UQHTooltipWidget Result = nullptr;

		AQHASHUD HUD = QH::GetHUD();

		if (HUD != nullptr)
		{
			Result = HUD.GetTooltipWidget();

			if (Result != nullptr)
			{
				Result.SetTitle(InTitle);
				Result.SetDescription(InDescription);
			}
		}

		return Result;
	}
}
