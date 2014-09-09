// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once


#define LOCTEXT_NAMESPACE "SProjectLauncherCookedPlatforms"


/**
 * Implements the cooked platforms panel.
 */
class SProjectLauncherCookedPlatforms
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SProjectLauncherCookedPlatforms) { }
	SLATE_END_ARGS()

public:

	/**
	 * Constructs the widget.
	 *
	 * @param InArgs The Slate argument list.
	 * @param InModel The data model.
	 */
	void Construct(	const FArguments& InArgs, const FProjectLauncherModelRef& InModel )
	{
		Model = InModel;

		MakePlatformMenu();

		ChildSlot
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					// platform menu
					SAssignNew(PlatformListView, SListView<TSharedPtr<FString> >)
					.HeaderRow(
					SNew(SHeaderRow)
					.Visibility(EVisibility::Collapsed)

					+ SHeaderRow::Column("PlatformName")
					.DefaultLabel(LOCTEXT("PlatformListPlatformNameColumnHeader", "Platform"))
					.FillWidth(1.0f)
					)
					.ItemHeight(16.0f)
					.ListItemsSource(&PlatformList)
					.OnGenerateRow(this, &SProjectLauncherCookedPlatforms::HandlePlatformListViewGenerateRow)
					.SelectionMode(ESelectionMode::None)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 6.0f, 0.0f, 4.0f)
				[
					SNew(SSeparator)
						.Orientation(Orient_Horizontal)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.HAlign(HAlign_Right)
						[
							SNew(STextBlock)
								.Text(LOCTEXT("SelectLabel", "Select:"))
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(8.0f, 0.0f)
						[
							// all platforms hyper-link
							SNew(SHyperlink)
								.OnNavigate(this, &SProjectLauncherCookedPlatforms::HandleAllPlatformsHyperlinkNavigate, true)
								.Text(LOCTEXT("AllPlatformsHyperlinkLabel", "All"))
								.ToolTipText(LOCTEXT("AllPlatformsButtonTooltip", "Select all available platforms."))
								.Visibility(this, &SProjectLauncherCookedPlatforms::HandleAllPlatformsHyperlinkVisibility)									
						]

					+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							// no platforms hyper-link
							SNew(SHyperlink)
								.OnNavigate(this, &SProjectLauncherCookedPlatforms::HandleAllPlatformsHyperlinkNavigate, false)
								.Text(LOCTEXT("NoPlatformsHyperlinkLabel", "None"))
								.ToolTipText(LOCTEXT("NoPlatformsHyperlinkTooltip", "Deselect all platforms."))
								.Visibility(this, &SProjectLauncherCookedPlatforms::HandleAllPlatformsHyperlinkVisibility)									
						]
				]
		];
	}

protected:

	/**
	 * Builds the platform menu.
	 *
	 * @return Platform menu widget.
	 */
	void MakePlatformMenu( )
	{
		TArray<ITargetPlatform*> Platforms = GetTargetPlatformManager()->GetTargetPlatforms();

		if (Platforms.Num() > 0)
		{
			PlatformList.Reset();
			for (int32 PlatformIndex = 0; PlatformIndex < Platforms.Num(); ++PlatformIndex)
			{
				FString PlatformName = Platforms[PlatformIndex]->PlatformName();

				PlatformList.Add(MakeShareable(new FString(PlatformName)));
			}
		}
	}

private:

	// Callback for clicking the 'Select All Platforms' button.
	void HandleAllPlatformsHyperlinkNavigate( bool AllPlatforms )
	{
		ILauncherProfilePtr SelectedProfile = Model->GetSelectedProfile();

		if (SelectedProfile.IsValid())
		{
			if (AllPlatforms)
			{
				TArray<ITargetPlatform*> Platforms = GetTargetPlatformManager()->GetTargetPlatforms();

				for (int32 PlatformIndex = 0; PlatformIndex < Platforms.Num(); ++PlatformIndex)
				{
					SelectedProfile->AddCookedPlatform(Platforms[PlatformIndex]->PlatformName());
				}
			}
			else
			{
				SelectedProfile->ClearCookedPlatforms();
			}
		}
	}

	// Callback for determining the visibility of the 'Select All Platforms' button.
	EVisibility HandleAllPlatformsHyperlinkVisibility( ) const
	{
		if (GetTargetPlatformManager()->GetTargetPlatforms().Num() > 1)
		{
			return EVisibility::Visible;
		}

		return EVisibility::Collapsed;
	}

	// Callback for getting the color of a platform menu check box.
	FSlateColor HandlePlatformMenuEntryColorAndOpacity( FString PlatformName ) const
	{
		ILauncherProfilePtr SelectedProfile = Model->GetSelectedProfile();

		if (SelectedProfile.IsValid())
		{
			ITargetPlatform* TargetPlatform = GetTargetPlatformManager()->FindTargetPlatform(PlatformName);

			if (TargetPlatform != NULL)
			{
//				if (TargetPlatform->HasValidBuild(SelectedProfile->GetProjectPath(), SelectedProfile->GetBuildConfiguration()))
				{
					return FEditorStyle::GetColor("Foreground");
				}
			}
		}

		return FLinearColor::Yellow;
	}

	// Handles generating a row widget in the map list view.
	TSharedRef<ITableRow> HandlePlatformListViewGenerateRow( TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable )
	{
		return SNew(SProjectLauncherPlatformListRow, Model.ToSharedRef())
			.PlatformName(InItem)
			.OwnerTableView(OwnerTable);
	}

private:

	// Holds a pointer to the data model.
	FProjectLauncherModelPtr Model;

	// Holds the platform list.
	TArray<TSharedPtr<FString> > PlatformList;

	// Holds the platform list view.
	TSharedPtr<SListView<TSharedPtr<FString> > > PlatformListView;

};


#undef LOCTEXT_NAMESPACE