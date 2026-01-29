[WorkbenchPluginAttribute(
	name: "Run Tracy Profiler",
	description: "Run Tracy Profiler",
	wbModules: { "ResourceManager", "ScriptEditor", "WorldEditor" },
	awesomeFontCode: 0x0054)]
class SCR_TracyPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		string binDir;
		Workbench.GetCwd(binDir);
		if (!Workbench.RunProcess(binDir + "/tracy/tracy-profiler.exe"))
			Print("SCR_TracyPlugin -> Tracy failed to start!", LogLevel.ERROR);
	}
}
