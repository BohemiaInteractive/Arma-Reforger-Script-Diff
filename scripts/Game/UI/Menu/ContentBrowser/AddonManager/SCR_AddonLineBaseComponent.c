/*!
Class for addon line button base.
This component just vizualize state and invoke interaction.
Items are not manipulated in this component.
*/

class SCR_AddonLineBaseComponent : SCR_ListMenuEntryComponent
{
	// Fields
	protected ref SCR_AddonLineBaseWidgets m_Widgets = new SCR_AddonLineBaseWidgets();

	protected ref SCR_WorkshopItem m_Item;
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected bool m_bUpdatingContinuously;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnEnableButton;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnDisableButton;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnFixButton;

	// Image effects
	protected SCR_ButtonEffectColor m_UpdateButtonIconColor;

	// Error flags
	protected SCR_EAddonLineErrorFlags m_eAddonLineErrorFlags;

	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_Widgets.Init(w);

		// Setup
		m_aMouseButtons.Insert(m_Widgets.m_MoveLeftButtonComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_MoveRightButtonComponent0);
		m_aMouseButtons.Insert(m_Widgets.m_DeleteButtonComponent0);

		// Button callbacks
		m_Widgets.m_DeleteButtonComponent0.m_OnClicked.Insert(OnDeleteButton);
		m_Widgets.m_MoveRightButtonComponent0.m_OnClicked.Insert(OnEnableButton);
		m_Widgets.m_MoveLeftButtonComponent0.m_OnClicked.Insert(OnDisableButton);

		// Fix update
		m_Widgets.m_UpdateButtonComponent0.m_OnClicked.Insert(OnUpdateButton);
		m_Widgets.m_FixButtonComponent0.m_OnClicked.Insert(OnFixButton);

