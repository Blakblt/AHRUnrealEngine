// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#pragma once

class UNREALED_API SEditorViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEditorViewport) {}
	SLATE_END_ARGS()
	
	SEditorViewport();
	virtual ~SEditorViewport();

	void Construct( const FArguments& InArgs );

	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent ) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	
	/**
	 * @return True if the viewport is being updated in realtime
	 */
	bool IsRealtime() const;

	/** @return True if the viewport is currently visible */
	virtual bool IsVisible() const;

	/**
	 * @return true if the specified coordinate system the active one active
	 */
	virtual bool IsCoordSystemActive( ECoordSystem CoordSystem ) const;
	
	/** @return The viewport command list */
	const TSharedPtr<FUICommandList> GetCommandList() const { return CommandList; }

	TSharedPtr<FEditorViewportClient> GetViewportClient() const { return Client; }

	/**
	 * Controls the visibility of the widget transform toolbar, if there is an associated toolbar
	 */
	virtual EVisibility GetTransformToolbarVisibility() const;

protected:
	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() = 0;
	virtual TSharedPtr<SWidget> MakeViewportToolbar() = 0;
	virtual void BindCommands();
	virtual const FSlateBrush* OnGetViewportBorderBrush() const { return NULL; }
	virtual FSlateColor OnGetViewportBorderColorAndOpacity() const { return FLinearColor::Black; }
	
	/**
	 * @return The visibility of widgets in the viewport (e.g, menus).  Note this is not the visibility of the scene rendered in the viewport                                                              
	 */
	virtual EVisibility OnGetViewportContentVisibility() const;

	/** UI Command delegate bindings */
	void OnToggleRealtime();
	void OnToggleStats();

	/**
	 * Toggles Stat command visibility in this viewport
	 *
	 * @param CommandName				Name of the command
	 */
	virtual void ToggleStatCommand(FString CommandName);

	/**
	 * Checks if Stat command is visible in this viewport
	 *
	 * @param CommandName				Name of the command
	 */
	virtual bool IsStatCommandVisible(FString CommandName) const;

	/**
	 * Toggles a show flag in this viewport
	 *
	 * @param EngineShowFlagIndex	the ID to toggle
	 */
	void ToggleShowFlag( uint32 EngineShowFlagIndex );

	/**
	 * Checks if a show flag is enabled in this viewport
	 *
	 * @param EngineShowFlagIndex	the ID to check
	 * @return true if the show flag is enabled, false otherwise
	 */
	bool IsShowFlagEnabled( uint32 EngineShowFlagIndex ) const;

	/**
	 * Changes the exposure setting for this viewport
	 *
	 * @param ID	The ID of the new exposure setting
	 */
	void ChangeExposureSetting( int32 ID );

	/**
	 * Checks if an exposure setting is selected
	 * 
	 * @param ID	The ID of the exposure setting to check
	 * @return		true if the exposure setting is checked
	 */
	bool IsExposureSettingSelected( int32 ID ) const;
	

	virtual void OnScreenCapture();
	virtual void OnScreenCaptureForProjectThumbnail();
	virtual bool DoesAllowScreenCapture() { return true; }
	
	/**
	 * Changes the snapping grid size
	 */
	virtual void OnIncrementPositionGridSize() {};
	virtual void OnDecrementPositionGridSize() {};
	virtual void OnIncrementRotationGridSize() {};
	virtual void OnDecrementRotationGridSize() {};

	/**
	 * @return true if the specified widget mode is active
	 */
	virtual bool IsWidgetModeActive( FWidget::EWidgetMode Mode ) const;

	/**
	 * @return true if the translate/rotate mode widget is visible 
	 */
	virtual bool IsTranslateRotateModeVisible() const;

	/**
	 * Moves between widget modes
	 */
	virtual void OnCycleWidgetMode();

	/**
	 * Cycles between world and local coordinate systems
	 */
	virtual void OnCycleCoordinateSystem();

	/**
	 * Called when the user wants to focus the viewport to the current selection
	 */
	virtual void OnFocusViewportToSelection(){}

	/** Gets the world this viewport is for */
	virtual UWorld* GetWorld() const;

	/**
	 * Called when surface snapping has been enabled/disabled
	 */
	static void OnToggleSurfaceSnap();

	/**
	 * Called to test whether surface snapping is enabled or not
	 */
	static bool OnIsSurfaceSnapEnabled();

protected:
	TSharedPtr<SOverlay> ViewportOverlay;
	/** Viewport that renders the scene provided by the viewport client */
	TSharedPtr<class FSceneViewport> SceneViewport;
	/** Widget where the scene viewport is drawn in */
	TSharedPtr<SViewport> ViewportWidget;
	/** The client responsible for setting up the scene */
	TSharedPtr<FEditorViewportClient> Client;
	/** Commandlist used in the viewport (Maps commands to viewport specific actions) */
	TSharedPtr<FUICommandList> CommandList;
	/** The last time the viewport was ticked (for visibility determination) */
	double LastTickTime;
};
