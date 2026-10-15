// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS.

#include "QHVoxelHypercubeMesher.h"
#include "CoreMinimal.h"
#include "Unreal4DCore/Model/Mesh/Tetra/ArrayTetraMesh4D.h"
#include "Unreal4DCore/Model/Mesh/Tetra/BoxTetraMesh4D.h"

FORCEINLINE static void AddFace(const TSharedPtr<FQHVoxelData> Data, UArrayTetraMesh4D* Mesh, float VoxelSize, FIntVector4 Position, FIntVector4 Offset, FVector4 TangentX, FVector4 TangentY, FVector4 TangentZ) {
	FQHVoxelValue FacingValue = Data->Get(Position + Offset);
	if (FacingValue) {
		// This face faces a solid voxel, so it's not visible.
		return;
	}
	FVector4 Base = FVector4(Position + Offset.ComponentMax(FIntVector4::ZeroValue)) * VoxelSize;
	// There are optimizations that could be applied here, but hopefully with this function inlined the compiler will do them for us.
	TArray<int32> Indices = Mesh->AppendVertices({
		Base,
		Base + TangentX,
		Base + TangentY,
		Base + TangentY + TangentX,
		Base + TangentZ,
		Base + TangentZ + TangentX,
		Base + TangentZ + TangentY,
		Base + TangentZ + TangentY + TangentX
	});
	Mesh->AppendTetraCellIndices(Indices[0], Indices[3], Indices[5], Indices[6]);
	Mesh->AppendTetraCellIndices(Indices[0], Indices[3], Indices[1], Indices[5]);
	Mesh->AppendTetraCellIndices(Indices[0], Indices[3], Indices[6], Indices[2]);
	Mesh->AppendTetraCellIndices(Indices[4], Indices[0], Indices[5], Indices[6]);
	Mesh->AppendTetraCellIndices(Indices[3], Indices[7], Indices[5], Indices[6]);
}

UTetraMesh4D* FQHVoxelHypercubeMesher::GenerateMesh(FIntRect4D ChunkBounds, const TSharedPtr<FQHVoxelData> Data, float VoxelSize) {
	UArrayTetraMesh4D* Mesh = NewObject<UArrayTetraMesh4D>(); // TODO: maybe add parameters? I'm not sure how the saving system works, perhaps this should be transient or perhaps the fact that it's only referenced from a transient UObject is sufficient.
	// TODO: read the data in a more efficient way by fetching the relevant part of the data 16tree in advance.
	ChunkBounds.Iterate([&](FIntVector4 Position) {
		FQHVoxelValue Here = Data->Get(Position);
		if (!Here) {
			return;
		}
		FVector4 X = FVector4(VoxelSize, 0, 0, 0);
		FVector4 Y = FVector4(0, VoxelSize, 0, 0);
		FVector4 Z = FVector4(0, 0, VoxelSize, 0);
		FVector4 W = FVector4(0, 0, 0, VoxelSize);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 1, 0, 0, 0), Y, Z, W);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4(-1, 0, 0, 0), Y, W, Z);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0, 1, 0, 0), X, W, Z);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0,-1, 0, 0), X, Z, W);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0, 0, 1, 0), X, Y, W);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0, 0,-1, 0), X, W, Y);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0, 0, 0, 1), X, Z, Y);
		AddFace(Data, Mesh, VoxelSize, Position, FIntVector4( 0, 0, 0,-1), X, Y, Z);
	});
	Mesh->SetFlatShadingNormals(); // Some of the comments in the mesh classes imply that a mesh having no normals is valid, but that isn't actually accepted so it's necessary to generate normals explicitly. This could be made more efficient by taking advantage of the fact that everything is cubes.
	return Mesh;
}
