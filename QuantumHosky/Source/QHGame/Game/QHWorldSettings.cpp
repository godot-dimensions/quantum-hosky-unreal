#include "QHWorldSettings.h"

#include "QHGame/4D/QHSlice4.h"

////////////////////////////////////////////////////////////////////////////////
AQHWorldSettings::AQHWorldSettings(const FObjectInitializer& InInit)
	: Super(InInit)
{

#if WITH_EDITOR
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &AQHWorldSettings::OnPostWorldInitialization);
#endif

}

#if WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void AQHWorldSettings::OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues InInit) const
{
	if (InWorld == GetWorld())
	{
		// Update any settings for the editor world here...
	}
}

#endif //~ WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void AQHWorldSettings::BeginDestroy()
{
	Super::BeginDestroy();

#if WITH_EDITOR
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
#endif //~ WITH_EDITOR

}

#if WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void AQHWorldSettings::PostEditChangeProperty(FPropertyChangedEvent& InEvent)
{
	Super::PostEditChangeProperty(InEvent);

	// Update any settings for the editor world based on property changes here...
}

#endif //~ WITH_EDITOR
