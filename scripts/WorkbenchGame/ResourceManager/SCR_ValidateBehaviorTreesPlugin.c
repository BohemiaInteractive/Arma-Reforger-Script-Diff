#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Validate Behavior Trees",
	description: "Opens all behavior trees to verify them for errors",
	wbModules: { "ResourceManager" })]
class SCR_ValidateBehaviorTreesPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		Print("ValidateBehaviorTreesPlugin: Start", LogLevel.NORMAL);

		array<string> files = {};
		FileIO.FindFiles(files.Insert, "AI/", ".bt");

		foreach (string file : files)
		{
			Print(string.Format("Opening: %1", file), LogLevel.NORMAL);
			Workbench.OpenResource(file);
		}

		Print("ValidateBehaviorTreesPlugin: End", LogLevel.NORMAL);
	}
}
#endif // WORKBENCH
