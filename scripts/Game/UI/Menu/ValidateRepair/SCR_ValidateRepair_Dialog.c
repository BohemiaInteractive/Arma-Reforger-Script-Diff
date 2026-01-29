class SCR_ValidateRepair_Dialog : SCR_TabDialog
{
	protected const ResourceName DOWNLOAD_LINE_LAYOUT 	= "{8E94F18104D88E73}UI/layouts/Menus/ContentBrowser/ValidateRepair/ValidateRepairLine.layout";
	protected const string WIDGET_VERTICAL_NAME 		= "DownloadManagerVertical";
	protected const string WIDGET_SCROLL_NAME 			= "DownloadManagerScroll";

	protected const string WIDGET_STATUS_TEXT_NAME 		= "m_GlobalStatusText";

	protected const string TEXT_VALIDATING_FORMAT 		= "#AR-AddonValidate_ValidationProgress";
	protected const string TEXT_VALIDATED_FORMAT 		= "#AR-AddonValidate_ValidationFinished";
	protected const string TEXT_ERROR_FORMAT 			= "#AR-AddonValidate_ValidationError";

	protected const string BUTTON_VALIDATE 		= "validation";
	protected const string BUTTON_REDOWNLOAD 	= "redownload";

	protected const ResourceName ADDON_VISUALS_CONFIG 	= "{DFFE187A7738CE15}scripts/Game/UI/Menu/ValidateRepair/AddonLineValidateStates.conf";

	protected ref map<SCR_AddonLineValidateRepairComponent, SCR_WorkshopItem> m_mEntriesComponents = new map<SCR_AddonLineValidateRepairComponent, SCR_WorkshopItem>();

	protected ref array<SCR_AddonLineValidateRepairComponent> m_aEntriesValidating = {};
	protected ref array<SCR_AddonLineValidateRepairComponent> m_aEntriesRepairing = {};
	protected ref array<SCR_AddonLineValidateRepairComponent> m_aEntriesDownloading = {};

	protected SCR_AddonLineValidateRepairComponent m_FocusedAddon;

	//! Holds all the different cached id with the enum as key
	protected ref map<SCR_EAddonValidateState, ref SCR_AddonLineValidateState> m_mCachedAddonStates;
	protected ref map<int, string> m_mCachedAddonErrors;

	protected SCR_InputButtonComponent m_ButtonValidate;
	protected SCR_InputButtonComponent m_ButtonRedownload;
	protected TextWidget m_wGlobalStatusText;
	
	protected int m_iValidationCount;
	protected int m_iFailedValidationCount;
	protected bool m_bValidationStarting;

	//------------------------------------------------------------------------------------------------
	bool HasRunningValidation()
	{
		return !m_aEntriesValidating.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	bool HasRunningRepair()
	{
		return !m_aEntriesRepairing.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	bool HasRunningDownload()
	{
		return !m_aEntriesDownloading.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		Resource rsc = BaseContainerTools.LoadContainer(ADDON_VISUALS_CONFIG);
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		SCR_AddonLineValidateStates statesConfig = SCR_AddonLineValidateStates.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		if (statesConfig)
		{
			m_mCachedAddonStates = new map<SCR_EAddonValidateState, ref SCR_AddonLineValidateState>();
			foreach (SCR_AddonLineValidateState state : statesConfig.m_aStates)
			{
				if (!m_mCachedAddonStates.Contains(state.GetState()))
					m_mCachedAddonStates.Insert(state.GetState(), state);
			}

			m_mCachedAddonErrors = new map<int, string>();
			foreach (SCR_AddonLineValidateError errorType : statesConfig.m_aErrors)
			{
				if (!m_mCachedAddonErrors.Contains(errorType.GetErrorCode()))
					m_mCachedAddonErrors.Insert(errorType.GetErrorCode(), errorType.GetText());
			}
		}

		m_wGlobalStatusText = TextWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_STATUS_TEXT_NAME));
		m_wGlobalStatusText.SetText(string.Empty);
	}


	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		Widget vLayout = m_wRoot.FindAnyWidget(WIDGET_VERTICAL_NAME);
		ScrollLayoutWidget scroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCROLL_NAME));

		if (!scroll || !vLayout)
			return;
		
		Widget childWidget = vLayout.GetChildren();
		while (childWidget)
		{
			vLayout.RemoveChild(childWidget);
			childWidget = vLayout.GetChildren();
		}
		
		super.OnMenuClose();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);

		m_ButtonValidate = FindButton(BUTTON_VALIDATE);
		BindButtonValidate(m_ButtonValidate);

		m_ButtonRedownload = FindButton(BUTTON_REDOWNLOAD);
		BindButtonRedownload(m_ButtonRedownload);
		m_ButtonRedownload.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void BindButtonValidate(notnull SCR_InputButtonComponent button)
	{
		button.m_OnActivated.Insert(OnValidateAllButton);

		if (m_aButtonComponents.Contains(BUTTON_VALIDATE))
			m_aButtonComponents.Set(BUTTON_VALIDATE, button);
		else
			m_aButtonComponents.Insert(BUTTON_VALIDATE, button);
	}

	//------------------------------------------------------------------------------------------------
	protected void BindButtonRedownload(notnull SCR_InputButtonComponent button)
	{
		button.m_OnActivated.Insert(OnReDownloadSelectedAddon);

		if (m_aButtonComponents.Contains(BUTTON_REDOWNLOAD))
			m_aButtonComponents.Set(BUTTON_REDOWNLOAD, button);
		else
			m_aButtonComponents.Insert(BUTTON_REDOWNLOAD, button);
	}

	//------------------------------------------------------------------------------------------------
	void LoadAddons(bool enabledAddons)
	{
		InitList(enabledAddons);
	}

	//------------------------------------------------------------------------------------------------
	void LoadAddon(notnull SCR_WorkshopItem item)
	{
		CreateList({item});

		OnValidateAllButton();
	}

	//------------------------------------------------------------------------------------------------
	// Init list of downloads
	protected void InitList(bool loadEnabledMods = true)
	{
		array<WorkshopItem> rawWorkshopItems = {};
		GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(rawWorkshopItems);

		// Register items in Addon Manager
		array<SCR_WorkshopItem> itemsRegistered = {};
		
		Revision itemRev;
		SCR_WorkshopItem itemRegistered;
		foreach (WorkshopItem i : rawWorkshopItems)
		{
			itemRegistered = SCR_AddonManager.GetInstance().Register(i);

			if (itemRegistered.IsWorldSave())
				continue;

			if (itemRegistered.GetEnabled() != loadEnabledMods)
				continue;
			
			itemRev = itemRegistered.GetCurrentLocalRevision();
			if (!itemRev.IsDownloaded())
				continue;

			// Addons
			itemsRegistered.Insert(itemRegistered);
		}

		// Sort by name...
		SCR_Sorting<SCR_WorkshopItem, SCR_CompareWorkshopItemName>.HeapSort(itemsRegistered);

		// Convert back to array<ref ...>
		array<ref SCR_WorkshopItem> itemsSorted = {};
		foreach (SCR_WorkshopItem i : itemsRegistered)
		{
			itemsSorted.Insert(i);
		}

		CreateList(itemsSorted);

		OnValidateAllButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateList(array<ref SCR_WorkshopItem> items)
	{
		Widget vLayout = m_wRoot.FindAnyWidget(WIDGET_VERTICAL_NAME);
		ScrollLayoutWidget scroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCROLL_NAME));

		if (!scroll || !vLayout)
			return;

		// Store scroll pos
		float scrollx, scrolly;
		scroll.GetSliderPos(scrollx, scrolly);

		foreach (SCR_WorkshopItem item : items)
		{
			Widget entry = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, vLayout);

			SCR_AddonLineValidateRepairComponent comp = SCR_AddonLineValidateRepairComponent.Cast(entry.FindHandler(SCR_AddonLineValidateRepairComponent));
			comp.Init(item, m_mCachedAddonStates, m_mCachedAddonErrors);

			// Store widgets and component
			m_mEntriesComponents.Insert(comp, item);
			comp.GetOnFocus().Insert(OnFocusAddon);
			comp.GetOnFocusLost().Insert(OnFocusLostAddon);

			comp.GetOnValidateStart().Insert(OnAddonStartValidating);
			comp.GetOnValidateEnd().Insert(OnAddonValidated);

			comp.GetOnRepairStart().Insert(OnAddonStartRepairing);
			comp.GetOnRepairEnd().Insert(OnAddonRepaired);
		}

		// Restore scroll pos
		scroll.SetSliderPos(scrollx, scrolly);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateStatusText()
	{
		if (!m_wGlobalStatusText || m_bValidationStarting)
			return;
		
		int progressCount = m_aEntriesValidating.Count() + m_aEntriesRepairing.Count() + m_aEntriesDownloading.Count();
		if (progressCount > 0 && progressCount <= m_iValidationCount)
			m_wGlobalStatusText.SetTextFormat(TEXT_VALIDATING_FORMAT, m_iValidationCount - progressCount, m_iValidationCount);
		else
		{
			if (m_iFailedValidationCount > 0)
				m_wGlobalStatusText.SetTextFormat(TEXT_ERROR_FORMAT, m_iFailedValidationCount);
			else
				m_wGlobalStatusText.SetText(TEXT_VALIDATED_FORMAT);
		
			m_ButtonValidate.SetEnabled(true);
			m_ButtonRedownload.SetEnabled(true);
			
			//! Hackfix
			GetGame().GetCallqueue().Call(DelayedDownloadButtonVisibility);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedDownloadButtonVisibility()
	{
		if (m_FocusedAddon && m_FocusedAddon.HasFailed() && !m_FocusedAddon.HasFailedDownloading())
			m_ButtonRedownload.SetVisible(true);
		else
			m_ButtonRedownload.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFocusAddon(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		m_FocusedAddon = addonComponent;

		if (m_FocusedAddon.HasFailed() && !m_FocusedAddon.HasFailedDownloading())
			m_ButtonRedownload.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFocusLostAddon(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		if (m_FocusedAddon == addonComponent)
			m_FocusedAddon = null;

		m_ButtonRedownload.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonStartValidating(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		m_aEntriesValidating.Insert(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonStartRepairing(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		m_aEntriesRepairing.Insert(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonStartDownloading(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		m_aEntriesDownloading.Insert(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonDownloaded(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		if (addonComponent.HasFailed())
			m_iFailedValidationCount++;
		
		m_aEntriesDownloading.RemoveItem(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonValidated(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		if (addonComponent.HasFailed())
			m_iFailedValidationCount++;
		
		m_aEntriesValidating.RemoveItem(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAddonRepaired(notnull SCR_AddonLineValidateRepairComponent addonComponent)
	{
		if (addonComponent.HasFailed())
			m_iFailedValidationCount++;
		
		m_aEntriesRepairing.RemoveItem(addonComponent);
		UpdateStatusText();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnValidateAllButton()
	{
		m_bValidationStarting = true;
		m_wGlobalStatusText.SetText("");
		
		m_iFailedValidationCount = 0;
		m_iValidationCount = m_aEntriesValidating.Count() + m_aEntriesRepairing.Count() + m_aEntriesDownloading.Count();
		foreach (SCR_AddonLineValidateRepairComponent component, SCR_WorkshopItem item : m_mEntriesComponents)
		{
			if (component.StartValidating())
				m_iValidationCount++;
		}
		
		if (m_iValidationCount > 0)
			DisableButtons();
		
		m_bValidationStarting = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnReDownloadSelectedAddon()
	{
		if (m_FocusedAddon && m_FocusedAddon.StartCleanDownload())
		{
			m_iValidationCount++;
			DisableButtons();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisableButtons()
	{
		m_ButtonValidate.SetEnabled(false);
		//m_ButtonRedownload.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCloseButton()
	{
		Close();
	}
}
