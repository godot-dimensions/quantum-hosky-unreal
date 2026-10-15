#include "QHVoxelRenderer.h"
#include "QHVoxelHypercubeMesher.h"
#include "CoreMinimal.h"
#include "CoreUObject.h"
#include "QHVoxelDefinitions.h"
#include "Unreal4DCore/Model/Mesh/Base/Mesh4D.h"
#include "Unreal4DCore/Scene/Components/Mesh4DComponent.h"

void FQHVoxelRenderer::CreateMeshes(UQHVoxelWorldComponent* World, const TSharedPtr<FQHVoxelData> Data) {
	TSubclassOf<UActorComponent> MeshComponentClass = UMesh4DComponent::StaticClass();
	FIntRect4D WorldBounds = World->GetWorldBounds();
	WorldBounds.Iterate(QH_RENDER_CHUNK_SIZE, [&](FIntVector4 Position) {
		FIntRect4D ChunkBounds = FIntRect4D(Position, Position + FIntVector4(QH_RENDER_CHUNK_SIZE));
		UTetraMesh4D* Mesh = FQHVoxelHypercubeMesher::GenerateMesh(ChunkBounds, Data, World->VoxelSize);
		if (Mesh->GetSimplexCellVertexIndices().Num() != 0) { // Empty meshes are sort of considered invalid. The rendering system disallows them.
			UMesh4DComponent* Component = NewObject<UMesh4DComponent>(World->GetOwner(), NAME_None, RF_Transient);
			Component->OnComponentCreated();
			World->GetOwner()->AddInstanceComponent(Component);
			Component->Mesh4D = Mesh;
			Component->SetupAttachment(World, NAME_None);
			Component->RegisterComponent();
		}
	});
}
