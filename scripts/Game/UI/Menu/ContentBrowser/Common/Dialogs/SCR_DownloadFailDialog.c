/*!
Dialog displayed when downloading fails
*/

//------------------------------------------------------------------------------------------------
class SCR_DownloadFailDialog : SCR_DownloadConfirmationDialog
{
	//------------------------------------------------------------------------------------------------
	static SCR_DownloadFailDialog CreateFailedAddonsDialog(notnull array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions, bool subscribeToAddons)
	{
		SCR_DownloadFailDialog dlg = new SCR_DownloadFailDialog();
	
		SetupAddons(dlg, addonsAndVersions, subscribeToAddons);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "download_failed", dlg);
		
		return dlg;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void SetupAddons(notnull out SCR_DownloadFailDialog dialog, notnull array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions, bool subscribeToAddons)
	{
		dialog.m_bDownloadMainItem = false;
		dialog.m_bSubscribeToAddons = subscribeToAddons;
		dialog.m_aDependencies = new array<ref SCR_WorkshopItem>;
		dialog.m_aDependencyVersions = new array<ref Revision>;
		
		foreach (Tuple2<SCR_WorkshopItem, ref Revision> i : addonsAndVersions)
		{
			dialog.m_aDependencies.Insert(i.param1);
			dialog.m_aDependencyVersions.Insert(i.param2);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		m_Widgets.m_UpdateSpacer.SetVisible(false);
	}
};