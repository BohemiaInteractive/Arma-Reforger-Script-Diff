
class SCR_AutotestHelper
{
	//------------------------------------------------------------------------------------------------
	static bool WorldOpenFile(ResourceName mapResource, ResourceName worldSystemsConfigResourceName)
	{
		if (!GetGame().InPlayMode())
		{
			bool result = false;
#ifdef WORKBENCH
			WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
			result = worldEditor.SetOpenedResource(mapResource);
			worldEditor.SwitchToGameMode();
#endif
			return result;
		}
		else
		{
			SCR_AutotestHarness.GetLogger().Log(
				string.Format("Requesting scenario change:\n\t%1\n\t%2", mapResource, worldSystemsConfigResourceName),
				level: LogLevel.DEBUG,
				forceFileWrite: true			
			);
			
			return SCR_AutotestHelper.RequestScenarioChangeTransition(mapResource, worldSystemsConfigResourceName);
		}
	}
	
	static void SwitchToGameMode()
	{
#ifdef WORKBENCH
		if (!GetGame().InPlayMode())
		{
			WorldEditor we = Workbench.GetModule(WorldEditor);
			if (!we.GetApi())
			{
				Print("WorldEditor is not open!", LogLevel.ERROR);
				return;
			}

			we.SwitchToGameMode();
		}
#endif
	}
	
	// OVERRIDE in game project

	//------------------------------------------------------------------------------------------------
	//! Game specific way to set world time multiplier if supported.
	static void WorldSetTimeCoef(int coef);

	//------------------------------------------------------------------------------------------------
	//! Game specific default world for testing.
	static ResourceName GetDefaultWorld();

	//------------------------------------------------------------------------------------------------
	//! Game specific default systems config for testing.
	static ResourceName GetDefaultSystemsConfig();
	
	//------------------------------------------------------------------------------------------------
	//! Game specific launch arguments, used when launching tests in new window from WB.
	static string GetDefaultLaunchParams();

	//------------------------------------------------------------------------------------------------
	//! Game specific way to transition to world for testing.
	protected static bool RequestScenarioChangeTransition(ResourceName mapResource, ResourceName worldSystemsConfigResourceName);
}
