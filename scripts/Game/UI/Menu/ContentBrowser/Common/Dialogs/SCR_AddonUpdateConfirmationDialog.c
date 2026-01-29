/*!
Override downlad confirm dialog for handling addons update 
Always targets latest versions

POSSIBLE TODO: Update addons one by one, check for dependency update collisions
*/

//------------------------------------------------------------------------------------------------
class SCR_AddonUpdateConfirmationDialog : SCR_DownloadConfirmationDialog
{
	//------------------------------------------------------------------------------------------------
	//! Download addons with specific versions
	static SCR_AddonUpdateConfirmationDialog CreateForUpdates(notnull array<ref Tuple2<SCR_WorkshopItem, ref Revision>> addonsAndVersions, bool subscribeToAddons)
	{
		SCR_DownloadConfirmationDialog dlg = new SCR_AddonUpdateConfirmationDialog();
	
		SetupDownloadDialogAddons(dlg, addonsAndVersions, subscribeToAddons);
		
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "download_confirmation", dlg);
		
		dlg.SetStyleDownloadAddons();
			
		return SCR_AddonUpdateConfirmationDialog.Cast(dlg);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void CreateAddonList()
	{
		// Create lines for other addons
		m_Widgets.m_OtherAddonsSection.SetVisible(!m_aDependencies.IsEmpty());
		if (!m_aDependencies.IsEmpty())
		{
			// Not downloaded - not avaiable offline 
			array<ref SCR_WorkshopItem> missingAddons = SCR_AddonManager.SelectItemsBasic(m_aDependencies, EWorkshopItemQuery.NOT_OFFLINE);
			
			foreach (SCR_WorkshopItem missing : missingAddons)
			{
				CreateAddonWidget(missing, m_Widgets.m_OtherAddonsList);
			}
			
			// Version difference
			array<ref SCR_WorkshopItem> versionDifferences = SCR_AddonManager.SelectItemsAnd(
				m_aDependencies, 
				EWorkshopItemQuery.UPDATE_AVAILABLE | EWorkshopItemQuery.OFFLINE);
			
			int vDifCount = versionDifferences.Count();
			bool hasDifference = vDifCount > 0;
			
			// Display version difference widgets 
			m_Widgets.m_UpdateAddonsList.SetVisible(hasDifference);
			m_Widgets.m_UpdateAddonsMessage.SetVisible(hasDifference);

			if (hasDifference)
			{
				// Create woidgets 
				foreach (SCR_WorkshopItem addon : versionDifferences)
				{
					if (!addon.IsDownloadRunning())
						CreateAddonWidget(addon, m_Widgets.m_UpdateAddonsList);
				}
				
				// Version change text
				m_Widgets.m_UpdateAddonsMessage.SetTextFormat("#AR-Workshop_Dialog_ConfirmDownload_VersionDifferences", vDifCount);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create and setup addon widget text and verions
	protected override void CreateAddonWidget(notnull SCR_WorkshopItem dep, Widget listRoot)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, listRoot);
		SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
		
		// Setup addon revision
		Revision revision = dep.GetLatestRevision();
		
		//Revision revision = dep.GetDependency().GetRevision();
		comp.InitForWorkshopItem(dep, revision);
	}
}