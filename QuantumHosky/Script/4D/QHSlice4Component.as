//______________________________________________________________________________
struct FQHSlice4State
{
	FVector4 Origin4;
	FVector Origin3;
	FEuler4D Rotation4;
	EQHSlice4RotationAxis RotationAxis;

	//------------------------------------------------------------------------------

	FQHSlice4State Lerp(const FQHSlice4State& InTo, const float InT) const
	{
		FQHSlice4State Result;
		Result.Origin4 = Origin4.Lerp(InTo.Origin4, float32(InT));
		Result.Origin3 = Math::Lerp(Origin3, InTo.Origin3, InT);
		Result.Rotation4 = Rotation4 + (InTo.Rotation4 - Rotation4).Wrapped() * InT;
		Result.RotationAxis = ((InT < 0.5f) ? RotationAxis : InTo.RotationAxis);

		return Result;
	}
}

//______________________________________________________________________________
enum EQHSlice4RotationAxis
{
	XY,
	XZ,
	YZ,
	XW,
	YW,
	ZW,

	MAX
}

//______________________________________________________________________________
event void FQHOnSlice4ActiveChanged(UQHASSlice4Component InSlice4Component, const bool InActive);

//______________________________________________________________________________
class UQHASSlice4Component
	: UQHSlice4Component
{
	default PrimaryComponentTick.bStartWithTickEnabled = false;

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool Slice4OriginStatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	EQHSlice4RotationAxis Slice4RotationAxis = QH::DefaultSlice4RotationAxis;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	float Slice4RotationSpeed = QH::DefaultSlice4RotationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool ResetSlice4OnActivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QH")
	bool ResetSlice4OnDeactivate = false;

	UPROPERTY()
	FQHOnSlice4ActiveChanged OnActiveChanged;

//==============================================================================
// private:

	private bool Slice4Active = false;

	private FQHSlice4State DefaultSlice4State;

	private FQHSlice4State Slice4StateBlendTo;
	private float Slice4StateBlendDuration = 0.0f;
	private float Slice4StateBlendTimer = 0.0f;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	bool IsSlice4Active() const
	{
		return Slice4Active;
	}

	////////////////////////////////////////////////////////////////////////////////
	FQHSlice4State GetSlice4State() const
	{
		FQHSlice4State Result;
		Result.Origin4 = Slice4.GetOrigin4();
		Result.Origin3 = Slice4.GetOrigin3();
		Result.Rotation4 = Slice4.GetRotation4();
		Result.RotationAxis = Slice4RotationAxis;

		return Result;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetSlice4State(const FQHSlice4State& InState, const float InBlendDuration = QH::DefaultSlice4BlendDuration)
	{
		if (InBlendDuration <= 0.0f)
		{
			SetSlice4State_Implementation(InState);
		}
		else
		{
			Slice4StateBlendTo = InState;
			Slice4StateBlendDuration = InBlendDuration;
			Slice4StateBlendTimer = 0.0f;

			SetComponentTickEnabled(true);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void ResetSlice4(const float InBlendDuration = QH::DefaultSlice4BlendDuration)
	{
		SetSlice4State(DefaultSlice4State, InBlendDuration);
	}

	////////////////////////////////////////////////////////////////////////////////
	const FQHSlice4State& GetDefaultSlice4State() const
	{
		return DefaultSlice4State;
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	void ActivateSlice4_Private()
	{
		if (!Slice4Active)
		{
			Slice4Active = true;

			if (ResetSlice4OnActivate)
			{
				ResetSlice4();
			}

			OnActiveChanged.Broadcast(this, true);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	void DeactivateSlice4_Private()
	{
		if (Slice4Active)
		{
			Slice4Active = false;

			if (ResetSlice4OnDeactivate)
			{
				ResetSlice4();
			}

			OnActiveChanged.Broadcast(this, false);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	private void SetSlice4State_Implementation(const FQHSlice4State& InState)
	{
		FQHSlice4 NewSlice4 = GetSlice4();

		NewSlice4.SetOrigin(InState.Origin4, InState.Origin3);
		NewSlice4.SetRotation4(InState.Rotation4);

		SetSlice4(NewSlice4);

		Slice4RotationAxis = InState.RotationAxis;
	}

//==============================================================================
// protected:

	//~ UActorComponent BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void BeginPlay()
	{
		DefaultSlice4State = GetSlice4State();
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Tick(const float InDeltaTime)
	{
		// Blend slice state over time
		{
			Slice4StateBlendTimer += InDeltaTime;

			const float BlendT = Math::Clamp(Math4D::SafeDivide(Slice4StateBlendTimer, Slice4StateBlendDuration, 1.0f), 0.0f, 1.0f);

			FQHSlice4State Slice4State = GetSlice4State();

			Slice4State = Slice4State.Lerp(Slice4StateBlendTo, BlendT);

			SetSlice4State_Implementation(Slice4State);

			if (Slice4StateBlendTimer >= Slice4StateBlendDuration)
			{
				SetComponentTickEnabled(false);
			}
		}
	}

	//~ UActorComponent END
}

//______________________________________________________________________________
namespace QH
{
	const EQHSlice4RotationAxis DefaultSlice4RotationAxis = EQHSlice4RotationAxis::XZ;
	const float DefaultSlice4RotationSpeed = 60.0f;
	const float DefaultSlice4BlendDuration = 0.7f;
}
