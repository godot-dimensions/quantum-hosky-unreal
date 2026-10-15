// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.
#include "QHVoxelWorldComponent.h"

#include "QHVoxelData.h"
#include "QHVoxelUtilities.h"
#include "QHVoxelRenderer.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"

UQHVoxelWorldComponent::UQHVoxelWorldComponent(const FObjectInitializer& Init)
	: UScene4DComponent(Init)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bHighPriority = true;
	bTickInEditor = true;
	bWantsInitializeComponent = true;
}

void UQHVoxelWorldComponent::CreateWorld()
{
	if (IsCreated())
	{
		return;
	}

	bIsCreated = true;

	*WorldOffset = FIntVector4::ZeroValue;

	if (!Data)
	{
		Data = CreateData();
	}
	FQHVoxelRenderer::CreateMeshes(this, Data);
}

void UQHVoxelWorldComponent::DestroyWorld()
{
	if (!IsCreated())
	{
		return;
	}

	bIsCreated = false;

	Data.Reset();

	WorldOffset = MakeShared<FIntVector4>(FIntVector4::ZeroValue);

	DestroyVoxelComponents();
}

void UQHVoxelWorldComponent::DestroyVoxelComponents()
{
	for (USceneComponent* Component : GetAttachChildren())
	{
		if (IsValid(Component) && Component->HasAnyFlags(RF_Transient) && Component->IsA<UMesh4DComponent>())
		{
			Component->DestroyComponent();
		}
	}
}

FIntRect4D UQHVoxelWorldComponent::GetWorldBounds() const
{
	return FQHVoxelUtilities::GetBoundsFromDepth<QH_RENDER_CHUNK_SIZE>(Render16treeDepth);
}

void UQHVoxelWorldComponent::OnRegister()
{
	Super::OnRegister();

	if (!IsCreated() && !HasAnyFlags(RF_Transient)) // If it's transient, that indicates it's in a preview actor.
	{
		CreateWorld();
	}
}

void UQHVoxelWorldComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (GetWorld()->WorldType != EWorldType::Editor && GetWorld()->WorldType != EWorldType::EditorPreview) // We don't want to tick the BP in preview
	{
		Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	}
}

void UQHVoxelWorldComponent::BeginDestroy()
{
	if (IsCreated())
	{
		DestroyWorld();
	}

	Super::BeginDestroy();
}

TSharedRef<FQHVoxelData> UQHVoxelWorldComponent::CreateData() const
{
	return FQHVoxelData::Create(FQHVoxelUtilities::Get16treeDepthContainingBounds<QH_DATA_CHUNK_SIZE>(GetWorldBounds()), GetWorldBounds());
}
