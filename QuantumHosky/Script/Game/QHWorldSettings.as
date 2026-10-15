class AQHASWorldSettings
	: AQHWorldSettings
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "QH")
	TSoftObjectPtr<AActor> DefaultSlice4;

//==============================================================================
// protected:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected UQHSlice4Component GetDefaultSlice4Component() const
	{
		UQHSlice4Component Result = nullptr;

		AActor DefaultSlice4Actor = System::LoadAsset_Blocking(DefaultSlice4);

		if (DefaultSlice4Actor != nullptr)
		{
			Result = UQHSlice4Component::Get(DefaultSlice4Actor);
		}

		if (Result == nullptr)
		{
			TArray<AQHSlice4Actor> Slice4Actors;
			GetAllActorsOfClass(Slice4Actors);

			if (!Slice4Actors.IsEmpty())
			{
				Result = Slice4Actors[0].Slice4Component;
			}
		}

		return Result;
	}
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHASWorldSettings GetWorldSettings()
	{
		return Cast<AQHASWorldSettings>(GetCurrentWorld().GetWorldSettings());
	}
}
