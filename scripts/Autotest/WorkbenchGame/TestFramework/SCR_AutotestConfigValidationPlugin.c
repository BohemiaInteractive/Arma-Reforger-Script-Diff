#ifdef WORKBENCH

[WorkbenchPluginAttribute(
	name: "Validate autotest configs",
	category: "QA",
	description: "Tries to load all existing autotest group configs for validation.\nResults must be checked in console.log.",
	// shortcut: "F4",
	wbModules: { "ResourceManager" },
	awesomeFontCode: 0xF188
)]
class SCR_AutotestConfigValidationPlugin : WorkbenchPlugin
{	
	override void Run()
	{
		array<string> addonGuids = {};
		GameProject.GetAvailableAddons(addonGuids);
		
		WBProgressDialog progress = new WBProgressDialog("Checking autotest config", null);
		
		foreach (string addonGuid : addonGuids)
		{
			string addonId = GameProject.GetAddonID(addonGuid);
			progress.SetActionTitle("Processing " + addonId);
			progress.SetProgress(0);
			
			set<ResourceName> autotestConfigGroups = SCR_TestResourceDatabase.Search(
				path: string.Format("$%1:Configs/", addonId),
				extensions: {"conf"},
				allowedTypes: {SCR_AutotestGroup}
			);
			
			int total = autotestConfigGroups.Count();
			foreach (int i, ResourceName configResourceName : autotestConfigGroups)
			{
				ValidateResource(configResourceName);
				progress.SetProgress(1 / (total / (i+1)));
			}
			
			PrintFormat("Checked %1 configs in: %2", autotestConfigGroups.Count(), addonId);
		}
		
		delete progress;
		const string msg = "See console.log output to see if there are any errors in the config files";
		PrintFormat(msg, level: LogLevel.NORMAL);
		Workbench.Dialog("", msg);
	}
	
	void ValidateResource(ResourceName resourceName)
	{
		Resource resource = Resource.Load(resourceName);
		
		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		if (!baseContainer)
		{
			PrintFormat("The provided config does not have a base container.", LogLevel.ERROR);
			return;
		}

		Managed managed = BaseContainerTools.CreateInstanceFromContainer(baseContainer);
		if (!managed)
		{
			PrintFormat("The provided config cannot be instanciated.", LogLevel.ERROR);
			return;
		}
	}
}

#endif