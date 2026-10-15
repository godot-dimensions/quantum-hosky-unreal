//______________________________________________________________________________
class AQHASHUD
	: AHUD
{

//==============================================================================
// public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UQHHUDWidget> HUDWidgetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UQHTooltipWidget> TooltipWidgetType;

//==============================================================================
// private:

	private UQHHUDWidget HUDWidget = nullptr;
	private UQHTooltipWidget TooltipWidget = nullptr;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	UQHHUDWidget GetHUDWidget() const
	{
		return HUDWidget;
	}

	////////////////////////////////////////////////////////////////////////////////
	UQHTooltipWidget GetTooltipWidget() const
	{
		return TooltipWidget;
	}

	////////////////////////////////////////////////////////////////////////////////
	bool GetInventoryOpen() const
	{
		return HUDWidget.GetInventoryOpen();
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetInventoryOpen(const bool InOpen)
	{
		HUDWidget.SetInventoryOpen(InOpen);
	}

	void SetInteractable(UQHInteractableComponent InInteractable)
	{
		HUDWidget.SetInteractable(InInteractable);
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnInventoryOpened(UQHHUDWidget InHUDWidget, const bool InOpen)
	{
		// TODO: Push/pop HUD input context to control player available interactions.

		if (InOpen)
		{
			// Allow user interaction with the HUD
			Widget::SetInputMode_GameAndUIEx(PlayerOwner);

			// Prevent the mouse from being captured when clicking on non-UI parts of the viewport
			Gameplay::SetViewportMouseCaptureMode(EMouseCaptureMode::NoCapture);

			PlayerOwner.bShowMouseCursor = true;

			// Position the mouse at the center of the viewport so that the user has a consistent starting point when opening the HUD
			{
				const FVector2D ViewportSize = WidgetLayout::GetViewportSize();

				PlayerOwner.SetMouseLocation(int32(ViewportSize.X * 0.5f), int32(ViewportSize.Y * 0.5f));
			}
		}
		else
		{
			// Restore regular user interaction with the viewport

			Widget::SetInputMode_GameOnly(PlayerOwner);

			Gameplay::SetViewportMouseCaptureMode(EMouseCaptureMode::CapturePermanently);

			PlayerOwner.bShowMouseCursor = false;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	private void DrawArrow(const float InStartX, const float InStartY, const float InEndX, const float InEndY, const FLinearColor& InColor, const float InArrowSize = 10.0f, const float InThickness = 0.0f)
	{
		const FVector2D Start(InStartX, InStartY);
		const FVector2D End(InEndX, InEndY);
		const FVector2D Dir = (End - Start).GetSafeNormal();
		const float Angle = Math::Atan2(Dir.Y, Dir.X);
		const float ArrowAngle0 = (Angle + (PI / 4.0f)); // +45 degrees
		const float ArrowAngle1 = (Angle - (PI / 4.0f)); // -45 degrees
		const FVector2D ArrowDir0(Math::Cos(ArrowAngle0), Math::Sin(ArrowAngle0));
		const FVector2D ArrowDir1(Math::Cos(ArrowAngle1), Math::Sin(ArrowAngle1));

		DrawLine(InStartX, InStartY, InEndX, InEndY, InColor, InThickness); // line
		DrawLine(InEndX, InEndY, (InEndX - (ArrowDir0.X * InArrowSize)), (InEndY - (ArrowDir0.Y * InArrowSize)), InColor, InThickness); // arrow top
		DrawLine(InEndX, InEndY, (InEndX - (ArrowDir1.X * InArrowSize)), (InEndY - (ArrowDir1.Y * InArrowSize)), InColor, InThickness); // arrow bottom
	}

	////////////////////////////////////////////////////////////////////////////////
	private void DrawCompass4(const int32 InWidth, const int32 InHeight)
	{
		// TODO: Render the compass as a widget alongside other HUD elements.
		// TODO: Render player forward vector inline with the compass for clarity as to how their view aligns with the compass view.
		// TODO: Render rotation axis orientation inline with the compass.
		// TODO: Render player 4D coordinates inline with the compass.

		UQHASSlice4Component ActiveSlice4Component = QH::GetActiveSlice4Component();
		const FQHSlice4 ActiveSlice4 = (ActiveSlice4Component != nullptr)
									 ? ActiveSlice4Component.GetSlice4()
									 : FQHSlice4();

		const float CompassX = 50.0f;
		const float CompassY = 50.0f;
		const float CompassWidth = 300.0f;
		const float CompassHeight = 300.0f;

		// Draw compass background
		DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.2f), CompassX, CompassY, CompassWidth, CompassHeight);

		// Draw player coordinates
		{
			const FVector4 PlayerLocation4 = QHGame::GetActorLocation4(QH::GetPlayerCharacter());

			const FString OriginText = f"({int32(PlayerLocation4.X):>5}, {int32(PlayerLocation4.Y):>5}, {int32(PlayerLocation4.Z):>5}, {int32(PlayerLocation4.W):>5})";
			const float TextStartX = (CompassX + 5.0f);
			const float TextStartY = (CompassY + 260.0f);
			const float TextScale = 2.0f;

			DrawText(OriginText, FLinearColor::Black, (TextStartX + TextScale), (TextStartY + TextScale), Scale = TextScale); // shadow
			DrawText(OriginText, FLinearColor::White, TextStartX, TextStartY, Scale = TextScale); // coordinates

			AQHASPlayerCharacter PlayerCharacter = QH::GetPlayerCharacter();
			const FVector PlayerLocation3 = PlayerCharacter.GetActorLocation();

			const FString OriginText3 = f"({int32(PlayerLocation3.X):>5}, {int32(PlayerLocation3.Y):>5}, {int32(PlayerLocation3.Z):>5})";
			const float TextStartX3 = (CompassX + 5.0f);
			const float TextStartY3 = (CompassY + 210.0f);
			const float TextScale3 = 2.0f;

			DrawText(OriginText3, FLinearColor::Black, (TextStartX3 + TextScale3), (TextStartY3 + TextScale3), Scale = TextScale3); // shadow
			DrawText(OriginText3, FLinearColor::White, TextStartX3, TextStartY3, Scale = TextScale3); // coordinates
		}

		// Draw active slice
		/*
		{
			const FString SliceNameText = (ActiveSlice4Component != nullptr)
										? f"{ActiveSlice4Component.Owner.GetActorNameOrLabel()}"
										: "No active slice";

			const float TextStartX = (CompassX + 5.0f);
			const float TextStartY = (CompassY + 220.0f);
			const float TextScale = 2.0f;

			DrawText(SliceNameText, FLinearColor::Black, (TextStartX + TextScale), (TextStartY + TextScale), Scale = TextScale); // shadow
			DrawText(SliceNameText, FLinearColor::Yellow, TextStartX, TextStartY, Scale = TextScale); // coordinates
		}
		*/

		// Draw slicing axes
		{
			const FMatrix Slice4RotationMatrix = ActiveSlice4.GetRotation4Matrix();

			// 4D slicing axes
			const FVector4 Slice4X = Slice4RotationMatrix.TransformVector4(FVector4(1.0f, 0.0f, 0.0f, 0.0f));
			const FVector4 Slice4Y = Slice4RotationMatrix.TransformVector4(FVector4(0.0f, 1.0f, 0.0f, 0.0f));
			const FVector4 Slice4Z = Slice4RotationMatrix.TransformVector4(FVector4(0.0f, 0.0f, 1.0f, 0.0f));
			const FVector4 Slice4W = Slice4RotationMatrix.TransformVector4(FVector4(0.0f, 0.0f, 0.0f, 1.0f));

			// Project 4D slicing axes to 3D
			const FVector Slice3X = Slice4X.Project4To3();
			const FVector Slice3Y = Slice4Y.Project4To3();
			const FVector Slice3Z = Slice4Z.Project4To3();
			const FVector Slice3W = Slice4W.Project4To3();

			// Stage the slicing axes in front of the camera and then project them to the screen

			const FTransform CameraTransform = QH::GetCameraTransform();
			const FVector AxisOrigin = FVector(100.0f, 0.0f, 0.0f);
			const FRotator AxisRotation(-25.0f, 105.0f, 25.0f);
			const float AxisLength = 8.0f;

			const FVector AxisStart = Project(CameraTransform.TransformPosition(AxisOrigin));
			const FVector AxisXEnd 	= Project(CameraTransform.TransformPosition(AxisOrigin + (AxisRotation.RotateVector(Slice3X) * AxisLength))); // 3D slicing x-axis
			const FVector AxisYEnd 	= Project(CameraTransform.TransformPosition(AxisOrigin + (AxisRotation.RotateVector(Slice3Y) * AxisLength))); // 3D slicing y-axis
			const FVector AxisZEnd 	= Project(CameraTransform.TransformPosition(AxisOrigin + (AxisRotation.RotateVector(Slice3Z) * AxisLength))); // 3D slicing z-axis
			const FVector AxisWEnd 	= Project(CameraTransform.TransformPosition(AxisOrigin + (AxisRotation.RotateVector(Slice3W) * AxisLength))); // 3D slicing w-axis

			const float AxisOffsetX = (InWidth * -0.4625f);
			const float AxisOffsetY = (InHeight * -0.4325f);
			const float AxisStartX = (CompassX + AxisStart.X + AxisOffsetX);
			const float AxisStartY = (CompassY + AxisStart.Y + AxisOffsetY);
			const float AxisArrowSize = 10.0f;
			const float AxisThickness = 2.0f;

			DrawArrow(AxisStartX, AxisStartY, (CompassX + AxisXEnd.X + AxisOffsetX), (CompassY + AxisXEnd.Y + AxisOffsetY), FLinearColor::Red, 		AxisArrowSize, AxisThickness); // x-axis
			DrawArrow(AxisStartX, AxisStartY, (CompassX + AxisYEnd.X + AxisOffsetX), (CompassY + AxisYEnd.Y + AxisOffsetY), FLinearColor::Green, 	AxisArrowSize, AxisThickness); // y-axis
			DrawArrow(AxisStartX, AxisStartY, (CompassX + AxisZEnd.X + AxisOffsetX), (CompassY + AxisZEnd.Y + AxisOffsetY), FLinearColor::Blue, 	AxisArrowSize, AxisThickness); // z-axis
			DrawArrow(AxisStartX, AxisStartY, (CompassX + AxisWEnd.X + AxisOffsetX), (CompassY + AxisWEnd.Y + AxisOffsetY), FLinearColor::Yellow, 	AxisArrowSize, AxisThickness); // w-axis
		}

		// Draw slicing rotation axis
		if (QH::IsSliceDigitalRotationEnabled())
		{
			const EQHSlice4RotationAxis RotationAxis = (ActiveSlice4Component != nullptr)
													 ? ActiveSlice4Component.Slice4RotationAxis
													 : QH::DefaultSlice4RotationAxis;

			const FString RotationAxisText = f"{RotationAxis:n}";
			const float TextStartX = (CompassX + 10.0f);
			const float TextStartY = (CompassY + 5.0f);
			const float TextScale = 3.0f;

			DrawText(RotationAxisText, FLinearColor::Black, (TextStartX + TextScale), (TextStartY + TextScale), Scale = TextScale); // shadow
			DrawText(RotationAxisText, FLinearColor::White, TextStartX, TextStartY, Scale = TextScale); // axis
		}
	}

//==============================================================================
// protected:

	//~ AActor BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
		// Create the HUD
		HUDWidget = Cast<UQHHUDWidget>(WidgetBlueprint::CreateWidget(HUDWidgetType, QH::GetPlayerController()));
		HUDWidget.OnInventoryOpened.AddUFunction(this, n"OnInventoryOpened");
		HUDWidget.AddToPlayerScreen();

		// Create the default tooltip widget
		TooltipWidget = Cast<UQHTooltipWidget>(WidgetBlueprint::CreateWidget(TooltipWidgetType, QH::GetPlayerController()));
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void EndPlay(EEndPlayReason EndPlayReason)
	{
		HUDWidget.OnInventoryOpened.UnbindObject(this);
	}

	//~ AActor END

	//~ AHUD BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void DrawHUD(int32 SizeX, int32 SizeY)
	{
		DrawCompass4(SizeX, SizeY);
	}

	//~ AHUD END
}

//______________________________________________________________________________
event void FQHOnInventoryOpened(UQHHUDWidget InHUDWidget, const bool InOpen);

//______________________________________________________________________________
class UQHHUDWidget
	: UUserWidget
{

//==============================================================================
// public:

	UPROPERTY()
	FQHOnInventoryOpened OnInventoryOpened;

//==============================================================================
// private:

	UPROPERTY(BindWidget)
	private UPanelWidget Panel_Inventory = nullptr;

	UPROPERTY(BindWidget)
	private UQHInventoryStorageWidget Widget_InventoryStorage = nullptr;

	UPROPERTY(BindWidget)
	private UQHInventoryQuickBarWidget Widget_InventoryQuickBar = nullptr;

	UPROPERTY(BindWidget)
	private UQHInteractWidget Widget_Interact = nullptr;

	private bool InventoryOpen = false;

//==============================================================================
// public:

	////////////////////////////////////////////////////////////////////////////////
	bool GetInventoryOpen() const
	{
		return InventoryOpen;
	}

	////////////////////////////////////////////////////////////////////////////////
	void SetInventoryOpen(const bool InOpen)
	{
		if (InventoryOpen != InOpen)
		{
			InventoryOpen = InOpen;

			UpdateInventoryOpen();

			OnInventoryOpened.Broadcast(this, InventoryOpen);
		}
	}

	void SetInteractable(UQHInteractableComponent InInteractable)
	{
		if (InInteractable != nullptr)
		{
			Widget_Interact.SetVisibility(ESlateVisibility::Visible);
			Widget_Interact.SetInteractable(InInteractable);
		}
		else
		{
			Widget_Interact.SetVisibility(ESlateVisibility::Hidden);
		}
	}

//==============================================================================
// private:

	////////////////////////////////////////////////////////////////////////////////
	private void UpdateInventoryOpen()
	{
		Panel_Inventory.SetVisibility(InventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION()
	private void OnGameReady()
	{
		UQHASInventoryComponent Inventory = QH::GetPlayerInventory();

		if (Inventory != nullptr)
		{
			Widget_InventoryStorage.InitializeInventory(Inventory);
			Widget_InventoryQuickBar.InitializeInventory(Inventory);
		}

		UpdateInventoryOpen();

		SetInteractable(nullptr);
	}

//==============================================================================
// protected:

	//~ UUserWidget BEGIN

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void OnInitialized()
	{
		// Wait for the game to be ready before fully initializing the HUD
		QH::BindOnGameReady(this, n"OnGameReady");
	}

	////////////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintOverride)
	protected void Destruct()
	{
		Widget_InventoryStorage.UninitializeInventory();
		Widget_InventoryQuickBar.UninitializeInventory();
	}

	//~ UUserWidget END
}

//______________________________________________________________________________
namespace QH
{
	////////////////////////////////////////////////////////////////////////////////
	AQHASHUD GetHUD()
	{
		APlayerController PlayerController = QH::GetPlayerController();

		return (PlayerController != nullptr)
			 ? Cast<AQHASHUD>(PlayerController.GetHUD())
			 : nullptr;
	}
}
