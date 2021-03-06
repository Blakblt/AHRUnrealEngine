// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#pragma once

#include "Persona.h"
#include "GraphEditor.h"
#include "SNodePanel.h"
#include "SCurveEditor.h"
#include "SAnimTrackPanel.h"
#include "SAnimEditorBase.h"

DECLARE_DELEGATE_OneParam( FOnSelectionChanged, const FGraphPanelSelectionSet& )
DECLARE_DELEGATE( FOnTrackSelectionChanged )
DECLARE_DELEGATE( FOnUpdatePanel )
DECLARE_DELEGATE_RetVal( float, FOnGetScrubValue )
DECLARE_DELEGATE( FRefreshOffsetsRequest )
DECLARE_DELEGATE( FDeleteNotify )
DECLARE_DELEGATE( FDeselectAllNotifies )
DECLARE_DELEGATE( FCopyNotifies )
DECLARE_DELEGATE_OneParam( FOnGetBlueprintNotifyData, TArray<FAssetData>& )

class SAnimNotifyNode;
class SAnimNotifyTrack;
class FNotifyDragDropOp;

namespace ENotifyPasteMode
{
	enum Type
	{
		MousePosition,
		OriginalTime
	};
}

namespace ENotifyPasteMultipleMode
{
	enum Type
	{
		Relative,
		Absolute
	};
}

namespace ENotifyStateHandleHit
{
	enum Type
	{
		Start,
		End,
		None
	};
}

struct FNotifyMarqueeOperation
{
	FNotifyMarqueeOperation()
		: Operation(Add)
		, bActive(false)
	{
	}

	enum Type
	{
		/** Holding down Ctrl removes nodes */
		Remove,
		/** Holding down Shift adds to the selection */
		Add,
		/** When nothing is pressed, marquee replaces selection */
		Replace
	} Operation;

	bool IsValid() const
	{
		return Rect.IsValid() && bActive;
	}

	void Start(const FVector2D& InStartLocation, FNotifyMarqueeOperation::Type InOperationType, TArray<TSharedPtr<SAnimNotifyNode>>& InOriginalSelection)
	{
		Rect = FMarqueeRect(InStartLocation);
		Operation = InOperationType;
		OriginalSelection = InOriginalSelection;
	}

	void End()
	{
		Rect = FMarqueeRect();
	}


	/** Given a mouse event, figure out what the marquee selection should do based on the state of Shift and Ctrl keys */
	static FNotifyMarqueeOperation::Type OperationTypeFromMouseEvent(const FPointerEvent& MouseEvent)
	{
		if(MouseEvent.IsControlDown())
		{
			return FNotifyMarqueeOperation::Remove;
		}
		else if(MouseEvent.IsShiftDown())
		{
			return FNotifyMarqueeOperation::Add;
		}
		else
		{
			return FNotifyMarqueeOperation::Replace;
		}
	}

public:
	/** The marquee rectangle being dragged by the user */
	FMarqueeRect Rect;

	/** Whether the marquee has been activated, usually by a drag */
	bool bActive;

	/** The original selection state before the marquee selection */
	TArray<TSharedPtr<SAnimNotifyNode>> OriginalSelection;
};

//////////////////////////////////////////////////////////////////////////
// SAnimNotifyPanel

class FAnimNotifyPanelCommands : public TCommands<FAnimNotifyPanelCommands>
{
public:
	FAnimNotifyPanelCommands()
		: TCommands<FAnimNotifyPanelCommands>("AnimNotifyPanel", NSLOCTEXT("Contexts", "AnimNotifyPanel", "Anim Notify Panel"), NAME_None, FEditorStyle::GetStyleSetName())
	{

	}

	TSharedPtr<FUICommandInfo> DeleteNotify;

	virtual void RegisterCommands() override;
};

// @todo anim : register when it's opened for the animsequence
// broadcast when animsequence changed, so that we refresh for multiple window
class SAnimNotifyPanel: public SAnimTrackPanel
{
public:
	SLATE_BEGIN_ARGS( SAnimNotifyPanel )
		: _Persona()
		, _Sequence()
		, _CurrentPosition()
		, _ViewInputMin()
		, _ViewInputMax()
		, _InputMin()
		, _InputMax()
		, _OnSetInputViewRange()
		, _OnSelectionChanged()
		, _OnGetScrubValue()
		, _OnRequestRefreshOffsets()
	{}

	SLATE_ARGUMENT( TSharedPtr<FPersona>,	Persona )
	SLATE_ARGUMENT( class UAnimSequenceBase*, Sequence)
	SLATE_ARGUMENT( float, WidgetWidth )
	SLATE_ATTRIBUTE( float, CurrentPosition )
	SLATE_ATTRIBUTE( float, ViewInputMin )
	SLATE_ATTRIBUTE( float, ViewInputMax )
	SLATE_ATTRIBUTE( float, InputMin )
	SLATE_ATTRIBUTE( float, InputMax )
	SLATE_ATTRIBUTE( TArray<FTrackMarkerBar>, MarkerBars )
	SLATE_EVENT( FOnSetInputViewRange, OnSetInputViewRange )
	SLATE_EVENT( FOnSelectionChanged, OnSelectionChanged )
	SLATE_EVENT( FOnGetScrubValue, OnGetScrubValue )
	SLATE_EVENT( FRefreshOffsetsRequest, OnRequestRefreshOffsets)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SAnimNotifyPanel();

