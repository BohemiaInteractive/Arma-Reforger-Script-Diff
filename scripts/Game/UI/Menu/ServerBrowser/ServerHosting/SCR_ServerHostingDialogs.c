class SCR_ServerHostingDialogs
{
	static const ResourceName DIALOGS_CONFIG = "{7E4D962E3084CD77}Configs/ServerBrowser/ServerHosting/Dialogs/ServerConfigDialogs.conf";
	
	//---------------------------------------------------------------------------------------------
	static SCR_ServerConfigSaveDialog CreateSaveConfirmDialog()
	{
		SCR_ServerConfigSaveDialog dialog = new SCR_ServerConfigSaveDialog();
		
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "save_confirm", dialog);
		
		return dialog;
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateSaveSuccessDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "save_successful");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateSaveFailedDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "save_failed");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateSaveOverrideDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "save_override");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateNoConnectionDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "no_connection");
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateRequiredDisableDialog()
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "required_disable");
	}
}