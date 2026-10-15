// Based on Voxel Plugin Free Legacy, which is copyright Voxel Plugin SAS. TODO: check with them whether we're actually allowed to produce modified versions of their code and maybe buy a license.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"
#include "Unreal4DCore/Math/IntRect4D.h"

#include "QHVoxelWorldComponent.generated.h"

class FQHVoxelData;

UCLASS()
class QHGAME_API UQHVoxelWorldComponent : public UScene4DComponent
{
	GENERATED_BODY()

	UQHVoxelWorldComponent(const FObjectInitializer& Init);
public:
	// Size of a voxel in cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (Recreate, ClampMin = 0.0001))
	float VoxelSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, AdvancedDisplay, Category = "Voxel", meta = (Recreate, ClampMin = 1, ClampMax = 26, UIMin = 1, UIMax = 26))
	int32 Render16treeDepth = 2;

	void CreateWorld();
	void DestroyWorld();
	void DestroyVoxelComponents();

	FQHVoxelData& GetData() const { return *Data; }
	FIntRect4D GetWorldBounds() const;
	FIntVector4 GetWorldOffset() const { return *WorldOffset; }

	UFUNCTION(BlueprintCallable, Category = "Voxel|General")
	inline bool IsCreated() const { return bIsCreated; }

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	//~ Begin UObject Interface
	virtual void BeginDestroy() override;
	//~ End UObject Interface

private:
	bool bIsCreated = false;
	TSharedPtr<FQHVoxelData, ESPMode::ThreadSafe> Data;

	TSharedRef<FIntVector4> WorldOffset = MakeShared<FIntVector4>(FIntVector4::ZeroValue);

	TSharedRef<FQHVoxelData> CreateData() const;
};
