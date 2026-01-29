#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Edit Game Settings", category: "User Settings", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF406)]
class SCR_ReforgerGameSettingsPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.SetOpenedResource("$profile:.save/settings/ReforgerGameSettings.conf");
	}
}
#endif // WORKBENCH
