class SCR_MissionHeader : MissionHeader
{
	[Attribute("", UIWidgets.EditBox, "Name of this mission.")]
	string m_sName;

	[Attribute("", UIWidgets.EditBox, "Author of this mission.")]
	string m_sAuthor;

	[Attribute("", UIWidgets.EditBox, "Path to mission.")]
	string m_sPath;

	[Attribute("", UIWidgets.EditBox, "Brief description of this mission's purpose.")]
	string m_sDescription;

	[Attribute("", UIWidgets.EditBox, "Detailed description of this mission (i.e. rules).")]
	string m_sDetails;

	[Attribute("{6CFADAEE9287D1D2}UI/Textures/WorldSelection/Default.edds", UIWidgets.ResourcePickerThumbnail, "Icon texture of this mission visible e.g. in menus.", "edds")]
	ResourceName m_sIcon;

	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourcePickerThumbnail, "Texture of this mission visible when loading this mission.", "edds")]
	ResourceName m_sLoadingScreen;

	[Attribute("{C58FCC06AF13075B}UI/Textures/MissionLoadingScreens/placeholder_1.edds", UIWidgets.ResourcePickerThumbnail, "Texture of this mission visible when loading this mission.", "edds")]
	ResourceName m_sPreviewImage;

	[Attribute("Sandbox", UIWidgets.EditBox, "Game mode of this mission.")]
	string m_sGameMode;

	[Attribute("1", UIWidgets.EditBox, "The count of players for this mission")]
	int m_iPlayerCount;

	[Attribute("7", UIWidgets.Flags, "If all save types are disabled, the entire persistence system is disabled", enumType: ESaveGameType)]
	ESaveGameType m_eSaveTypes;

	[Attribute("0", uiwidget: UIWidgets.Flags, "Editable Game Flags", "", ParamEnumArray.FromEnum(EGameFlags))]
	EGameFlags m_eEditableGameFlags;

	[Attribute("0", uiwidget: UIWidgets.Flags, "Default Game Flags", "", ParamEnumArray.FromEnum(EGameFlags))]
	EGameFlags m_eDefaultGameFlags;
	
	[Attribute("1", desc: "When true, this scenario will be listed in the scenario menu")]
	bool m_bShowInScenarioMenu;

	[Attribute("", UIWidgets.ResourceNamePicker, "Configuration file for briefing screen.", "conf")]
	ResourceName m_sBriefingConfig;
	
	[Attribute("0", desc: "If the scenario allows it, its daytime and weather will use values from this header. Requires SCR_TimeAndWeatherHandlerComponent on gamemode entity.")];
	bool m_bOverrideScenarioTimeAndWeather;
	
	[Attribute("8", UIWidgets.Slider, "Starting time of day (hours)", "0 23 1")]
	int m_iStartingHours;
	
	[Attribute("0", UIWidgets.Slider, "Starting time of day (minutes)", "0 59 1")]
	int m_iStartingMinutes;
	
	[Attribute("0")]
	bool m_bRandomStartingDaytime;
	
	[Attribute("1", UIWidgets.Slider, "Time acceleration during the day (1 = 100%, 2 = 200% etc)", "0.1 12 0.1")]
	float m_fDayTimeAcceleration;
	
	[Attribute("1", UIWidgets.Slider, "Time acceleration during the night (1 = 100%, 2 = 200% etc)", "0.1 12 0.1")]
	float m_fNightTimeAcceleration;
	
	[Attribute("0")]
	bool m_bRandomStartingWeather;
	
	[Attribute("0", desc: "Weather can change during gameplay")];
	bool m_bRandomWeatherChanges;
	
	[Attribute("1", UIWidgets.EditBox, "Player XP multiplier (when enabled in gamemode; 1 for default)")]
	float m_fXpMultiplier;
	
	[Attribute("", desc: "Determines whether map markers can be deleted only by player who placed them or by anyone within faction")]
	bool m_bMapMarkerEnableDeleteByAnyone;
	
	[Attribute("10", desc: "How many map markers per player can exist at a time")];
	int m_iMapMarkerLimitPerPlayer;
	
	[Attribute("", UIWidgets.Object, "Player limits per faction")]
	ref array<ref SCR_FactionLimit> m_aFactionLimits;
	
	[Attribute("0", desc: "Armavision is allowed for players without elevated rights in multiplayer")];
	bool m_bIsArmavisionAllowedInMP;
	
	bool m_bLoadOnStart;
	string m_sOwner;

	//------------------------------------------------------------------------------------------------
	//! Returns whether mission can be played in multiplayer or not.
	bool IsMultiplayer()
	{
		return m_iPlayerCount > 1;
	}
	
	/*!
	Get mission header from a MissionWorkshopItem.
	\return File name
	*/
	static SCR_MissionHeader GetMissionHeader(notnull MissionWorkshopItem item, bool blockFromAddons = false)
	{
		// Terminate if scenario is from addon and scenarios from addons are marked to be blocked.
		// Make use in situations like MainMenu, where MissionHeaders in addons are not accessible.
		if (blockFromAddons && item.GetOwner())
			return null;
		
		return SCR_MissionHeader.Cast(MissionHeader.ReadMissionHeader(item.Id()));
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Returns map of player limits per faction.
	map<string, int> GetFactionLimitMap()
	{
		map<string, int> missionFactionLimitMap = new map<string, int>();
		if (!m_aFactionLimits.IsEmpty())
		{
			foreach (SCR_FactionLimit factionLimit : m_aFactionLimits)
			{
				Print(factionLimit.m_sFactionKey);
				if (factionLimit && !factionLimit.m_sFactionKey.IsEmpty())
					missionFactionLimitMap.Insert(factionLimit.m_sFactionKey, factionLimit.m_iFactionLimit);
			}
		}
		return missionFactionLimitMap;
	}
};
