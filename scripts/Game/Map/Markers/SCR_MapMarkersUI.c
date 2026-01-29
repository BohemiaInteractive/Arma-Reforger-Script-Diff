
void MarkerPlacedInvoker(int posX, int posY, bool isLocal);	// world pos X, world pos Y, is visible to everyone or just the player who placed it
typedef func MarkerPlacedInvoker;

//! Markers UI map component 
class SCR_MapMarkersUI : SCR_MapUIBaseComponent
{
	[Attribute("#AR-MapMarker_ParentCategory", UIWidgets.Auto, "Menu category name" )]
	protected string m_sCategoryName;
	
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourceNamePicker, desc: "Icons imageset", params: "imageset" )]
	protected ResourceName m_sIconImageset;
	
	[Attribute("scenarios", UIWidgets.Auto, "Category icon quad" )]
	protected string m_sCategoryIconName;
	
	[Attribute("{46C46D97D1FE6241}UI/layouts/Map/MapMarkerEditBox.layout", UIWidgets.ResourceNamePicker, desc: "Edit box dialog when placing custom marker", params: "layout" )]
	protected ResourceName m_sEditBoxLayout;
	
	[Attribute("{DEA2D3B788CDCB4F}UI/layouts/Map/MapIconSelectorEntry.layout", UIWidgets.ResourceNamePicker, desc: "Icon selection entry layout", params: "layout" )]
	protected ResourceName m_sSelectorIconEntry;
	
	[Attribute("{8A5D43FC8AC6C171}UI/layouts/Map/MapColorSelectorEntry.layout", UIWidgets.ResourceNamePicker, desc: "Color selection entry layout", params: "layout" )]
	protected ResourceName m_sSelectorColorEntry;
	
	[Attribute("{4B6A50B3D8200779}UI/layouts/Map/MapDimensionSelectorEntry.layout", UIWidgets.ResourceNamePicker, desc: "Dimension selection entry layout", params: "layout" )]
	protected ResourceName m_sSelectorDimensionEntry;
	
	[Attribute("{DF5BCE91F8A59977}UI/layouts/Map/MapMilitaryMarkerEditBox.layout", UIWidgets.ResourceNamePicker, desc: "Edit box dialog when placing military symbol marker", params: "layout" )]
	protected ResourceName m_sMilitaryEditBoxLayout;
	
	[Attribute("cancel", UIWidgets.Auto, "Delete icon quad" )]
	protected string m_sDeleteIconName;
	
	[Attribute("20", UIWidgets.Auto, "Icon selector entries per line" )]
	protected int m_iIconsPerLine;

	protected const int USERID_EDITBOX = 1000;		// unique id set to editbox allowing us to find it in case there are other editboxes
	protected const int USERID_EDITBOX_MIL = 1001; 	// unique id set to military editbox allowing us to find it in case there are other editboxes
	protected const string ICON_ENTRY = "IconEntry";
	protected const string COLOR_ENTRY = "ColorEntry";
	protected const string ICON_SELECTOR = "IconSelector";
	protected const string COLOR_SELECTOR = "ColorSelector";
	protected const ResourceName SELECTOR_LINE = "{CF8EC7A0D310A8D9}UI/layouts/Map/MapColorSelectorLine.layout";
	protected const int SPIN_BOX_YES = 1;
	
	protected ref Color BACKGROUND_DEFAULT = new Color(4,4,4,255);
	protected ref Color BACKGROUND_SELECTED = new Color(16,16,16,255);
 	
	protected bool m_bIsMilitaryMarker;								// is custom or military marker
	protected int m_bIsDelayed;										// used to delay input context for marker dialog by one frame so it doesnt trigger inputs used to open it
	protected int m_iIconEntryCount;								// how many icon entries are within the current tab
	protected int m_iIconLines;	
	protected SCR_MapMarkerEntryPlaced m_PlacedMarkerConfig;		// saved entry for custom text placed markers
	protected SCR_MapMarkerEntryMilitary m_MilitaryMarkerConfig;
	protected Widget m_MarkerEditRoot;
	protected Widget m_IconSelector;
	protected ImageWidget m_wMarkerPreview;
	protected ImageWidget m_wMarkerPreviewGlow;
	protected TextWidget m_wMarkerPreviewText;
	protected OverlayWidget m_wMarkerPreviewMilitary;
	protected SCR_MilitarySymbolUIComponent m_MarkerPreviewMilitaryComp;
	protected ref SCR_MilitarySymbol m_MilSymbolPreview;
	protected SCR_TabViewComponent m_TabComponent;
	protected SCR_EditBoxComponent m_EditBoxComp;
	protected SCR_SliderComponent m_SliderComp;
	protected TextWidget m_wFactionSelectionText;
	protected TextWidget m_wDimensionSelectionText;
	protected SCR_ComboBoxComponent m_ComboBoxComp1;
	protected SCR_ComboBoxComponent m_ComboBoxComp2;
	protected SCR_SelectionMenuCategoryEntry m_ReconCategoryEntry;
	protected SCR_SpinBoxComponent m_TimestampSpinBox;
	
	// Placed marker attributes
	protected int m_iWantedIconEntry;
	protected int m_iSelectedIconID;							// used for selecting icon when edit dialog is confirmed
	protected int m_iSelectedColorID;
	protected int m_iSelectedFactionID;
	protected int m_iWantedDimensionEntry;
	protected int m_iSelectedDimensionID;
	protected float m_fRotation;
	protected bool m_bIsTimestampVisible;
	protected SCR_MarkerMilitaryType m_eMilitaryTypeA;
	protected SCR_MarkerMilitaryType m_eMilitaryTypeB;
	protected EMilitarySymbolIcon m_eMilitaryTypeAIcon;
	protected EMilitarySymbolIcon m_eMilitaryTypeBIcon;
	protected SCR_ButtonBaseComponent m_SelectedIconButton;		// used for (un)coloring and selecting proper button when navigating on controller
	protected SCR_ButtonBaseComponent m_SelectedColorButton;
	protected SCR_ButtonBaseComponent m_SelectedFactionButton;
	protected SCR_ButtonBaseComponent m_SelectedDimensionButton;
	
	protected SCR_MapMarkerManagerComponent m_MarkerMgr;
	protected SCR_SelectionMenuCategoryEntry m_RootCategoryEntry;
	protected SCR_SelectionMenuEntry m_MarkerRemoveEntry;
	protected SCR_MapMarkerBase m_RemovableMarker;
	protected SCR_MapMarkerBase m_EditedMarker;					// when edit is used, original marked being edited is hidden and cached here so it doesnt clash with edit preview
	protected SCR_MapCursorModule m_CursorModule;
	
	protected ref ScriptInvokerBase<MarkerPlacedInvoker> m_OnCustomMarkerPlaced = new ScriptInvokerBase<MarkerPlacedInvoker>();
	
	protected ref map<SCR_ButtonBaseComponent, int> m_mIconIDs = new map<SCR_ButtonBaseComponent, int>();	// marker icon buttons to config ids
	protected ref map<SCR_ButtonBaseComponent, int> m_mColorIDs = new map<SCR_ButtonBaseComponent, int>();	// marker color buttons to config ids
	protected ref map<SCR_ButtonBaseComponent, int> m_mFactionIDs = new map<SCR_ButtonBaseComponent, int>();// marker faction buttons to config ids
	protected ref map<SCR_ButtonBaseComponent, int> m_mDimensionIDs = new map<SCR_ButtonBaseComponent, int>();// marker dimension buttons to config ids
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<MarkerPlacedInvoker> GetOnCustomMarkerPlaced()
	{
		return m_OnCustomMarkerPlaced; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether the marker is owned by the local player
	static bool IsOwnedMarker(notnull SCR_MapMarkerBase marker)
	{
		PlayerController localController = GetGame().GetPlayerController();
		if (localController)
		{
			if (marker.GetMarkerOwnerID() == localController.GetPlayerId())
				return true;
		}
		
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Create custom marker dialog
	//! \param tabID is ID of selected tabWidget tab, if not set first is default
	//! \param selectedIconEntry is ID of selected icon, if not set first is default
	protected void CreateMarkerEditDialog(bool isEditing = false, int tabID = 0, int selectedIconEntry = -1, int selectedColorEntry = -1)
	{
		m_bIsMilitaryMarker = false;
		
		if (m_MarkerEditRoot)
			CleanupMarkerEditWidget();
		
		m_iWantedIconEntry = selectedIconEntry;
		
		m_MarkerEditRoot = GetGame().GetWorkspace().CreateWidgets(m_sEditBoxLayout, m_RootWidget);
		
		float screenX, screenY;
		m_MapEntity.GetMapWidget().GetScreenSize(screenX, screenY);
		FrameSlot.SetPos(m_MarkerEditRoot, GetGame().GetWorkspace().DPIUnscale(screenX * 0.5), GetGame().GetWorkspace().DPIUnscale(screenY * 0.5));
		
		m_wMarkerPreview = ImageWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerIcon"));
		m_wMarkerPreviewGlow = ImageWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerIconGlow"));
		m_wMarkerPreviewText = TextWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerText"));
		
		InitColorIcons(selectedColorEntry);
		
		Widget sliderRotation = m_MarkerEditRoot.FindAnyWidget("SliderRoot");
		m_SliderComp = SCR_SliderComponent.Cast(sliderRotation.FindHandler(SCR_SliderComponent));
		m_SliderComp.m_OnChanged.Insert(OnSliderChanged);
		
		Widget categoryTab = m_MarkerEditRoot.FindAnyWidget("MarkerEditTab");
		m_TabComponent = SCR_TabViewComponent.Cast(categoryTab.FindHandler(SCR_TabViewComponent));
		
		array<ref SCR_MarkerIconCategory> categoriesArr = m_PlacedMarkerConfig.GetIconCategories();
		foreach (SCR_MarkerIconCategory category : categoriesArr)
		{
			m_TabComponent.AddTab(string.Empty, category.m_sName, identifier: category.m_sIdentifier);
		}
		
		m_TabComponent.GetOnChanged().Insert(OnTabChanged);
		m_TabComponent.ShowTab(tabID, true, false);
		
		Widget editBoxRoot = m_MarkerEditRoot.FindAnyWidget("EditBoxRoot");
		editBoxRoot.FindAnyWidget("EditBox").SetUserID(USERID_EDITBOX);
		m_EditBoxComp = SCR_EditBoxComponent.Cast(editBoxRoot.FindHandler(SCR_EditBoxComponent));
		m_EditBoxComp.m_OnTextChange.Insert(OnEditBoxTextChanged);
		m_EditBoxComp.SetValue(string.Empty);
				
		SCR_InputButtonComponent confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonPublic").FindHandler(SCR_InputButtonComponent));
		
		if (isEditing)
		{
			if (m_EditedMarker.GetMarkerID() >= 0)
				confirmComp.m_OnClicked.Insert(OnPlaceMarkerConfirmed);
			else
				confirmComp.m_OnClicked.Insert(OnPlaceMarkerConfirmedPrivate);
			
			confirmComp.SetLabel("#AR-ServerHosting_Edit");
			
			m_MarkerEditRoot.FindAnyWidget("ButtonPrivate").SetVisible(false);
			m_MarkerEditRoot.FindAnyWidget("ButtonPrivate").SetEnabled(false);
		}
		else 
		{
			confirmComp.m_OnClicked.Insert(OnPlaceMarkerConfirmed);
			confirmComp.SetLabel("#AR-MapMarker_PlacePublic");
			
			SocialComponent sc = GetSocialComponent();
			if (sc && !sc.IsPrivilegedTo(EUserInteraction.UserGeneratedContent))
				confirmComp.SetEnabled(false);
			else
				confirmComp.SetEnabled(true);
		
			confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonPrivate").FindHandler(SCR_InputButtonComponent));
			confirmComp.m_OnClicked.Insert(OnPlaceMarkerConfirmedPrivate);
		}
				
		confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonCancel").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(OnEditCancelled);
			
		FocusWidget(m_SelectedIconButton.GetRootWidget());
		
		m_CursorModule.HandleDialog(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateMilitaryMarkerEditDialog(bool isEditing = false, int selectedFactionEntry = -1, int selectedDimensionEntry = -1)
	{
		m_bIsMilitaryMarker = true;
		m_iWantedDimensionEntry = selectedDimensionEntry;
		
		m_MarkerEditRoot = GetGame().GetWorkspace().CreateWidgets(m_sMilitaryEditBoxLayout, m_RootWidget);
		
		float screenX, screenY;
		m_MapEntity.GetMapWidget().GetScreenSize(screenX, screenY);
		FrameSlot.SetPos(m_MarkerEditRoot, GetGame().GetWorkspace().DPIUnscale(screenX * 0.5), GetGame().GetWorkspace().DPIUnscale(screenY * 0.5));
		
		m_wMarkerPreviewMilitary = OverlayWidget.Cast(m_MarkerEditRoot.FindAnyWidget("SymbolOverlay"));
		m_MarkerPreviewMilitaryComp = SCR_MilitarySymbolUIComponent.Cast(m_wMarkerPreviewMilitary.FindHandler(SCR_MilitarySymbolUIComponent));
		m_wMarkerPreviewText = TextWidget.Cast(m_MarkerEditRoot.FindAnyWidget("MarkerText"));
		
		m_MilSymbolPreview = new SCR_MilitarySymbol();
		m_MilSymbolPreview.SetIdentity(EMilitarySymbolIdentity.BLUFOR);
		m_MilSymbolPreview.SetDimension(EMilitarySymbolDimension.LAND);
		
		InitFactionIcons(selectedFactionEntry);
						
		m_ComboBoxComp1 = SCR_ComboBoxComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ComboBox1").FindHandler(SCR_ComboBoxComponent));
		m_ComboBoxComp1.m_OnChanged.Insert(OnComboBoxChangedA);
		
		m_ComboBoxComp2 = SCR_ComboBoxComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ComboBox2").FindHandler(SCR_ComboBoxComponent));
		m_ComboBoxComp2.m_OnChanged.Insert(OnComboBoxChangedB);
			
		m_ComboBoxComp1.AddItem("");
		m_ComboBoxComp2.AddItem("");
		
		array<ref SCR_MarkerMilitaryType> types = m_MilitaryMarkerConfig.GetMilitaryTypes();
		foreach (int i, SCR_MarkerMilitaryType markerType : types)
		{
			m_ComboBoxComp1.AddItem(markerType.GetTranslation(), false, markerType);
			m_ComboBoxComp2.AddItem(markerType.GetTranslation(), false, markerType);
		}
				
		m_ComboBoxComp1.SetCurrentItem(0);
		OnComboBoxChangedA(m_ComboBoxComp1, -1);
		
		m_ComboBoxComp2.SetCurrentItem(0);
		OnComboBoxChangedB(m_ComboBoxComp2, -1);
		
		Widget editBoxRoot = m_MarkerEditRoot.FindAnyWidget("EditBoxRoot");
		editBoxRoot.FindAnyWidget("EditBox").SetUserID(USERID_EDITBOX_MIL);
		m_EditBoxComp = SCR_EditBoxComponent.Cast(editBoxRoot.FindHandler(SCR_EditBoxComponent));
		m_EditBoxComp.m_OnTextChange.Insert(OnEditBoxTextChanged);
		m_EditBoxComp.SetValue(string.Empty);
		
		SCR_InputButtonComponent confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonPublic").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(OnPlaceMarkerConfirmed);
		
		if (isEditing)
			confirmComp.SetLabel("#AR-ServerHosting_Edit");
		else 
			confirmComp.SetLabel("#AR-MapMarker_PlacePublic");
		
		confirmComp = SCR_InputButtonComponent.Cast(m_MarkerEditRoot.FindAnyWidget("ButtonCancel").FindHandler(SCR_InputButtonComponent));
		confirmComp.m_OnClicked.Insert(OnEditCancelled);
		
		FocusWidget(m_SelectedFactionButton.GetRootWidget());
		
		m_CursorModule.HandleDialog(true);

		Widget timestampWidget = m_MarkerEditRoot.FindAnyWidget("TimestampSpinBox");
		if (timestampWidget)
		{
			m_TimestampSpinBox = SCR_SpinBoxComponent.Cast(timestampWidget.FindHandler(SCR_SpinBoxComponent));
			m_TimestampSpinBox.m_OnChanged.Insert(OnTimestampSpinBoxChanged);
			m_TimestampSpinBox.SetCurrentItem(SPIN_BOX_YES, false, false);
			OnTimestampSpinBoxChanged(m_TimestampSpinBox, SPIN_BOX_YES);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	protected void FocusWidget(Widget widget)
	{
		GetGame().GetWorkspace().SetFocusedWidget(widget);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init faction icons
	protected void InitFactionIcons(int selectedFactionEntry)
	{
		array<ref SCR_MarkerMilitaryFactionEntry> factionsArr = m_MilitaryMarkerConfig.GetMilitaryFactionEntries();
		Widget factionSelector = m_MarkerEditRoot.FindAnyWidget("FactionSelector");		
		Widget factionSelectorLine = factionSelector.FindAnyWidget("FactionSelectorLine");
		m_wFactionSelectionText = TextWidget.Cast(factionSelector.FindAnyWidget("TextSelection"));
		SCR_ButtonImageComponent firstFactionEntry;
		
		m_mFactionIDs.Clear();
		m_SelectedFactionButton = null;
		
		string imageset, quad;
		
		foreach (int i, SCR_MarkerMilitaryFactionEntry factionEntry : factionsArr)
		{			
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorIconEntry, factionSelectorLine);
			button.SetName("FactionEntry" + i.ToString());
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			buttonComp.GetImageWidget().SetColor(factionEntry.GetColor());
			factionEntry.GetIconResource(imageset, quad);
			buttonComp.SetImage(imageset, quad);
		
			buttonComp.m_OnClicked.Insert(OnFactionEntryClicked);
			buttonComp.m_OnFocus.Insert(OnFactionEntryFocused);
			
			m_mFactionIDs.Insert(buttonComp, i);
			
			if (!firstFactionEntry)
				firstFactionEntry = buttonComp;
		}
		
		if (selectedFactionEntry == -1)
		{
			OnFactionEntryClicked(firstFactionEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = SCR_MapHelper<SCR_ButtonBaseComponent, int>.GetKeyByValue(m_mFactionIDs, selectedFactionEntry);
			if (buttonComp)
				OnFactionEntryClicked(buttonComp);
			else 
				OnFactionEntryClicked(firstFactionEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init dimension icons
	protected void InitDimensionIcons()
	{
		SCR_MarkerMilitaryFactionEntry factionEntry = m_MilitaryMarkerConfig.GetFactionEntry(m_iSelectedFactionID);
		array<ref SCR_MarkerMilitaryDimension> dimensionsArr = m_MilitaryMarkerConfig.GetMilitaryDimensions();
		if (!factionEntry || dimensionsArr.IsEmpty())
			return;
		
		Widget dimensionSelector = m_MarkerEditRoot.FindAnyWidget("DimensionSelector");		
		Widget dimensionSelectorLine = dimensionSelector.FindAnyWidget("DimensionSelectorLine");
		m_wDimensionSelectionText = TextWidget.Cast(dimensionSelector.FindAnyWidget("TextSelection"));
		SCR_ButtonImageComponent firstDimensionEntry;
		
		m_mDimensionIDs.Clear();
		m_SelectedDimensionButton = null;
						
		Widget child = dimensionSelectorLine.GetChildren();
		while (child)
		{
			child.RemoveFromHierarchy();
			child = dimensionSelectorLine.GetChildren();
		}
		
		foreach (int i, SCR_MarkerMilitaryDimension dimensionEntry : dimensionsArr)
		{			
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorDimensionEntry, dimensionSelectorLine);
			button.SetName("DimensionEntry" + i.ToString());
			
			SCR_MilitarySymbol milSymbol = new SCR_MilitarySymbol();
			milSymbol.SetIdentity(factionEntry.GetFactionIdentity());
			milSymbol.SetDimension(dimensionEntry.GetDimension());
			
			Widget overlay = button.FindAnyWidget("OverlaySymbol");
			overlay.SetColor(factionEntry.GetColor());
			
			SCR_MilitarySymbolUIComponent symbolComp = SCR_MilitarySymbolUIComponent.Cast(overlay.FindHandler(SCR_MilitarySymbolUIComponent));
			symbolComp.Update(milSymbol);
			
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			buttonComp.m_OnClicked.Insert(OnDimensionEntryClicked);
			buttonComp.m_OnFocus.Insert(OnDimensionEntryFocused);
			
			m_mDimensionIDs.Insert(buttonComp, i);
			
			if (!firstDimensionEntry)
				firstDimensionEntry = buttonComp;
		}
		
		if (m_iWantedDimensionEntry == -1)
		{
			OnDimensionEntryClicked(firstDimensionEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = SCR_MapHelper<SCR_ButtonBaseComponent, int>.GetKeyByValue(m_mDimensionIDs, m_iWantedDimensionEntry);
			if (buttonComp)
				OnDimensionEntryClicked(buttonComp);
			else 
				OnDimensionEntryClicked(firstDimensionEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init color icons
	protected void InitColorIcons(int selectedColorEntry)
	{
		array<ref SCR_MarkerColorEntry> colorsArr = m_PlacedMarkerConfig.GetColorEntries();
		Widget colorSelector = m_MarkerEditRoot.FindAnyWidget(COLOR_SELECTOR);		
		Widget colorSelectorLine = colorSelector.FindAnyWidget("ColorSelectorLine");
		SCR_ButtonImageComponent firstColorEntry;
		
		m_mColorIDs.Clear();
		m_SelectedColorButton = null;
		
		foreach (int i, SCR_MarkerColorEntry colorEntry : colorsArr)
		{			
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorColorEntry, colorSelectorLine);
			button.SetName(COLOR_ENTRY + i.ToString());
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			buttonComp.GetImageWidget().SetColor(colorEntry.GetColor());
			buttonComp.m_OnClicked.Insert(OnColorEntryClicked);
			buttonComp.m_OnFocus.Insert(OnColorEntryFocused);
			
			m_mColorIDs.Insert(buttonComp, i);
			
			if (!firstColorEntry)
				firstColorEntry = buttonComp;
		}
		
		if (selectedColorEntry == -1)
		{
			OnColorEntryClicked(firstColorEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = SCR_MapHelper<SCR_ButtonBaseComponent, int>.GetKeyByValue(m_mColorIDs, selectedColorEntry);
			if (buttonComp)
				OnColorEntryClicked(buttonComp);
			else 
				OnColorEntryClicked(firstColorEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init icons by category
	protected void InitCategoryIcons(SCR_TabViewContent tabContent)
	{
		m_mIconIDs.Clear();
		m_SelectedIconButton = null;
		
		string imageset, imagesetGlow, quad;
		
		m_IconSelector = m_MarkerEditRoot.FindAnyWidget(ICON_SELECTOR);		
		array<ref SCR_MarkerIconEntry> iconsArr = m_PlacedMarkerConfig.GetIconEntries();
				
		Widget child = m_IconSelector.GetChildren();
		while (child)
		{
			child.RemoveFromHierarchy();
			child = m_IconSelector.GetChildren();
		}
		
		Widget iconSelectorLine = GetGame().GetWorkspace().CreateWidgets(SELECTOR_LINE, m_IconSelector);
		m_iIconEntryCount = 0;
		m_iIconLines = 1;
				
		SCR_ButtonImageComponent firstEntry;
		
		foreach (int i, SCR_MarkerIconEntry iconEntry : iconsArr)
		{
			if (iconEntry.m_sCategoryIdentifier != tabContent.m_sTabIdentifier)
				continue;
			
			m_iIconEntryCount++;
			if (m_iIconEntryCount > m_iIconsPerLine * m_iIconLines)
			{
				iconSelectorLine = GetGame().GetWorkspace().CreateWidgets(SELECTOR_LINE, m_IconSelector);
				m_iIconLines++;
			}

			Widget button = GetGame().GetWorkspace().CreateWidgets(m_sSelectorIconEntry, iconSelectorLine);
			button.SetName(ICON_ENTRY + m_iIconEntryCount.ToString());
			SCR_ButtonImageComponent buttonComp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
			m_mIconIDs.Insert(buttonComp, i);
			if (!firstEntry)
				firstEntry = buttonComp;
			
			iconEntry.GetIconResource(imageset, imagesetGlow, quad);
			buttonComp.SetImage(imageset, quad);
			buttonComp.m_OnClicked.Insert(OnIconEntryClicked);
			buttonComp.m_OnFocus.Insert(OnIconEntryFocused);
		}
		
		if (m_iWantedIconEntry == -1)
		{
			OnIconEntryClicked(firstEntry);
		}
		else
		{
			SCR_ButtonBaseComponent buttonComp = SCR_MapHelper<SCR_ButtonBaseComponent, int>.GetKeyByValue(m_mIconIDs, m_iWantedIconEntry);
			if (buttonComp)
				OnIconEntryClicked(buttonComp);
			else 
				OnIconEntryClicked(firstEntry);
		}
	}
			
	//------------------------------------------------------------------------------------------------
	//! Create static markers
	protected void CreateStaticMarkers()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		Faction localFaction;
		int localFactionIndex = -1;
		if (factionManager)
		{
			localFaction = factionManager.SGetLocalPlayerFaction();
			localFactionIndex = factionManager.GetFactionIndex(localFaction);
		}

		array<SCR_MapMarkerBase> markersSimple = m_MarkerMgr.GetStaticMarkers();
		foreach(SCR_MapMarkerBase markerDis: m_MarkerMgr.GetDisabledMarkers())
		{
			markersSimple.Insert(markerDis);
		}

		foreach (SCR_MapMarkerBase marker: markersSimple)
		{
			if (!factionManager || marker.GetMarkerFactionFlags() == 0)
			{
				marker.OnCreateMarker(true);
				continue;
			}

			if (!localFaction || !marker.IsFaction(localFactionIndex))
			{
				if (Replication.IsServer())				// if server, enemy markers have to be kept for sync but are disabled
					marker.SetServerDisabled(true);
				else
					continue;
			}
			
			marker.OnCreateMarker(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create dynamic markers
	protected void CreateDynamicMarkers()
	{
		array<SCR_MapMarkerEntity> markersDynamic = m_MarkerMgr.GetDynamicMarkers();
		foreach (SCR_MapMarkerEntity marker : markersDynamic)
		{
			marker.OnCreateMarker();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Remove callback from radial menu
	protected void RemoveMarkerMenu()
	{
		RemoveOwnedMarker(m_RemovableMarker);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Attemt to remove marker, only works if owned
	protected void RemoveOwnedMarker(SCR_MapMarkerBase marker)
	{		
		if (marker)
			m_MarkerMgr.RemoveStaticMarker(marker);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker edit widget removal
	protected void CleanupMarkerEditWidget()
	{
		if (m_MarkerEditRoot)
			m_MarkerEditRoot.RemoveFromHierarchy();
		
		m_bIsDelayed = false;
				
		m_CursorModule.HandleDialog(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SocialComponent GetSocialComponent()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		
		return SocialComponent.Cast(pc.FindComponent(SocialComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------
	//! SCR_TabViewComponent event
	protected void OnTabChanged(SCR_TabViewComponent tabView, Widget widget, int index)
	{
		SCR_TabViewContent tab = tabView.GetEntryContent(index);
		
		InitCategoryIcons(tab);
		m_SliderComp.SetValue(0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnFactionEntryClicked(SCR_ButtonBaseComponent component)
	{
		if (m_SelectedFactionButton)
		{
			m_SelectedFactionButton.ColorizeBackground();
		}
		
		component.ColorizeBackground();	// this will color the button to hover color for KBM
		m_SelectedFactionButton = component;
		m_iSelectedFactionID = m_mFactionIDs.Get(component);
		
		SCR_MarkerMilitaryFactionEntry entry = m_MilitaryMarkerConfig.GetFactionEntry(m_iSelectedFactionID);
		m_wFactionSelectionText.SetText(entry.GetTranslation());
		m_wMarkerPreviewMilitary.SetColor(entry.GetColor());
		m_MilSymbolPreview.SetIdentity(entry.GetFactionIdentity());
		m_MarkerPreviewMilitaryComp.Update(m_MilSymbolPreview);	
		
		InitDimensionIcons();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnFactionEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnFactionEntryClicked(buttonComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnDimensionEntryClicked(SCR_ButtonBaseComponent component)
	{
		if (m_SelectedDimensionButton)
		{
			m_SelectedDimensionButton.ColorizeBackground();
		}
		
		component.ColorizeBackground();	// this will color the button to hover color for KBM
		m_SelectedDimensionButton = component;
		m_iSelectedDimensionID = m_mDimensionIDs.Get(component);
		m_iWantedDimensionEntry = m_iSelectedDimensionID;
	
		SCR_MarkerMilitaryDimension entry = m_MilitaryMarkerConfig.GetDimensionEntry(m_iSelectedDimensionID);
		m_wDimensionSelectionText.SetText(entry.GetTranslation());	
		m_MilSymbolPreview.SetDimension(entry.GetDimension());
		m_MarkerPreviewMilitaryComp.Update(m_MilSymbolPreview);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnDimensionEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnDimensionEntryClicked(buttonComp);
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnColorEntryClicked(SCR_ButtonBaseComponent component)
	{		
		if (m_SelectedColorButton)
		{
			m_SelectedColorButton.ColorizeBackground();
		}
		
		component.ColorizeBackground();	// this will color the button to hover color for KBM
		m_SelectedColorButton = component;
		m_iSelectedColorID = m_mColorIDs.Get(component);
		
		m_wMarkerPreview.SetColor(m_PlacedMarkerConfig.GetColorEntry(m_iSelectedColorID));
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnColorEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnColorEntryClicked(buttonComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnIconEntryClicked(notnull SCR_ButtonBaseComponent component)
	{		
		if (m_SelectedIconButton)
		{
			m_SelectedIconButton.ColorizeBackground();
		}
		
		component.ColorizeBackground();	// this will color the button to hover color for KBM
		m_SelectedIconButton = component;
		m_iSelectedIconID = m_mIconIDs.Get(component);
		
		ResourceName imageset, imagesetGlow;
		string quad;
		m_PlacedMarkerConfig.GetIconEntry(m_iSelectedIconID, imageset, imagesetGlow, quad);
		
		m_wMarkerPreview.LoadImageFromSet(0, imageset, quad);
		m_wMarkerPreviewGlow.LoadImageFromSet(0, imagesetGlow, quad);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonImageComponent event
	protected void OnIconEntryFocused(Widget rootW)
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		SCR_ButtonBaseComponent buttonComp = SCR_ButtonBaseComponent.Cast(rootW.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComp)
			OnIconEntryClicked(buttonComp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_EditBoxComponent event
	protected void OnEditBoxTextChanged(string text)
	{
		m_wMarkerPreviewText.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_SliderComponent event
	protected void OnSliderChanged(SCR_SliderComponent slider, float value)
	{
		m_wMarkerPreview.SetRotation(value);
		m_wMarkerPreviewGlow.SetRotation(value);
		m_fRotation = value;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ComboBoxComponent event
	protected void OnComboBoxChangedA(SCR_ComboBoxComponent comp, int value)
	{
		m_eMilitaryTypeA = SCR_MarkerMilitaryType.Cast(comp.GetItemData(value));
		if (m_eMilitaryTypeA)
			m_eMilitaryTypeAIcon = m_eMilitaryTypeA.GetType();
		else 
			m_eMilitaryTypeAIcon = 0;
		
		m_MilSymbolPreview.SetIcons(m_eMilitaryTypeAIcon | m_eMilitaryTypeBIcon);
		m_MarkerPreviewMilitaryComp.Update(m_MilSymbolPreview);	
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ComboBoxComponent event
	protected void OnComboBoxChangedB(SCR_ComboBoxComponent comp, int value)
	{
		m_eMilitaryTypeB = SCR_MarkerMilitaryType.Cast(comp.GetItemData(value));
		if (m_eMilitaryTypeB)
			m_eMilitaryTypeBIcon = m_eMilitaryTypeB.GetType();
		else 
			m_eMilitaryTypeBIcon = 0;
		
		m_MilSymbolPreview.SetIcons(m_eMilitaryTypeAIcon | m_eMilitaryTypeBIcon);
		m_MarkerPreviewMilitaryComp.Update(m_MilSymbolPreview);	
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_SpinBoxComponent event
	protected void OnTimestampSpinBoxChanged(SCR_SpinBoxComponent comp, int selectedItem)
	{
		m_bIsTimestampVisible = selectedItem == SPIN_BOX_YES;
	}
		
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonTextComponent event
	protected void OnPlaceMarkerConfirmed(SCR_InputButtonComponent button)
	{
		if (m_EditedMarker)
		{
			RemoveOwnedMarker(m_EditedMarker);
			m_EditedMarker = null;
		}
		
		OnInsertMarker(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonTextComponent event
	protected void OnPlaceMarkerConfirmedPrivate(SCR_InputButtonComponent button)
	{
		OnInsertMarker(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_ButtonTextComponent event
	protected void OnEditCancelled(SCR_InputButtonComponent button)
	{
		if (m_EditedMarker)
		{
			m_EditedMarker.SetVisible(true);
			m_EditedMarker = null;
		}
		
		CleanupMarkerEditWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInsertMarker(bool isLocal)
	{
		float wX, wY;
		m_MapEntity.GetMapCenterWorldPosition(wX, wY);
		
		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		
		if (m_bIsMilitaryMarker)
		{
			marker.SetType(SCR_EMapMarkerType.PLACED_MILITARY);
			marker.SetFlags(m_eMilitaryTypeAIcon | m_eMilitaryTypeBIcon);
			marker.SetMarkerConfigID(m_iSelectedDimensionID * 100 + m_iSelectedFactionID); // combination of faction and dimension id
		}
		else 
		{
			marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
			marker.SetRotation(m_fRotation);
			marker.SetColorEntry(m_iSelectedColorID);
			marker.SetIconEntry(m_iSelectedIconID);
		}
		
		marker.SetCustomText(m_EditBoxComp.GetValue());
		marker.SetWorldPos(wX, wY);
		marker.SetTimestampVisibility(m_bIsTimestampVisible);

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			marker.SetTimestamp(world.GetServerTimestamp());
		
		if (!isLocal)
		{
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
			{
				Faction markerOwnerFaction = SCR_FactionManager.SGetPlayerFaction(GetGame().GetPlayerController().GetPlayerId());
				if (markerOwnerFaction)
					marker.AddMarkerFactionFlags(factionManager.GetFactionIndex(markerOwnerFaction));
			}
		}
		
		m_MarkerMgr.InsertStaticMarker(marker, isLocal);
		m_OnCustomMarkerPlaced.Invoke(wX, wY, isLocal);
		
		CleanupMarkerEditWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapRadialUI event
	protected void OnRadialMenuInit()
	{
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;
		
		SCR_MapRadialUI radialUI = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));

		m_RootCategoryEntry = radialUI.AddRadialCategory(m_sCategoryName);
		m_RootCategoryEntry.SetIcon(m_sIconImageset, m_sCategoryIconName);
		
		array<ref SCR_MapMarkerEntryConfig> entryConfigs = markerConfig.GetMarkerEntryConfigs();
		
		foreach (SCR_MapMarkerEntryConfig entry : entryConfigs)		// menu entries
		{			
			if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_MILITARY)
			{				
				SCR_MapMarkerEntryMilitary entryMil = SCR_MapMarkerEntryMilitary.Cast(entry);
				
				SCR_MapMarkerMenuEntry menuEntry = new SCR_MapMarkerMenuEntry();
				menuEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_MILITARY);
				menuEntry.SetName(entryMil.GetMenuDescription());
				menuEntry.GetOnPerform().Insert(OnEntryPerformed);
				menuEntry.SetIcon(entryMil.GetMenuImageset(), entryMil.GetMenuIcon());
				
				radialUI.InsertCustomRadialEntry(menuEntry, m_RootCategoryEntry);
			}
			else if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_CUSTOM)
			{
				SCR_MapMarkerEntryPlaced entryPlaced = SCR_MapMarkerEntryPlaced.Cast(entry);
				
				SCR_MapMarkerMenuEntry menuEntry = new SCR_MapMarkerMenuEntry();
				menuEntry.SetMarkerType(SCR_EMapMarkerType.PLACED_CUSTOM);
				menuEntry.SetName(entryPlaced.GetMenuDescription());
				menuEntry.GetOnPerform().Insert(OnEntryPerformed);
				menuEntry.SetIcon(entryPlaced.GetMenuImageset(), entryPlaced.GetMenuIcon());
	
				radialUI.InsertCustomRadialEntry(menuEntry, m_RootCategoryEntry);
			}
			else if (entry.GetMarkerType() == SCR_EMapMarkerType.PLACED_RECON)
			{
				SCR_MapMarkerEntryRecon entryRecon = SCR_MapMarkerEntryRecon.Cast(entry);
				m_ReconCategoryEntry = radialUI.AddRadialCategory(entryRecon.GetMenuDescription(), m_RootCategoryEntry);
				m_ReconCategoryEntry.SetName(entryRecon.GetMenuDescription());
				m_ReconCategoryEntry.SetIcon(entryRecon.GetMenuImageset(), entryRecon.GetMenuIcon());

				entryRecon.CreateEntries(radialUI, m_ReconCategoryEntry);

				// radial category is deleted if nothing is added to the entry
				if (m_ReconCategoryEntry.GetEntries().IsEmpty())
				{
					m_RootCategoryEntry.RemoveEntry(m_ReconCategoryEntry);
				}
			}
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_RadialMenuController event
	protected void OnRadialMenuOpen(SCR_RadialMenuController controller)
	{		
		SCR_MapRadialUI radialUI = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));
		if (m_MarkerRemoveEntry)
		{
			radialUI.RemoveRadialEntry(m_MarkerRemoveEntry);
			m_RemovableMarker = null;
		}
		
		// delete marker button
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (marker)
			{
				if (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM && marker.GetType() != SCR_EMapMarkerType.PLACED_MILITARY)
					continue;
				
				m_MarkerRemoveEntry = radialUI.AddRadialEntry("#AR-MapMarker_DeleteHint");
				m_MarkerRemoveEntry.SetIcon(m_sIconImageset, m_sDeleteIconName);
				m_MarkerRemoveEntry.GetOnPerform().Insert(RemoveMarkerMenu);
				
				m_RemovableMarker = marker;
				
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapMarkerMenuEntry event
	protected void OnEntryPerformed(SCR_SelectionMenuEntry entry)
	{
		SCR_MapMarkerMenuEntry markerEntry = SCR_MapMarkerMenuEntry.Cast(entry);
		
		if (markerEntry.GetMarkerType() == SCR_EMapMarkerType.PLACED_CUSTOM)
		{
			if (!m_PlacedMarkerConfig)
			{
				SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
				if (!markerConfig)
					return;
				
				m_PlacedMarkerConfig = SCR_MapMarkerEntryPlaced.Cast(markerConfig.GetMarkerEntryConfigByType(markerEntry.GetMarkerType()));
				if (!m_PlacedMarkerConfig)
					return;
			}
			
			CreateMarkerEditDialog();
		}		
		else if (markerEntry.GetMarkerType() == SCR_EMapMarkerType.PLACED_MILITARY)			
		{			
			if (!m_MilitaryMarkerConfig)
			{
				SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
				if (!markerConfig)
					return;
				
				m_MilitaryMarkerConfig = SCR_MapMarkerEntryMilitary.Cast(markerConfig.GetMarkerEntryConfigByType(markerEntry.GetMarkerType()));
				if (!m_MilitaryMarkerConfig)
					return;
			}
			
			CreateMilitaryMarkerEditDialog();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Quick open input for marker category
	protected void OnInputQuickMarkerMenu(float value, EActionTrigger reason)
	{
		SCR_MapRadialUI mapRadial = SCR_MapRadialUI.GetInstance();
		if (!mapRadial)
			return;
		
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		if (focusedWidget)
			return;
		
		mapRadial.GetRadialController().OnInputOpen();

		// Callqueue fixes selecting in the radial
		GetGame().GetCallqueue().Call(DelayedQuickMarkerMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedQuickMarkerMenu()
	{
		SCR_MapRadialUI mapRadial = SCR_MapRadialUI.GetInstance();
		if (!mapRadial)
			return;

		if (m_ReconCategoryEntry && m_ReconCategoryEntry.IsEnabled())
			mapRadial.GetRadialController().GetRadialMenu().PerformEntry(m_ReconCategoryEntry);
		else
			mapRadial.GetRadialController().GetRadialMenu().PerformEntry(m_RootCategoryEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker delete quickbind
	protected void OnInputMarkerDelete(float value, EActionTrigger reason)
	{
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (marker)
			{
				if (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM && marker.GetType() != SCR_EMapMarkerType.PLACED_MILITARY)
					continue;
				
				if (!marker.CanBeRemovedByOwner())
					continue;
				
				RemoveOwnedMarker(marker);
			}		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Marker select
	protected void OnInputMapSelect(float value, EActionTrigger reason)
	{
		if ((m_CursorModule.GetCursorState() & SCR_MapCursorModule.STATE_POPUP_RESTRICTED) != 0)
			return;

		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		
		SCR_MapMarkerWidgetComponent markerComp;
		foreach ( Widget widget : widgets )
		{
			markerComp = SCR_MapMarkerWidgetComponent.Cast(widget.FindHandler(SCR_MapMarkerWidgetComponent));	
			if (!markerComp)
				continue;
						
			SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
			if (!marker)
				continue;
		
			m_EditedMarker = marker;
			SCR_EMapMarkerType type = marker.GetType();
			
			if (IsOwnedMarker(marker) && (type == SCR_EMapMarkerType.PLACED_CUSTOM || type == SCR_EMapMarkerType.PLACED_MILITARY))
			{	
				int wPos[2];
				float screenX, screenY;
				marker.GetWorldPos(wPos);
				m_MapEntity.WorldToScreen(wPos[0], wPos[1], screenX, screenY);
				m_MapEntity.PanSmooth(screenX, screenY, 0.1);
								
				if (type == SCR_EMapMarkerType.PLACED_CUSTOM )
				{
					if (!m_PlacedMarkerConfig)
						return;
					
					CreateMarkerEditDialog(true, m_PlacedMarkerConfig.GetIconCategoryID(marker.GetIconEntry()), marker.GetIconEntry(), marker.GetColorEntry());
					
					OnEditBoxTextChanged(marker.GetCustomText());
					m_SliderComp.SetValue(marker.GetRotation());
					m_EditBoxComp.SetValue(marker.GetCustomText());
				} 
				else 
				{
					if (!m_MilitaryMarkerConfig)
						return;
					
					CreateMilitaryMarkerEditDialog(true, marker.GetMarkerConfigID() % SCR_MapMarkerEntryMilitary.FACTION_DETERMINATOR, marker.GetMarkerConfigID() * SCR_MapMarkerEntryMilitary.DIMENSION_DETERMINATOR);
					OnEditBoxTextChanged(marker.GetCustomText());
					m_EditBoxComp.SetValue(marker.GetCustomText());
					
					int markerFlags = marker.GetFlags();
					bool secondType;
					
					array<ref SCR_MarkerMilitaryType> milTypes = m_MilitaryMarkerConfig.GetMilitaryTypes();
					foreach (int i, SCR_MarkerMilitaryType milType : milTypes)
					{
						if (markerFlags & milType.GetType())
						{
							if (secondType)
							{
								m_ComboBoxComp2.SetCurrentItem(i+1);	// +1 is here and below since we add a NONE entry to the combo boxes during creation
								OnComboBoxChangedB(m_ComboBoxComp2, i+1);
								break;
							}
							else 
							{
								m_ComboBoxComp1.SetCurrentItem(i+1);
								OnComboBoxChangedA(m_ComboBoxComp1, i+1);					
								if (markerFlags == milType.GetType())	// return if singular flag
									break;
								
								secondType = true;
							}
						}
					}

					if (m_TimestampSpinBox)
					{
						bool showTimestamp = marker.IsTimestampVisible();
						m_TimestampSpinBox.SetCurrentItem(showTimestamp, false, false);
						OnTimestampSpinBoxChanged(m_TimestampSpinBox, showTimestamp);
					}
				}
				
				marker.SetVisible(false);												
				
				break;
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Menu select/confirm bind
	protected void OnInputMenuConfirm(float value, EActionTrigger reason)
	{
		if (!m_bIsDelayed)
			return;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused || focused.GetUserID() == USERID_EDITBOX || focused.GetUserID() == USERID_EDITBOX_MIL)
			return;
		
		if (m_ComboBoxComp1 && (m_ComboBoxComp1.IsOpened() || m_ComboBoxComp1.GetRootWidget().FindAnyWidget("ComboButton") == focused))
			return;
		
		if (m_ComboBoxComp2 && (m_ComboBoxComp2.IsOpened() || m_ComboBoxComp2.GetRootWidget().FindAnyWidget("ComboButton") == focused))
			return;
		
		if (m_MarkerEditRoot)
			OnPlaceMarkerConfirmed(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu alternative confirm bind
	protected void OnInputMenuConfirmAlter(float value, EActionTrigger reason)
	{
		if (m_MarkerEditRoot)
			OnPlaceMarkerConfirmedPrivate(null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Menu back/escape bind
	protected void OnInputMenuBack(float value, EActionTrigger reason)
	{
		if (m_MarkerEditRoot)
			OnEditCancelled(null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuDown(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (name.Contains(ICON_ENTRY))
			FocusWidget(m_SelectedColorButton.GetRootWidget());
		else if (name.Contains("FactionEntry"))
			FocusWidget(m_SelectedDimensionButton.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuUp(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return;
		
		name = focused.GetName();
		
		if (name.Contains(COLOR_ENTRY))
			FocusWidget(m_SelectedIconButton.GetRootWidget());
		else if (focused.GetUserID() == USERID_EDITBOX)
			FocusWidget(m_SelectedColorButton.GetRootWidget());
		else if (focused.GetUserID() == USERID_EDITBOX_MIL)
			FocusWidget(m_ComboBoxComp2.GetRootWidget());
		else if (name.Contains("DimensionEntry")) 
			FocusWidget(m_SelectedFactionButton.GetRootWidget());
		else if (name.Contains("ComboButton"))
		{
			while (focused.GetParent() != null)
			{
				focused = focused.GetParent();
				if (focused.GetName().Contains("ComboBox1"))
				{
					FocusWidget(m_SelectedDimensionButton.GetRootWidget());
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuLeft(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (!name.Contains(ICON_ENTRY))
			return;
		
		int pos = name.ToInt(offset: 9);
		int target = pos - 1;
		
		if (target < 1)
			target = m_iIconEntryCount;
		
		Widget w = m_IconSelector.FindAnyWidget(ICON_ENTRY + target);
		if (w)
			FocusWidget(w);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputMenuRight(float value, EActionTrigger reason)
	{
		string name;
		
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (focused)
			name = focused.GetName();
		
		if (!name.Contains(ICON_ENTRY))
			return;
		
		int pos = name.ToInt(offset: 9);
		int target = pos + 1;
		
		if (target > m_iIconEntryCount)
			target = 1;
		
		Widget w = m_IconSelector.FindAnyWidget(ICON_ENTRY + target);
		if (w)
			FocusWidget(w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
	protected void OnDragWidget(Widget widget)
	{
		SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
		if (marker)
		{
			if (!IsOwnedMarker(marker) || (marker.GetType() != SCR_EMapMarkerType.PLACED_CUSTOM && marker.GetType() != SCR_EMapMarkerType.PLACED_MILITARY) )
				return;
			
			marker.SetDragged(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
	protected void OnDragEnd(Widget widget, bool wasDragged)
	{		
		SCR_MapMarkerBase marker = m_MarkerMgr.GetMarkerByWidget(widget);
		if (marker)
		{
			SCR_EMapMarkerType type = marker.GetType();
			
			if (!IsOwnedMarker(marker) || (type != SCR_EMapMarkerType.PLACED_CUSTOM && type != SCR_EMapMarkerType.PLACED_MILITARY))
				return;
			
			marker.SetDragged(false);
			
			if (!wasDragged)
				return;
			
			vector pos = FrameSlot.GetPos(widget);

			float wX, wY;
			m_MapEntity.ScreenToWorld(GetGame().GetWorkspace().DPIScale(pos[0]), GetGame().GetWorkspace().DPIScale(pos[1]), wX, wY);
			
			marker.SetWorldPos(wX, wY); // set the old marker pos here as well because of the delay it takes between client and server to delete it
			
			SCR_MapMarkerBase markerNew = new SCR_MapMarkerBase();
			markerNew.SetType(marker.GetType());
			markerNew.SetWorldPos(wX, wY);
			markerNew.SetCustomText(marker.GetCustomText());
			markerNew.SetMarkerFactionFlags(marker.GetMarkerFactionFlags());
			markerNew.SetTimestampVisibility(marker.IsTimestampVisible());

			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				markerNew.SetTimestamp(world.GetServerTimestamp());
			
			if (type == SCR_EMapMarkerType.PLACED_CUSTOM)
			{
				markerNew.SetColorEntry(marker.GetColorEntry());
				markerNew.SetIconEntry(marker.GetIconEntry());
				markerNew.SetRotation(marker.GetRotation());
			}
			else 
			{
				markerNew.SetFlags(marker.GetFlags());
				markerNew.SetMarkerConfigID(marker.GetMarkerConfigID()); // combination of faction and dimension id
			}
			
			int markerID = marker.GetMarkerID();
			RemoveOwnedMarker(marker);
			
			bool isLocal = markerID == -1;
			m_MarkerMgr.InsertStaticMarker(markerNew, isLocal);
		
			m_OnCustomMarkerPlaced.Invoke(wX, wY, isLocal);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		CreateStaticMarkers();
		CreateDynamicMarkers();
		
		m_MarkerMgr.EnableUpdate(true);		// run frame update manager side
		
		GetGame().GetInputManager().AddActionListener("MapQuickMarkerMenu", EActionTrigger.DOWN, OnInputQuickMarkerMenu);
		GetGame().GetInputManager().AddActionListener("MapMarkerDelete", EActionTrigger.DOWN, OnInputMarkerDelete);
		GetGame().GetInputManager().AddActionListener("MapSelect", EActionTrigger.DOWN, OnInputMapSelect);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnInputMenuConfirm);
		GetGame().GetInputManager().AddActionListener("MenuRefresh", EActionTrigger.DOWN, OnInputMenuConfirmAlter);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnInputMenuBack);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnInputMenuDown);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnInputMenuUp);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnInputMenuRight);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnInputMenuLeft);
		
		if ( SCR_MapToolInteractionUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolInteractionUI)) )	// if dragging available, add callback
		{
			SCR_MapToolInteractionUI.GetOnDragWidgetInvoker().Insert(OnDragWidget);
			SCR_MapToolInteractionUI.GetOnDragEndInvoker().Insert(OnDragEnd);
		}
		
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (markerConfig)
		{
			foreach (SCR_MapMarkerEntryConfig entryType : markerConfig.GetMarkerEntryConfigs())
			{
				entryType.OnMapOpen(m_MapEntity, this);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{				
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;
		
		foreach (SCR_MapMarkerEntryConfig entryType : markerConfig.GetMarkerEntryConfigs())
		{
			entryType.OnMapClose(m_MapEntity, this);
		}
		
		CleanupMarkerEditWidget();
		GetGame().GetInputManager().RemoveActionListener("MapQuickMarkerMenu", EActionTrigger.DOWN, OnInputQuickMarkerMenu);
		GetGame().GetInputManager().RemoveActionListener("MapMarkerDelete", EActionTrigger.DOWN, OnInputMarkerDelete);
		GetGame().GetInputManager().RemoveActionListener("MapSelect", EActionTrigger.DOWN, OnInputMapSelect);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnInputMenuConfirm);
		GetGame().GetInputManager().RemoveActionListener("MenuRefresh", EActionTrigger.DOWN, OnInputMenuConfirmAlter);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, OnInputMenuBack);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnInputMenuDown);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnInputMenuUp);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnInputMenuRight);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnInputMenuLeft);
		
		m_MarkerMgr.EnableUpdate(false);
		super.OnMapClose(config);
	}

	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));
		if (radialMenu)
		{
			radialMenu.GetOnMenuInitInvoker().Insert(OnRadialMenuInit);
			radialMenu.GetRadialController().GetOnInputOpen().Insert(OnRadialMenuOpen);
		}
		
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		
		m_MarkerMgr = SCR_MapMarkerManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_MapMarkerManagerComponent));
		SCR_MapMarkerConfig markerConfig = m_MarkerMgr.GetMarkerConfig();
		if (!markerConfig)
			return;
		
		m_PlacedMarkerConfig = SCR_MapMarkerEntryPlaced.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_CUSTOM));
		m_MilitaryMarkerConfig = SCR_MapMarkerEntryMilitary.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_MILITARY));
		
		array<ref SCR_MapMarkerEntryConfig> entryConfigs = markerConfig.GetMarkerEntryConfigs();
		
		foreach (SCR_MapMarkerEntryConfig entryType : entryConfigs)
		{
			entryType.OnMapInit(m_MapEntity, this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_MarkerEditRoot)
		{
			if (!m_bIsDelayed)
			{
				m_bIsDelayed = true;
				return;
			}
			
			GetGame().GetInputManager().ActivateContext("MapMarkerEditContext");
		}
	}
}
