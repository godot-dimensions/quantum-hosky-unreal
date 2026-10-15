#include "QHScene3To4Component.h"

#include "QHGame/4D/QHScene4Component.h"
#include "QHGame/4D/QHSlice4Component.h"
#include "QHGame/4D/QHSlice4MemberComponent.h"
#include "Unreal4DCore/Math/Transform4D.h"
#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

void UQHScene3To4Component::SetUpdateTransformEnabled(const bool InEnabled)
{
	TransformUpdateLock += (InEnabled ? -1 : 1);
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::UpdateTransform4From3(const bool InAbsolute /*= true*/)
{
	// TODO: Update entire 4D transform from 3D transform.

	const FTransform Transform3 = Scene3Component
								? Scene3Component->GetComponentTransform()
								: FTransform();

	if ((TransformUpdateLock == 0) && (Scene4Component || Scene4DComponent))
	{
		FVector4 Location4 = FVector4::Zero();

		if (InAbsolute)
		{
			// NOTE: If the 4D transform is not 'inside' the slice (even if a part of the object is intersecting the slice),
			// this will `snap` the 4D transform into the slice and will create 4D motion discontinuities.
			// In this case, apply the 3D -> 4D transform updates with InAbsolute = false to translate the object in 4D by its corresponding 3D translation, maintaining smooth 4D motion.
			Location4 = OwnerSlice4
					  ? OwnerSlice4->GetSlice4().Slice3ToWorld4(Transform3.GetLocation())
					  : FVector4(Transform3.GetLocation(), 0.0f);
		}
		else
		{
			// Calculate 3D translation
			const FVector Translation3 = (Transform3.GetLocation() - PreviousTransform3.GetLocation());

			FVector4 Translation4 = FVector4(Translation3, 0.0f);

			if (OwnerSlice4)
			{
				// Calculate 4D translation
				Translation4 = OwnerSlice4->GetSlice4().GetRotation4Matrix().TransformFVector4(Translation4);
			}

			Location4 = Scene4Component
					  ? Scene4Component->GetTransform4().Origin
					  : Scene4DComponent->GetGlobalPosition4D();

			// Translate object in 4D by its corresponding 3D translation
			Location4 += Translation4;
		}

		// Apply updated 4D transform
		++TransformUpdateLock;
		{
			if (Scene4Component)
			{
				FTransform4D Transform4 = Scene4Component->GetTransform4();
				Transform4.Origin = Location4;

				Scene4Component->SetTransform4(Transform4);
			}
			else
			{
				check(Scene4DComponent);

				Scene4DComponent->SetGlobalPosition4D(Location4);
			}
		}
		--TransformUpdateLock;
	}

	// We always update the previous transform even if transform updating is locked so that subsequent delta operations are accurate
	PreviousTransform3 = Transform3;
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::UpdateTransform3From4()
{
	if ((TransformUpdateLock == 0) && Scene3Component && (Scene4Component || Scene4DComponent))
	{
		const FVector4 Location4 = Scene4Component
								 ? Scene4Component->GetTransform4().Origin
								 : Scene4DComponent->GetGlobalPosition4D();

		// NOTE: If the 4D location is 'outside' the slice (even if a part of the object is intersecting the slice), this will snap the location to be 'inside' the slice.
		const FVector Location3 = OwnerSlice4
								? OwnerSlice4->GetSlice4().World4ToSlice3(Location4)
								: FVector(Location4);

		// Apply updated 3D transform
		++TransformUpdateLock;
		{
			Scene3Component->SetWorldLocation(Location3, false, nullptr, ETeleportType::TeleportPhysics);
		}
		--TransformUpdateLock;
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::SetOwnerSlice4(UQHSlice4Component* InSlice4Component)
{
	if (OwnerSlice4)
	{
		OwnerSlice4->OnSlice4Changed.RemoveDynamic(this, &UQHScene3To4Component::OnSlice4Changed);
	}

	OwnerSlice4 = InSlice4Component;

	if (OwnerSlice4)
	{
		OwnerSlice4->OnSlice4Changed.AddDynamic(this, &UQHScene3To4Component::OnSlice4Changed);
	}

	UpdateTransform4From3();
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::OnTransform3Changed(USceneComponent* InComponent, const EUpdateTransformFlags InFlags, const ETeleportType InTeleport)
{
	// NOTE: We apply 3D -> 4D transform changes as deltas as the 4D transform may not be 'inside' the slice (even if a part of the object is intersecting the slice);
	// absolute 3D -> 4D transform applications 'snap' the object into the slice, creating motion discontinuties for 4D transforms not 'inside' the slice
	UpdateTransform4From3(false);
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::OnOwnerSlice4Changed(UQHSlice4MemberComponent* InSlice4MemberComponent, UQHSlice4Component* InNewOwner, UQHSlice4Component* InOldOwner)
{
	SetOwnerSlice4(InNewOwner);
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::OnSlice4Changed(UQHSlice4Component* InSlice4Component, const FQHSlice4& InNewSlice, const FQHSlice4& InOldSlice)
{
	if (UpdateTransform3OnSlice4Changed)
	{
		UpdateTransform3From4();
	}
	else
	{
		// If specified, keep the object in-place in 3D even if the 4D slice moves about it (e.g. the player character);
		// NOTE: This causes the object to move in 4D even if its 3D location remains fixed (unless at the slice origin)
		UpdateTransform4From3();
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	Scene3Component = Owner->GetRootComponent();

	if (Scene3Component)
	{
		Scene3Component->TransformUpdated.AddUObject(this, &UQHScene3To4Component::OnTransform3Changed);
	}

	Scene4Component = Owner->FindComponentByClass<UQHScene4Component>();
	Scene4DComponent = Owner->FindComponentByClass<UScene4DComponent>();

	Slice4MemberComponent = Owner->FindComponentByClass<UQHSlice4MemberComponent>();

	UQHSlice4Component* Slice4 = nullptr;

	if (Slice4MemberComponent)
	{
		Slice4 = Slice4MemberComponent->GetOwnerSlice4();

		Slice4MemberComponent->OnOwnerSlice4Changed.AddDynamic(this, &UQHScene3To4Component::OnOwnerSlice4Changed);
	}

	SetOwnerSlice4(Slice4);
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene3To4Component::EndPlay(const EEndPlayReason::Type InReason)
{
	Super::EndPlay(InReason);

	if (Scene3Component)
	{
		Scene3Component->TransformUpdated.RemoveAll(this);
	}

	if (Slice4MemberComponent)
	{
		Slice4MemberComponent->OnOwnerSlice4Changed.RemoveDynamic(this, &UQHScene3To4Component::OnOwnerSlice4Changed);
	}

	SetOwnerSlice4(nullptr);
}
