/*
	This file integrates the Autotest Framework into the game project.
*/

modded class SCR_AutotestHelper
{
	static const ResourceName WORLD_EMPTY = "{B82E6DD9E16C665E}worlds/Autotest/Empty.ent";
	static const ResourceName WORLD_AUTOTEST_PLAIN = "{3048828FE14AE687}worlds/Autotest/Autotest_GameMode_Plain.ent";
	static const ResourceName WORLD_MPTEST = "{96A8AF57260A7392}worlds/MP/MpTest/MpTest.ent";

	static const ResourceName SYSTEMS_CONFIG_BASE = "{1C60D2EDA2B468B8}Configs/Systems/BaseGameModeSystems.conf";

	override static ResourceName GetDefaultWorld()
	{
		return WORLD_AUTOTEST_PLAIN;
	}

	override static ResourceName GetDefaultSystemsConfig()
	{
		return GetGame().GetSystemsConfig();
	}
	
	override static string GetDefaultLaunchParams()
	{
		return "-profile ArmaReforger/autotest\n-logLevel debug\n-noFocus\n-forceUpdate\n-noThrow\n-window\n";
	}

	override static bool RequestScenarioChangeTransition(ResourceName mapResource, ResourceName worldSystemsConfigResourceName)
	{
		string mapPath = mapResource.GetPath();
		return GameStateTransitions.RequestScenarioChangeTransition(
			mapPath,
			worldSystemsConfigResourceName,
			addonList: "58D0FB3206B6F859"
		);
	}
}