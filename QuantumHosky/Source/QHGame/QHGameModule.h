#pragma once

#include "QHDefines.h"

class AActor;
class UActorComponent;

//------------------------------------------------------------------------------

#include "Unreal4DCore/Math/Euler4D.h"
#include "Unreal4DCore/Math/Transform4D.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

#include "QHGameModule.generated.h"

//______________________________________________________________________________
UCLASS()
class QHGAME_API UQHGameLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

//==============================================================================
public:

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static UScene4DComponent* GetActor4DComponent(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FTransform4D GetActorLocalTransform4D(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FVector4 GetActorLocalPosition4D(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FEuler4D GetActorLocalRotationDegrees4D(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FTransform4D GetActorTransform4(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FVector4 GetActorLocation4(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true)
	static FEuler4D GetActorRotation4(const AActor* InActor);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static FVector4 Vector3DTo4D(const UObject* InWorldContext, const FVector& InVector3D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static FVector Vector4DTo3D(const UObject* InWorldContext, const FVector4& InVector4D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static FVector4 Position3DTo4D(const UObject* InWorldContext, const FVector& InPosition3D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static FVector Position4DTo3D(const UObject* InWorldContext, const FVector4& InPosition4D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static FVector4 Position4DToSlice4D(const UObject* InWorldContext, const FVector4& InPosition4D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static float SliceDistance4D(const UObject* InWorldContext, const FVector4& InPosition4D);

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (DeterminesOutputType = "OutComponentType"))
	static UActorComponent* FindComponent(const AActor* InActor, const TSubclassOf<UActorComponent>& OutComponentType);

	template <typename T>
	static T* FindComponent(const AActor* InActor)
	{
		return Cast<T>(UQHGameLibrary::FindComponent(InActor, T::StaticClass()));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure = true, meta = (WorldContext = "InWorldContext"))
	static void DrawLine(const UObject* InWorldContext, const FVector& InStart, const FVector& InEnd, const FColor& InColor, const float InDuration = 0.0f, const float InThickness = 0.0f);
};