		// Find button image effects
		m_UpdateButtonIconColor = SCR_ButtonEffectColor.Cast(m_Widgets.m_UpdateButtonComponent0.FindEffect(SCR_ListEntryHelper.EFFECT_ICON_COLOR));

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Insert(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Insert(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Insert(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Insert(OnAnyDownloadCompleted);
		}

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		if (downloadManager)
		{
			downloadManager.m_OnNewDownload.Remove(OnAnyNewDownload);
			downloadManager.m_OnDownloadFailed.Remove(OnAnyDownloadError);
			downloadManager.m_OnDownloadCanceled.Remove(OnAnyDownloadCompleted);
			downloadManager.m_OnDownloadComplete.Remove(OnAnyDownloadCompleted);
		}

		StopContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool result = super.OnFocus(w, x, y);
		UpdateAllWidgets();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool result = super.OnFocusLost(w, x, y);
		UpdateAllWidgets();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		super.UpdateModularButtons();
		
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_Widgets.m_MoveRightButtonComponent0, m_bUnavailable, m_bFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		super.OnTooltipShow(tooltip);

		if (tooltip.IsValid("IncompatibleAddonVersion", m_Widgets.m_wIncompatibleButton))
			OnIncompatibleTooltipShow(tooltip);

		else if (tooltip.IsValid("Update", m_Widgets.m_wUpdateButton))
			OnUpdateTooltipShow(tooltip);

		else if (tooltip.IsValid("MissingDependencies", m_Widgets.m_wFixButton))
			OnDependenciesTooltipShow(tooltip);

		else if (tooltip.IsValid("Corrupted", m_Widgets.m_wRepairButton))
			OnDependenciesTooltipShow(tooltip);

		else if (tooltip.IsValid("Disable", m_Widgets.m_wMoveLeftButton))
			OnSideTooltipShow(tooltip);

		else if (tooltip.IsValid("Enable", m_Widgets.m_wMoveRightButton))
			OnSideTooltipShow(tooltip);
	}

	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	//! Update vizual state of line
	protected void UpdateAllWidgets()
	{
		if (!m_Item)
			return;

		HandleDownloadChanges();

		// Update name
		m_Widgets.m_wNameText.SetText(m_Item.GetName());
		
		// Show type 
		string imageType = "modIcon";
		if (m_Item.IsWorldSave())
			imageType = "save-published";
		
		m_Widgets.m_wItemTypeImage.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, imageType);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleDownloadChanges()
	{
		if (!m_Item)
			return;

		UpdateIssueState();

		UpdateCompatibilityWidgets();
		UpdateDownloadProgressWidgets();

		// Delete Button
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		m_Widgets.m_DeleteButtonComponent0.SetVisible(
			!SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item) &&
			m_bFocused &&
			availability != SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED &&
			GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE
		);

		// Dependencies Button
		m_Widgets.m_FixButtonComponent0.SetVisible(RequiresDownloadingDependencies());

		// Repair Button
		WorkshopItem item = m_Item.GetWorkshopItem();
		Revision itemRevision = item.GetActiveRevision();
		m_Widgets.m_RepairButtonComponent0.SetVisible(itemRevision.IsCorrupted());

		// Continuous update
		if (SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item))
			StartContinuousUpdate();
		else
			StopContinuousUpdate();
	}

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
		if (!SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item))
			StopContinuousUpdate();

		UpdateDownloadProgressWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleEnableButtons(bool addonEnabled, bool forceHidden = false)
	{
		// Left Panel
		m_Widgets.m_wLeftSeparator.SetVisible(!addonEnabled || forceHidden);
		m_Widgets.m_wSizeMoveRight.SetVisible(!addonEnabled && !forceHidden);
		m_Widgets.m_MoveRightButtonComponent0.SetVisible(!addonEnabled && m_bFocused && !forceHidden);

		// Right Panel
		m_Widgets.m_wRightSeparator.SetVisible(addonEnabled || forceHidden);
		m_Widgets.m_wSizeMoveLeft.SetVisible(addonEnabled || forceHidden);
		m_Widgets.m_MoveLeftButtonComponent0.SetVisible(addonEnabled && m_bFocused && !forceHidden);
	}

	//------------------------------------------------------------------------------------------------
	//! Display incopatible icon
	//! Set icon to incompatible and update base on current state
	protected void UpdateCompatibilityWidgets()
	{
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());

		// Incompatible
		bool showIncompatibility = 	availability != SCR_ERevisionAvailability.ERA_AVAILABLE &&
									availability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;

		m_Widgets.m_IncompatibleButtonComponent0.SetVisible(showIncompatibility);

		if (showIncompatibility)
		{
			string texture = SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(m_Item.Internal_GetWorkshopItem());
			if (!texture.IsEmpty())
				m_Widgets.m_IncompatibleButtonComponent1.SetImage(texture);
		}

		// Update
		// Displaying button for update/cancel download
		bool isDownloading = SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item);
		m_Widgets.m_UpdateButtonComponent0.SetVisible(isDownloading || m_Item.GetUpdateAvailable());

		string texture = SCR_WorkshopUiCommon.ICON_UPDATE;
		Color color = UIColors.CopyColor(UIColors.SLIGHT_WARNING);
		if (isDownloading)
		{
			texture = SCR_WorkshopUiCommon.ICON_CANCEL_DOWNLOAD;
			color = UIColors.CopyColor(UIColors.NEUTRAL_ACTIVE_STANDBY);
		}

		m_Widgets.m_UpdateButtonComponent1.SetImage(texture);
		if (m_UpdateButtonIconColor)
			m_UpdateButtonIconColor.m_cDefault = color;

		m_Widgets.m_UpdateButtonComponent0.InvokeAllEnabledEffects(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressWidgets()
	{
		m_Widgets.m_wHorizontalState.SetVisible(SCR_WorkshopUiCommon.IsDownloadingAddon(m_Item));

		if (!m_Widgets.m_wHorizontalState.IsVisible())
			return;

		string percentage = UIConstants.FormatUnitPercentage(SCR_WorkshopUiCommon.GetDownloadProgressPercentage(SCR_DownloadManager.GetItemDownloadActionsProgress(m_Item)));
		string message = WidgetManager.Translate("%1 %2", SCR_WorkshopUiCommon.DOWNLOAD_STATE_DOWNLOADING, percentage);

		m_Widgets.m_wStateText.SetText(message);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateIssueState()
	{
		m_eAddonLineErrorFlags = 0;

		if (!m_Item)
		{
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.ITEM_ISSUES;
			return;
		}

		//TODO: unify with workshop tiles, as there's no reason to have separate state handling of what's just another representation of an addon

		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());
		bool downloadIssue = availability == SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED;
		bool issues = HasItemAnyIssue();

		m_bUnavailable = issues || downloadIssue;

		// Update flags
		if (issues)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.ITEM_ISSUES;

		if (availability != SCR_ERevisionAvailability.ERA_AVAILABLE && !downloadIssue)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.REVISION_AVAILABILITY_ISSUE;

		if (RequiresDownloadingDependencies())
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.MISSING_DEPENDENCIES;

		if (downloadIssue)
			m_eAddonLineErrorFlags |= SCR_EAddonLineErrorFlags.DOWNLOAD_ISSUES;
	}

	//------------------------------------------------------------------------------------------------
	protected bool RequiresDownloadingDependencies()
	{
		if (!m_Item)
			return false;

		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();

		if (!dependencies.IsEmpty())
			dependencies = SCR_AddonManager.SelectItemsOr(dependencies, EWorkshopItemQuery.NOT_OFFLINE | EWorkshopItemQuery.UPDATE_AVAILABLE);

		return !dependencies.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasItemAnyIssue()
	{
		EWorkshopItemProblem problem = m_Item.GetHighestPriorityProblem();

		bool hasIssue = (
			problem == EWorkshopItemProblem.BROKEN ||
			problem == EWorkshopItemProblem.DEPENDENCY_MISSING ||
			problem == EWorkshopItemProblem.DEPENDENCY_OUTDATED
		);

		return hasIssue;
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

	// --- Callbacks ---
	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopItemChange()
	{
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeleteButton()
	{
		SCR_WorkshopUiCommon.OnDeleteAddonButton(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnActionButton()
	{
		if (!m_Item)
			return;

		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFixButton()
	{
		if (m_OnFixButton)
			m_OnFixButton.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void OnUpdateButton()
	{
		if (!m_Item)
			return;

		SCR_WorkshopUiCommon.ExecutePrimaryAction(m_Item, m_DownloadRequest);
	}

	//------------------------------------------------------------------------------------------------
	void OnOpenDetailsButton()
	{
		if (!m_Item)
			return;

		ContentBrowserDetailsMenu.OpenForWorkshopItem(m_Item);
	}

	//------------------------------------------------------------------------------------------------
	void OnEnableButton()
	{
		if (!CanBeEnabled())
			return;

		// Enable
		if (m_OnEnableButton)
			m_OnEnableButton.Invoke(this);

		HandleEnableButtons(true);
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	void OnDisableButton()
	{
		if (!m_Item)
			return;

		if (m_OnDisableButton)
			m_OnDisableButton.Invoke(this);

		HandleEnableButtons(false);
		UpdateModularButtons();
	}

	// --- Tooltips ---
	//------------------------------------------------------------------------------------------------
	protected void OnUpdateTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		string message = SCR_WorkshopUiCommon.LABEL_UPDATE;
		if (SCR_WorkshopUiCommon.IsDownloadingAddonOrDependencies(m_Item))
			message = SCR_WorkshopUiCommon.LABEL_CANCEL;

		SCR_ScriptedWidgetTooltipContentBase content = m_CurrentTooltip.GetContent();
		if (content)
			content.SetMessage(message);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnIncompatibleTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		SCR_ERevisionAvailability availability = SCR_AddonManager.ItemAvailability(m_Item.Internal_GetWorkshopItem());

		SCR_ScriptedWidgetTooltipContentBase content = tooltip.GetContent();
		if (!content)
			return;

		content.SetMessage(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(availability));
		content.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSideTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		SCR_ActionHintScriptedWidgetTooltip content = SCR_ActionHintScriptedWidgetTooltip.Cast(tooltip.GetContent());
		if (!content)
			return;

		if (!CanBeEnabled() && !IsEnabled())
		{
			content.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			content.SetActionColor(Color.FromInt(UIColors.IDLE_DISABLED.PackToInt()));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDependenciesTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		SCR_ScriptedWidgetTooltipContentBase content = tooltip.GetContent();
		if (!content)
			return;

		content.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
	}

	// --- Public ---
	//------------------------------------------------------------------------------------------------
	bool IsEnabled();

	//------------------------------------------------------------------------------------------------
	//! Setup line
	void Init(SCR_WorkshopItem item)
	{
		m_Item = item;

		if (m_Item)
		{
			m_Item.m_OnChanged.Insert(OnWorkshopItemChange);
			m_Item.m_OnGetAsset.Insert(OnWorkshopItemChange);
			m_Item.m_OnDependenciesLoaded.Insert(OnWorkshopItemChange);
			if (!m_Item.GetDetailsLoaded())
				m_Item.LoadDetails();
		}

		UpdateAllWidgets();
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EAddonLineErrorFlags GetErrorFlags()
	{
		return m_eAddonLineErrorFlags;
	}

	//------------------------------------------------------------------------------------------------
	// Addons with issues should not be enabled, except for availability issues since those only prevent online play
	bool CanBeEnabled()
	{
		UpdateIssueState();

		bool canBeEnabled =
			m_Item &&
			!(m_eAddonLineErrorFlags & SCR_EAddonLineErrorFlags.MISSING_DEPENDENCIES) &&
			!(m_eAddonLineErrorFlags & SCR_EAddonLineErrorFlags.ITEM_ISSUES) &&
			!(m_eAddonLineErrorFlags & SCR_EAddonLineErrorFlags.DOWNLOAD_ISSUES);

		return canBeEnabled;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnEnableButton()
	{
		if (!m_OnEnableButton)
			m_OnEnableButton = new ScriptInvokerScriptedWidgetComponent();

		return m_OnEnableButton;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnDisableButton()
	{
		if (!m_OnDisableButton)
			m_OnDisableButton = new ScriptInvokerScriptedWidgetComponent();

		return m_OnDisableButton;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnFixButton()
	{
		if (!m_OnFixButton)
			m_OnFixButton = new ScriptInvokerScriptedWidgetComponent();

		return m_OnFixButton;
	}
}

//TODO: unify error states, there's no reason this should have it's custom handling compared to workshop tiles or other UI reperesentations of mods
enum SCR_EAddonLineErrorFlags
{
	MISSING_DEPENDENCIES		= 1 << 0,
	ITEM_ISSUES					= 1 << 1,
	REVISION_AVAILABILITY_ISSUE	= 1 << 2,
	DOWNLOAD_ISSUES				= 1 << 3
}
