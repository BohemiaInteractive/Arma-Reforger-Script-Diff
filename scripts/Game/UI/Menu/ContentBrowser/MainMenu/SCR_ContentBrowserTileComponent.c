//#define WORKSHOP_DEBUG

//! Component for a tile in the content browser
//! You must call SetWorkshopItem() after tile creation to activate it
class SCR_ContentBrowserTileComponent : SCR_ScriptedWidgetComponent
{
	protected static int s_iTileWidth = 285;
	protected static int s_iTileHeight = 224;
	
	// Event Handlers
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocus = new ScriptInvokerScriptedWidgetComponent();
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFocusLost = new ScriptInvokerScriptedWidgetComponent();
	protected ref ScriptInvokerScriptedWidgetComponent m_OnChange = new ScriptInvokerScriptedWidgetComponent();

	protected ref SCR_WorkshopTileWidgets m_Widgets = new SCR_WorkshopTileWidgets();

	protected bool m_bFocused;
	protected bool m_bUpdatingContinuously;

	protected ref SCR_WorkshopItem m_Item;

	protected SCR_WorkshopItemBackendImageComponent m_BackendImageComponent;
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected SCR_ERevisionAvailability m_eRevisionAvailability;

	protected SCR_EWorkshopTileErrorState m_eErrorState;
	protected SCR_EAddonPrimaryActionState m_ePrimaryActionState;

	protected SCR_ScriptedWidgetTooltip m_Tooltip;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_Widgets.Init(m_wRoot);

		// Tile Size
		s_iTileWidth = m_Widgets.m_wSizeLayoutMain.GetWidthOverride();
		s_iTileHeight = m_Widgets.m_wSizeLayoutMain.GetHeightOverride();
		
		m_Widgets.m_EnableAddonButtonComponent.m_OnToggled.Insert(OnEnableButtonToggled);
		m_Widgets.m_FavouriteButtonComponent0.m_OnToggled.Insert(OnFavouriteButtonToggled);

		Widget backendImage = m_wRoot.FindAnyWidget("m_BackendImage");
		if (backendImage)
			m_BackendImageComponent = SCR_WorkshopItemBackendImageComponent.Cast(backendImage.FindHandler(SCR_WorkshopItemBackendImageComponent));

		UpdateWarningOverlay();
		m_Widgets.m_WarningOverlayComponent.GetOnWarningIconButtonClicked().Insert(OnWarningButtonClicked);

