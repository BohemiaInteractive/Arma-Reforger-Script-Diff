class SCR_ModDetailsAdditionalInfoDialog : SCR_ConfigurableDialogUi
{	
	protected ref SCR_ModDetailsAdditionalInfoDialogContentWidgets m_Widgets = new SCR_ModDetailsAdditionalInfoDialogContentWidgets;

	protected SCR_WorkshopItem m_Item;	
	protected SCR_InputButtonComponent m_Licenses;

	//------------------------------------------------------------------------------------------------
	static SCR_ModDetailsAdditionalInfoDialog CreateDialog(SCR_WorkshopItem scriptedItem)
	{
		if (!scriptedItem)
			return null;
		
		WorkshopItem item = scriptedItem.GetWorkshopItem();
		if (!item)
			return null;
		
		string licenseText = item.LicenseText();
		
		array<WorkshopAuthor> contributors = {};
		int contributorsCount = item.Contributors(contributors);
		
		string tag = "AdditionalInfo";
		if (licenseText.IsEmpty() && contributorsCount == 0)
			tag = "AdditionalInfo_Small";
		
		SCR_ModDetailsAdditionalInfoDialog dialog = new SCR_ModDetailsAdditionalInfoDialog();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, tag, dialog);
		
		dialog.SetItem(scriptedItem);
		
		return dialog;
	}
	
	// --- Overrides ---
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		// Connection state
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);

		BindButtonCancel(m_DynamicFooter.FindButton(UIConstants.BUTTON_BACK));
		
		m_Licenses = m_DynamicFooter.FindButton("Licenses");
		if (m_Licenses)
			m_Licenses.m_OnActivated.Insert(OnLicensesButton);
		
		m_Widgets.Init(GetContentLayoutRoot());
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		ClearWorkshopItemInvokers();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
	}

	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	protected void UpdateWidgets(bool visible = true)
	{
		if (!m_Item)
			return;
		
		WorkshopItem item = m_Item.GetWorkshopItem();
		if (!item)
			return;
		
		// License
		string license = item.License();
		string customLicenseText = item.LicenseText();
		
		m_Widgets.m_wVerticalLayoutLicense.SetVisible(visible);
		m_Widgets.m_wLicenseName.SetText(license);
		
		m_Widgets.m_wLicenseContentOverlay.SetVisible(!customLicenseText.IsEmpty());
		m_Widgets.m_wLicenseContent.SetText(customLicenseText);
		
		// Contributors
		array<WorkshopAuthor> contributors = {};
		int contributorsCount = item.Contributors(contributors);
		string contributorsNames;
		
		foreach (int i, WorkshopAuthor contributor : contributors)
		{
			if (i == 0)
				contributorsNames = contributor.Name();
			else
				contributorsNames += "\n" + contributor.Name();
		}

		m_Widgets.m_wVerticalLayoutContributors.SetVisible(visible && contributorsCount > 0);
		m_Widgets.m_wContributorsContent.SetText(contributorsNames);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearWorkshopItemInvokers()
	{
		if (!m_Item)
			return;

		m_Item.m_OnGetAsset.Remove(OnItemLoaded);
		m_Item.m_OnTimeout.Remove(OnItemLoadTimeout);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemLoaded()
	{
		ClearWorkshopItemInvokers();
		
		m_Widgets.m_LoadingOverlayComponent.SetShown(false);
		UpdateWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemLoadTimeout()
	{
		ClearWorkshopItemInvokers();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{
		GetGame().GetCallqueue().Call(UpdateLicenseButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateLicenseButton()
	{
		SCR_ConnectionUICommon.SetConnectionButtonEnabled(m_Licenses, SCR_ServicesStatusHelper.SERVICE_WORKSHOP, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLicensesButton()
	{
		PlatformService ps = GetGame().GetPlatformService();
		if (!ps)
			return;

		string url = GetGame().GetBackendApi().GetLinkItem(SCR_WorkshopUiCommon.LINK_NAME_LICENSES);
		ps.OpenBrowser(url);
	}
	
	// --- Public ---
	//------------------------------------------------------------------------------------------------
	void SetItem(SCR_WorkshopItem scriptedItem)
	{
		if (!scriptedItem)
			return;
		
		ClearWorkshopItemInvokers();
		
		m_Item = scriptedItem;
		
		if (!m_Item.GetDetailsLoaded())
		{
			m_Item.m_OnGetAsset.Insert(OnItemLoaded);
			m_Item.m_OnTimeout.Insert(OnItemLoadTimeout);

			m_Item.LoadDetails();
			UpdateWidgets(false);
			
			return;
		}

		m_Widgets.m_LoadingOverlayComponent.SetShown(false);
		UpdateWidgets();
	}
}
