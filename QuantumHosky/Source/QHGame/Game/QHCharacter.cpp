#include "QHCharacter.h"

#include "QHGame/4D/QHScene3To4Component.h"
#include "QHGame/4D/QHScene4Component.h"
#include "QHGame/4D/QHSlice4MemberComponent.h"
#include "QHGame/Game/QHCharacterMovementComponent.h"

////////////////////////////////////////////////////////////////////////////////
AQHCharacter::AQHCharacter(const FObjectInitializer& Init)
	: Super(Init.SetDefaultSubobjectClass<UQHCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	Scene4Component = CreateDefaultSubobject<UQHScene4Component>("Scene4Component");
	Slice4MemberComponent = CreateDefaultSubobject<UQHSlice4MemberComponent>("Slice4MemberComponent");
	Scene3To4Component = CreateDefaultSubobject<UQHScene3To4Component>("Scene3To4Component");
}
