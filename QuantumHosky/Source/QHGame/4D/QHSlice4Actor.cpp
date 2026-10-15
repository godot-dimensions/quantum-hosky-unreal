#include "QHSlice4Actor.h"

#include "QHGame/4D/QHSlice4Component.h"
#include "QHGame/4D/QHSlice4MeshManagerComponent.h"

////////////////////////////////////////////////////////////////////////////////
AQHSlice4Actor::AQHSlice4Actor(const FObjectInitializer& Init)
	: Super(Init)
{
	Slice4Component = CreateDefaultSubobject<UQHSlice4Component>(TEXT("Slice4Component"));
	Slice4MeshManagerComponent = CreateDefaultSubobject<UQHSlice4MeshManagerComponent>(TEXT("Slice4MeshManagerComponent"));
}
