#include "QHScene4Component.h"

#include "Unreal4DCore/Scene/Components/Scene4DComponent.h"

////////////////////////////////////////////////////////////////////////////////
void UQHScene4Component::SetTransform4(const FTransform4D& InTransform4)
{
	if (!Transform4.IsEqualApprox(InTransform4))
	{
		const FTransform4D OldTransform4 = MoveTemp(Transform4);

		Transform4 = InTransform4;

		OnTransform4Changed.Broadcast(this, Transform4, OldTransform4);
	}
}

////////////////////////////////////////////////////////////////////////////////
void UQHScene4Component::PostLoad()
{
	Super::PostLoad();

	if (!UpgradedTransform4)
	{
		if (AActor* Owner = GetOwner())
		{
			if (UScene4DComponent* Scene4DComponent = Owner->FindComponentByClass<UScene4DComponent>())
			{
				Scene4DComponent->SetLocalTransform4D(Transform4);

				UpgradedTransform4 = true;
			}
		}
	}
}

#if WITH_EDITOR

////////////////////////////////////////////////////////////////////////////////
void UQHScene4Component::PostEditChangeProperty(FPropertyChangedEvent& InEvent)
{
	if (InEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(UQHScene4Component, Transform4))
	{
		// TODO: Send old transform4 when modifying transform4 at edit-time.
		OnTransform4Changed.Broadcast(this, Transform4, Transform4);
	}
}

#endif //~ WITH_EDITOR
