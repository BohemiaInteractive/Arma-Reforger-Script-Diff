class SCR_GameLogHelper
{
	//------------------------------------------------------------------------------------------------
	static void LogScenariosConfPaths(int maxWorkshopScenarios = 500)
	{
		array<ResourceName> officialScenarioResourceNames = GetGame().GetDefaultGameConfigs();
		array<MissionWorkshopItem> missionWorkshopItems = {};

		WorkshopApi workshopAPI = GetGame().GetBackendApi().GetWorkshop();
		if (workshopAPI)
			workshopAPI.GetPageScenarios(missionWorkshopItems, 0, maxWorkshopScenarios);

		Print("--------------------------------------------------");
		if (!officialScenarioResourceNames)
		{
			Print("could not get official scenarios conf paths", LogLevel.WARNING);
		}
		else
		{
			Resource resource;
			BaseContainer container;
			SCR_MissionHeader missionHeader;
			string missionName;
			PrintFormat("Official scenarios (%1 entries)", officialScenarioResourceNames.Count());
			Print("--------------------------------------------------");
			foreach (ResourceName officialScenarioResourceName : officialScenarioResourceNames)
			{
				missionHeader = SCR_ConfigHelperT<SCR_MissionHeader>.GetConfigObject(officialScenarioResourceName);
				if (missionHeader)
				{
					missionName = missionHeader.m_sName;
				}
				else
				{
					missionName = "n/a";
					resource = Resource.Load(officialScenarioResourceName);
					if (resource.IsValid())
					{
						container = resource.GetResource().ToBaseContainer();
						if (container)
							container.Get("m_sName", missionName);
					}
				}

				if (missionName.Contains("#"))
					missionName = WidgetManager.Translate(missionName);

				PrintFormat("%1 (%2)", officialScenarioResourceName, missionName);
			}
		}

		Print("--------------------------------------------------");

		if (!workshopAPI)
		{
			Print("could not get Workshop API to read mod scenarios conf paths", LogLevel.WARNING);
			Print("--------------------------------------------------");
			return;
		}

		int workshopCount = missionWorkshopItems.Count() - officialScenarioResourceNames.Count();
		if (workshopCount < 1)
			return;

		PrintFormat("Workshop scenarios (%1 entries)", workshopCount);
		Print("--------------------------------------------------");
		foreach (MissionWorkshopItem mission : missionWorkshopItems)
		{
			// mods cannot be loaded here, so ResourceName will do
			if (mission.GetOwner())
				Print(mission.Id());
		}
		Print("--------------------------------------------------");
	}
};
