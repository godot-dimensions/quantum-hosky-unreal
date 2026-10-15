// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#include "QHVoxelWorld.h"

#include "Components/BillboardComponent.h"

AQHVoxelWorld::AQHVoxelWorld(const FObjectInitializer& Init)
	: AActor(Init)
{
	WorldRoot = CreateDefaultSubobject<UQHVoxelWorldComponent>("Root");
	WorldRoot->SetupAttachment(RootComponent);

	// The billboard appears to not be working. Perhaps it's not compatible with a 4D root. This isn't high priority to fix, but I figure I might as well leave the code here.
#if WITH_EDITOR
	auto* SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		static ConstructorHelpers::FObjectFinder<UTexture2D> SpriteFinder(TEXT("/Engine/EditorResources/S_Terrain"));
		SpriteComponent->Sprite = SpriteFinder.Object;
		SpriteComponent->SetRelativeScale3D(FVector(0.5f));
		SpriteComponent->bHiddenInGame = true;
		SpriteComponent->bIsScreenSizeScaled = true;
		SpriteComponent->SpriteInfo.Category = TEXT("Voxel World");
		SpriteComponent->SpriteInfo.DisplayName = FText::FromString("Voxel World");
		SpriteComponent->SetupAttachment(RootComponent);
		SpriteComponent->bReceivesDecals = false;
	}
#endif
}