	void SetSequence(class UAnimSequenceBase *	InSequence);

	FReply InsertTrack(int32 TrackIndexToInsert);
	FReply DeleteTrack(int32 TrackIndexToDelete);
	bool CanDeleteTrack(int32 TrackIndexToDelete);
	void DeleteNotify(FAnimNotifyEvent* Notify);
	void Update();
	TWeakPtr<FPersona> GetPersona() const { return PersonaPtr; }

	/** Returns the position of the notify node currently being dragged. Returns -1 if no node is being dragged */
	float CalculateDraggedNodePos() const;

	/**Handler for when a notify node drag has been initiated */
	FReply OnNotifyNodeDragStarted(TArray<TSharedPtr<SAnimNotifyNode>> NotifyNodes, TSharedRef<SWidget> Decorator, const FVector2D& ScreenCursorPos, const FVector2D& ScreenNodePosition, const bool bDragOnMarker);

	virtual float GetSequenceLength() const override {return Sequence->SequenceLength;}

	void CopySelectedNotifiesToClipboard() const;
	void OnPasteNotifies(SAnimNotifyTrack* RequestTrack, float ClickTime, ENotifyPasteMode::Type PasteMode, ENotifyPasteMultipleMode::Type MultiplePasteType);

	/** Handler for properties changing on objects */
	FCoreUObjectDelegates::FOnObjectPropertyChanged::FDelegate OnPropertyChangedHandle;
	void OnPropertyChanged(UObject* ChangedObject, FPropertyChangedEvent& PropertyEvent);

	/** SWidget Interface */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);	
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent);
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	/** End SWidget Interface */

	void RefreshMarqueeSelectedNodes(const FGeometry& PanelGeo);

	void OnNotifyObjectChanged(UObject* EditorBaseObj, bool bRebuild);

private:
	TSharedPtr<SBorder> PanelArea;
	class UAnimSequenceBase* Sequence;
	float WidgetWidth;
	TAttribute<float> CurrentPosition;
	FOnSelectionChanged OnSelectionChanged;
	FOnGetScrubValue OnGetScrubValue;
	
	/** Manager for mouse controlled marquee selection */
	FNotifyMarqueeOperation Marquee;

	/** Delegate to request a refresh of the offsets calculated for notifies */
	FRefreshOffsetsRequest OnRequestRefreshOffsets;

	/** Store the position of a currently dragged node for display across tracks */
	float CurrentDragXPosition;

	/** Cached list of anim tracks for notify node drag drop */
	TArray<TSharedPtr<SAnimNotifyTrack>> NotifyAnimTracks;

	// Read common info from the clipboard
	bool ReadNotifyPasteHeader(FString& OutPropertyString, const TCHAR*& OutBuffer, float& OutOriginalTime, float& OutOriginalLength, int32& OutTrackSpan) const;

	// this just refresh notify tracks - UI purpose only
	// do not call this from here. This gets called by asset. 
	void RefreshNotifyTracks();
	void PostUndo();

	/** Handler for delete command */
	void OnDeletePressed();

	/** We support keyboard focus to detect when we should process key commands like delete */
	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	// Called when a track changes it's selection; iterates all tracks collecting selected items
	void OnTrackSelectionChanged();

	// Called to deselect all notifies across all tracks
	void DeselectAllNotifies();

	// Binds the UI commands for this widget to delegates
	void BindCommands();

	/** Populates the given class array with all classes deriving from those originally present
	 * @param InAssetsToSearch Assets to search to detect child classes
	 * @param InOutAllowedClassNames Classes to allow, this will be expanded to cover all derived classes of those originally present
	 */
	void PopulateNotifyBlueprintClasses(TArray<FString>& InOutAllowedClasses);

	/** Find blueprints matching allowed classes and all derived blueprints 
	 * @param OutNotifyData Asset data matching allowed classes and their children
	 * @param InOutAllowedClassNames Classes to allow, this will be expanded to cover all derived classes of those originally present
	 */
	void OnGetNotifyBlueprintData(TArray<FAssetData>& OutNotifyData, TArray<FString>* InOutAllowedClassNames);

	/** Persona reference **/
	TWeakPtr<FPersona> PersonaPtr;

	/** Attribute for accessing any section/branching point positions we have to draw */
	TAttribute<TArray<FTrackMarkerBar>>	MarkerBars;

	/** UI commands for this widget */
	TSharedPtr<FUICommandList> UICommandList;

	/** Classes that are known to be derived from blueprint notifies */
	TArray<FString> NotifyClassNames;

	/** Classes that are known to be derived from blueprint state notifies */
	TArray<FString> NotifyStateClassNames;

	/** Handle to the registered OnPropertyChangedHandle delegate */
	FDelegateHandle OnPropertyChangedHandleDelegateHandle;
};