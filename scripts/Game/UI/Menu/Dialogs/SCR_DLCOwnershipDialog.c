class SCR_DLCOwnershipDialog : SCR_ConfigurableDialogUi
{
	//expand this to have more images per specific DLC when we have enums for it
	protected ResourceName m_sDLCImage;

	//------------------------------------------------------------------------------------------------
	void SCR_DLCOwnershipDialog(ResourceName dlcImage = "{96A553E9F0E57986}UI/Textures/DLC/RallyDLCShowcase.edds")
	{
		m_sDLCImage = dlcImage;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "dlc_not_owned", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		ImageWidget dlcImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("DLCImage"));
		if (dlcImage)
			dlcImage.LoadImageTexture(0, m_sDLCImage);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		DlcManager manager = DlcManager.Get();
		if (!manager)
			return;
		
		//for now use Rally, expand depending on where the content is from if needed in future
		manager.OpenStore(EDlc.RALLY);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		DlcManager manager = DlcManager.Get();
		if (manager)
			manager.RefreshLicenses();
		
		Close();
	}
};
