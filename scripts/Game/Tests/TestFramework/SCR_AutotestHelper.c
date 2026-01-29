
// TODO move to utils
class SCR_AutotestHelper
{
	static const ResourceName WORLD_EMPTY = "{B82E6DD9E16C665E}worlds/Autotest/Empty.ent";
	static const ResourceName WORLD_AUTOTEST_PLAIN = "{3048828FE14AE687}worlds/Autotest/Autotest_GameMode_Plain.ent";
	static const ResourceName WORLD_MPTEST = "{96A8AF57260A7392}worlds/MP/MpTest/MpTest.ent";
	
	static bool WorldOpenFile(ResourceName mapResource)
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
			string path = mapResource.GetPath();
			//HOTFIX: Add vanilla addon to the transition in order to prevent ilformed reload
			// TODO: This API have to be still improved, so the game could easily set scenario "without changing addons"
			return GameStateTransitions.RequestScenarioChangeTransition(path, GetGame().GetSystemsConfig(), "58D0FB3206B6F859");
		}
	}
}
