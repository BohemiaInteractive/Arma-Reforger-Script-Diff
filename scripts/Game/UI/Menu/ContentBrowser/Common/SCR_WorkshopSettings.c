/*
Container of workshop settings. Also has static functions to manager storage of the settings.
*/

class SCR_WorkshopSettings : ModuleGameSettings
{
	[Attribute("true")]
	bool m_bAutoEnableDownloadedAddons;
	
	// Add more options here
	
	protected static const string USER_SETTINGS_MODULE = "SCR_WorkshopSettings";
	
	//------------------------------------------------------------------------------------------
	static SCR_WorkshopSettings Get()
	{
		SCR_WorkshopSettings settings = new SCR_WorkshopSettings;
		BaseContainer container = GetGame().GetGameUserSettings().GetModule("SCR_WorkshopSettings");
		BaseContainerTools.WriteToInstance(settings, container);
		return settings;
	}
	
	//------------------------------------------------------------------------------------------
	static void Save(SCR_WorkshopSettings settings)
	{
		BaseContainer container = GetGame().GetGameUserSettings().GetModule("SCR_WorkshopSettings");
		BaseContainerTools.ReadFromInstance(settings, container);
		GetGame().UserSettingsChanged();
	}
};