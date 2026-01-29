#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Edit Engine Settings", category: "User Settings", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF4FE)]
class SCR_ReforgerEngineSettingsPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.SetOpenedResource("$profile:.save/settings/ReforgerEngineSettings.conf");
	}
}
#endif // WORKBENCH
