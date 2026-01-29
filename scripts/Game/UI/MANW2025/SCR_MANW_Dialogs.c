class SCR_MANW_Dialogs
{
	protected const string CONFIG_DIALOG 	= "{A6D2D86E123D905F}Configs/Dialogs/MANW_Dialogs.conf";
	protected const string CONFIG_ENTRY 	= "DialogMANW";
	
	//------------------------------------------------------------------------------------------------
	static void CreateBannerDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(CONFIG_DIALOG, CONFIG_ENTRY, new SCR_MANW_BannerDialog);
	}
}