		UpdateHeader();
		m_Widgets.m_MainActionButtonComponent0.m_OnClicked.Insert(OnMainActionButtonClicked);
		m_Widgets.m_RepairActionButtonComponent0.m_OnClicked.Insert(OnRepairActionButtonClicked);

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Insert(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Insert(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Insert(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Insert(OnAnyDownloadCompleted);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Remove(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Remove(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Remove(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Remove(OnAnyDownloadCompleted);
		}

		StopContinuousUpdate();

		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(OnWorkshopItemChange);
			m_Item.m_OnGetAsset.Remove(OnWorkshopItemChange);
			m_Item.m_OnDependenciesLoaded.Remove(OnWorkshopItemChange);
		}
		
		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_OnFocus.Invoke(this);
		m_bFocused = true;

		UpdateFavouriteButton();
		UpdateNamesScrolling();
		UpdateHeader();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);

		return super.OnFocus(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_OnFocusLost.Invoke(this);
		m_bFocused = false;

		UpdateFavouriteButton();
		UpdateNamesScrolling();
		UpdateHeader();

		// Tooltips
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);

		return super.OnFocusLost(w, x, y);
	}

	// ---- Protected ----
	//------------------------------------------------------------------------------------------------
	protected void UpdateStateFlags()
	{
		if (!m_Item)
			return;
		
		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();
		WorkshopItem item = m_Item.GetWorkshopItem();
		Revision itemRevision = item.GetActiveRevision();
		EWorkshopItemProblem itemProblem = m_Item.GetHighestPriorityProblem();
		
		// --- Revision Availability ---
		m_eRevisionAvailability = SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		if (item)
			m_eRevisionAvailability = addonManager.ItemAvailability(item);

		// --- Primary Action State ---
		m_ePrimaryActionState = SCR_WorkshopUiCommon.GetPrimaryActionState(m_Item);
		
		// --- Error State ---
		if (m_Item.GetRestricted())
			m_eErrorState = SCR_EWorkshopTileErrorState.RESTRICTED;
		else if (m_eRevisionAvailability != SCR_ERevisionAvailability.ERA_AVAILABLE && m_eRevisionAvailability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY)
			m_eErrorState = SCR_EWorkshopTileErrorState.REVISION_AVAILABILITY_ISSUE;
		else if (itemProblem == EWorkshopItemProblem.DEPENDENCY_MISSING || itemProblem == EWorkshopItemProblem.DEPENDENCY_DISABLED)
			m_eErrorState = SCR_EWorkshopTileErrorState.DEPENDENCIES_ISSUE;
		else if (item && (item.IsUnlisted() && (item.IsAuthor() || item.IsContributor())))
			m_eErrorState = SCR_EWorkshopTileErrorState.UNLISTED;
		else if (itemRevision && itemRevision.IsCorrupted())
			m_eErrorState = SCR_EWorkshopTileErrorState.DAMAGED;
		else
			m_eErrorState = SCR_EWorkshopTileErrorState.NONE;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNames()
	{
		m_Widgets.m_wAddonName.SetText(m_Item.GetName());
		m_Widgets.m_wAddonAuthor.SetText(WidgetManager.Translate("#AR-Workshop_AddonAuthorPrefix", m_Item.GetAuthorName()));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateNamesScrolling()
	{
		if (m_bFocused)
		{
			if (!m_Widgets.m_FrameNameComponent.GetContentFitX())
				m_Widgets.m_FrameNameComponent.AnimationStart();

			if (!m_Widgets.m_FrameAuthorComponent.GetContentFitX())
				m_Widgets.m_FrameAuthorComponent.AnimationStart();
		}
		else
		{
			m_Widgets.m_FrameNameComponent.AnimationStop();
			m_Widgets.m_FrameNameComponent.ResetPosition();

			m_Widgets.m_FrameAuthorComponent.AnimationStop();
			m_Widgets.m_FrameAuthorComponent.ResetPosition();
		}

		// TODO: additional authors
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateRating()
	{
		m_Widgets.m_wRating.SetVisible(m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED);
		if (!m_Widgets.m_wRating.IsVisible())
			return;

		m_Widgets.m_RatingComponent1.SetTitle(UIConstants.FormatUnitPercentage(SCR_WorkshopUiCommon.GetRatingPercentage(m_Item.GetAverageRating())));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateImage()
	{
		if (!m_BackendImageComponent)
			return;

		m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDependencyCountWidgets()
	{
		if (!m_Item)
			return;

		int nDependencies = m_Item.GetLatestDependencies().Count();
		int nDependent = SCR_WorkshopUiCommon.GetDownloadedDependentAddons(m_Item).Count();

		// Displays
		m_Widgets.m_wDependent.SetVisible(nDependent > 0);
		m_Widgets.m_wDependencies.SetVisible(nDependencies > 0);
		
		m_Widgets.m_DependentComponent1.SetTitle(Math.ClampInt(nDependent, 0, SCR_WorkshopUiCommon.MAX_DEPENDENCIES_SHOWN).ToString());
		m_Widgets.m_DependenciesComponent1.SetTitle(Math.ClampInt(nDependencies, 0, SCR_WorkshopUiCommon.MAX_DEPENDENCIES_SHOWN).ToString());
		
		UpdateDependenciesTooltip();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressBar()
	{
		float progress;

		if (m_Item && m_ePrimaryActionState == SCR_EAddonPrimaryActionState.DOWNLOADING)
			progress = SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item);

		m_Widgets.m_wDownloadProgressBar.SetCurrent(progress);
		m_Widgets.m_wDownloadProgressBarOverlay.SetVisible(progress > 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateFavouriteButton()
	{
		if (!m_Item)
			return;
		
		m_Widgets.m_FavouriteButtonComponent0.SetToggled(m_Item.GetFavourite(), false);
		m_Widgets.m_FavouriteButtonComponent0.SetVisible((m_bFocused || m_Item.GetFavourite()) && m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowBackendEnv()
	{
		WorkshopItem item = m_Item.GetWorkshopItem();
		if (!item)
		{
			m_Widgets.m_wBackendSource.SetVisible(false);
			return;
		}
		
		string gameEnv = GetGame().GetBackendApi().GetBackendEnv();
		string modEnv = m_Item.GetWorkshopItem().GetBackendEnv();

		bool display = (modEnv != "local") && (modEnv != "ask") && (modEnv != "invalid");
		bool envMatch = gameEnv == modEnv;

		m_Widgets.m_wBackendSource.SetVisible(display && !envMatch);
		if (!display)
			return;

		m_Widgets.m_BackendSourceComponent1.SetTitle(modEnv);

		if (envMatch)
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.CONFIRM);
		else
			m_Widgets.m_BackendSourceComponent0.SetIconColor(UIColors.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWarningOverlay()
	{
		// No error, hide
		if (!m_Item || m_eErrorState == SCR_EWorkshopTileErrorState.NONE)
		{
			m_Widgets.m_WarningOverlayComponent.SetWarningVisible(false);
			m_Widgets.m_WarningOverlayComponent.SetBlurUnderneath(false);
			
			if (m_BackendImageComponent)
				m_BackendImageComponent.SetImageSaturation(UIConstants.ENABLED_WIDGET_SATURATION);
			
			return;
		}

		// Errors, show and desaturate the image
		m_Widgets.m_WarningOverlayComponent.SetWarningVisible(true);
		m_Widgets.m_WarningOverlayComponent.SetBlurUnderneath(m_eErrorState == SCR_EWorkshopTileErrorState.RESTRICTED);
		
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetImageSaturation(UIConstants.DISABLED_WIDGET_SATURATION);

		// Update Message
		switch (m_eErrorState)
		{
			case SCR_EWorkshopTileErrorState.RESTRICTED:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.GetRestrictedAddonStateText(m_Item), SCR_WorkshopUiCommon.ICON_REPORTED);
				m_Widgets.m_WarningOverlayComponent.ResetWarningColor();
				break;
			}

			case SCR_EWorkshopTileErrorState.DEPENDENCIES_ISSUE:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.MESSAGE_DEPENDENCIES_MISSING, SCR_WorkshopUiCommon.ICON_DEPENDENCIES);
				m_Widgets.m_WarningOverlayComponent.ResetWarningColor();
				break;
			}

			case SCR_EWorkshopTileErrorState.REVISION_AVAILABILITY_ISSUE:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning(
					SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(m_eRevisionAvailability),
					SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(m_eRevisionAvailability)
				);
				m_Widgets.m_WarningOverlayComponent.ResetWarningColor();
				break;
			}
			
			case SCR_EWorkshopTileErrorState.UNLISTED:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning("#AR-Player_Groups_Private", "flag-2", "{E23427CAC80DA8B7}UI/Textures/Icons/icons_mapMarkersUI.imageset");
				
				Color color = Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt());
				m_Widgets.m_WarningOverlayComponent.SetWarningColor(color, color);
				break;
			}
			
			case SCR_EWorkshopTileErrorState.DAMAGED:
			{
				m_Widgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.MESSAGE_CORRUPTED, SCR_WorkshopUiCommon.ICON_REPAIR);
				m_Widgets.m_WarningOverlayComponent.ResetWarningColor();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHeader()
	{
		if (!m_Item)
			return;
		
		m_Widgets.m_wRepairActionButton.SetVisible(m_bFocused && m_eErrorState == SCR_EWorkshopTileErrorState.DAMAGED);

		m_Widgets.m_wMainActionButton.SetVisible(
			(m_bFocused || m_ePrimaryActionState != SCR_EAddonPrimaryActionState.DOWNLOAD) &&
			m_ePrimaryActionState != SCR_EAddonPrimaryActionState.DOWNLOADED &&
			m_eErrorState != SCR_EWorkshopTileErrorState.RESTRICTED
		);

		if (!m_Widgets.m_wMainActionButton.IsVisible())
			return;

		UpdateHeaderTooltip();
		
		SCR_ButtonEffectColor iconColorEffect = SCR_ButtonEffectColor.Cast(m_Widgets.m_MainActionButtonComponent0.FindEffect(SCR_ListEntryHelper.EFFECT_ICON_COLOR));
		if (!iconColorEffect)
			return;
		
		SCR_ButtonEffectColor messageColorEffect = SCR_ButtonEffectColor.Cast(m_Widgets.m_MainActionButtonComponent0.FindEffect(SCR_ListEntryHelper.EFFECT_MESSAGE_COLOR));
		if (!messageColorEffect)
			return;

		string icon;
		string message;
		Color iconColor;
		Color messageColor;
		
		SCR_WorkshopUiCommon.GetPrimaryActionLook(m_ePrimaryActionState, m_eRevisionAvailability, m_Item, icon, iconColor, message, messageColor);
	
		m_Widgets.m_wMainActionButton.SetVisible(m_Item.GetSizeBytes() > 0);
		
		if (iconColor)	
			iconColorEffect.m_cDefault = iconColor;
		
		if (messageColor)
			messageColorEffect.m_cDefault = messageColor;
		
		m_Widgets.m_MainActionButtonComponent0.InvokeAllEnabledEffects(true);
		
		m_Widgets.m_MainActionButtonComponent1.SetImage(icon);
		
		m_Widgets.m_wMainActionText.SetText(message);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMANWVisuals()
	{
		if (!m_Item.GetWorkshopItem())
		{
			m_Widgets.m_wMANWLogo.SetVisible(false);
			return;
		}
		
		m_Widgets.m_ManwAwardComponent.UpdateAwards(m_Item);
		
		array<WorkshopTag> items = {};
		m_Item.GetWorkshopItem().GetTags(items);
		foreach(WorkshopTag item: items)
		{
			if (item.Name() == "MANW_2025")
			{
				m_Widgets.m_wMANWLogo.SetVisible(true);
				return;
			}
		}
		
		m_Widgets.m_wMANWLogo.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		// Tile visibility
		if (!m_Item)
		{
			m_wRoot.SetVisible(false);
			return;
		}

		m_wRoot.SetVisible(true);

		// Check states and update dynamic elements
		HandleDownloadChanges();

		// Update static elements
		UpdateNames();
		UpdateNamesScrolling();
		UpdateRating();
		UpdateImage();
		ShowBackendEnv();
		SCR_WorkshopUiCommon.UpdateEnableAddonToggleButton(m_Widgets.m_EnableAddonButtonComponent, m_Item, m_ePrimaryActionState);
		UpdateFavouriteButton();
		UpdateDependencyCountWidgets();
		
		UpdateEnableButtonTooltip();
		UpdateMANWVisuals();
		
		// Show type 
		bool isSave = m_Item.IsWorldSave();
		
		m_Widgets.m_wItemTypeImage.SetVisible(isSave);
		m_Widgets.m_wItemTypeShadow.SetVisible(isSave);
		
		if (isSave)
		{
			const string imageType = "save-published"; // TODO: check for good const usage
		
			m_Widgets.m_wItemTypeImage.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, imageType);
			m_Widgets.m_wItemTypeShadow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, imageType);
		}
		
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleDownloadChanges()
	{
		UpdateStateFlags();

		UpdateDownloadProgressBar();
		UpdateWarningOverlay();
		UpdateHeader();

		if (m_Item && m_ePrimaryActionState == SCR_EAddonPrimaryActionState.DOWNLOADING)
			StartContinuousUpdate();
		else
			StopContinuousUpdate();
		
		m_OnChange.Invoke(this);
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Each tile does it's own callqueue, so there's going to be many! Desyncronized with other UIs that might show download progress, like the top right button
		
	//------------------------------------------------------------------------------------------------
	protected void StartContinuousUpdate()
	{
		if (m_bUpdatingContinuously || !m_Item)
			return;

		GetGame().GetCallqueue().CallLater(ContinuousUpdate, SCR_WorkshopUiCommon.CONTINUOUS_UPDATE_DELAY, true);

		m_bUpdatingContinuously = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StopContinuousUpdate()
	{
		if (m_bUpdatingContinuously)
			GetGame().GetCallqueue().Remove(ContinuousUpdate);

		m_bUpdatingContinuously = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ContinuousUpdate()
	{
		if (!m_Item || m_ePrimaryActionState != SCR_EAddonPrimaryActionState.DOWNLOADING)
		{
			StopContinuousUpdate();
			return;
		}

		if (!m_Item.GetWorkshopItem().GetDownloadingRevision())
			return;
		
		UpdateDownloadProgressBar();
		UpdateHeader();
	}

//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void OnEnableButtonToggled(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.OnEnableAddonToggleButton(m_Item, comp);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFavouriteButtonToggled(SCR_ModularButtonComponent comp, bool toggled)
	{
		if (!m_Item)
			return;

		m_Item.SetFavourite(toggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWarningButtonClicked(ScriptInvokerScriptedWidgetComponent comp)
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMainActionButtonClicked(SCR_ModularButtonComponent comp)
	{
		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRepairActionButtonClicked(SCR_ModularButtonComponent comp)
	{
		if (!m_Item || m_Item.GetRestricted())
			return;

		SCR_ValidateRepair_Dialog dialogValidator = SCR_CommonDialogs.CreateValidateRepairDialog();
		dialogValidator.LoadAddon(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopItemChange()
	{
		UpdateAllWidgets();
	}

	// --- Any downloads ---
	// We need to update the progress bar and main action display if we start downloading this item or any of it's dependencies
	//------------------------------------------------------------------------------------------------
	protected void OnAnyNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		HandleDownloadChanges();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadError(SCR_WorkshopItemActionDownload action, int reason)
	{
		HandleDownloadChanges();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnyDownloadCompleted(SCR_WorkshopItemActionDownload action)
	{
		HandleDownloadChanges();
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Overreliance on hardcoded strings and static methods in tooltips
	
	// ---- Tooltips ----
	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		m_Tooltip = tooltip;
		
		UpdateDependenciesTooltip();
		UpdateHeaderTooltip();
		UpdateEnableButtonTooltip();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDependenciesTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContentRoot())
			return;
		
		SCR_ContentBrowser_AddonsTooltipComponent comp = SCR_ContentBrowser_AddonsTooltipComponent.FindComponent(m_Tooltip.GetContentRoot());
		if (!comp)
			return;
		
		if (m_Tooltip.IsValid("Dependent", m_Widgets.m_wDependent))
			comp.Init(SCR_WorkshopUiCommon.GetDownloadedDependentAddons(m_Item));
		else if (m_Tooltip.IsValid("Dependencies", m_Widgets.m_wDependencies))
			comp.Init(m_Item.GetLatestDependencies());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHeaderTooltip()
	{
		if (!m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("MainAction", m_Widgets.m_wMainActionButton))
			return;
		
		m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.GetPrimaryActionTooltipMessage(m_ePrimaryActionState, m_Item));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateEnableButtonTooltip()
	{
		if (!m_Item || !m_Tooltip || !m_Tooltip.GetContent() || !m_Tooltip.IsValid("Enable", m_Widgets.m_wEnableAddonButton))
			return;
		
		if (m_Item.GetEnabled())
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_DISABLE);
		else
			m_Tooltip.GetContent().SetMessage(SCR_WorkshopUiCommon.LABEL_ENABLE);
	}

//---- REFACTOR NOTE END ----	

	// ---- Public ----
	//------------------------------------------------------------------------------------------------
	//! If a null pointer is passed, tile becomes hidden
	void SetWorkshopItem(SCR_WorkshopItem workshopItem)
	{
		// Unregister from previous item
		if (m_Item)
		{
			m_Item.m_OnChanged.Remove(OnWorkshopItemChange);
			m_Item.m_OnGetAsset.Remove(OnWorkshopItemChange);
			m_Item.m_OnDependenciesLoaded.Remove(OnWorkshopItemChange);
			m_Item = null; // Release the old item
		}

		if (workshopItem)
		{
			m_Item = workshopItem;
			m_Item.m_OnChanged.Insert(OnWorkshopItemChange);
			m_Item.m_OnGetAsset.Insert(OnWorkshopItemChange);
			m_Item.m_OnDependenciesLoaded.Insert(OnWorkshopItemChange);
		}

		UpdateAllWidgets();

		// Since we reuse these tiles, when we assign a new workshop item we should not animate the effects, but we must force them into the correct value
		m_Widgets.m_EnableAddonButtonComponent.InvokeAllEnabledEffects(true);
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocus()
	{
		return m_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFocusLost()
	{
		return m_OnFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnChange()
	{
		return m_OnChange;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ContentBrowserTileComponent FindComponent(notnull Widget w)
	{
		return SCR_ContentBrowserTileComponent.Cast(w.FindHandler(SCR_ContentBrowserTileComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetTileWidth()
	{
		return s_iTileWidth;
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetTileHeight()
	{
		return s_iTileHeight;
	}
}

// Displayed in the warning overlay
enum SCR_EWorkshopTileErrorState
{
	NONE,
	RESTRICTED,
	REVISION_AVAILABILITY_ISSUE,
	DEPENDENCIES_ISSUE,
	UNLISTED,
	DAMAGED
}
