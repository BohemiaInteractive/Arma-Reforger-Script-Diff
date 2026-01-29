//------------------------------------------------------------------------------------------------
enum ESettingManagerModuleType
{
	SETTINGS_MANAGER_INVALID,
	SETTINGS_MANAGER_KEYBINDING,
	SETTINGS_MANAGER_VIDEO
};

//------------------------------------------------------------------------------------------------
class SCR_SettingsManager
{
	protected ref array<ref SCR_SettingsManagerModuleBase> m_aModules = {};
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManager()
	{
		//todo: rework this to load out of config so we can just have config with all the modules
		SCR_SettingsManagerVideoModule videoModule = new SCR_SettingsManagerVideoModule();
		if (videoModule)
			AddModule(videoModule);
		
		SCR_SettingsManagerKeybindModule keybindModule = new SCR_SettingsManagerKeybindModule();
		if (keybindModule)
			AddModule(keybindModule);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_SettingsManagerModuleBase GetModule(ESettingManagerModuleType moduleType)
	{
		foreach(SCR_SettingsManagerModuleBase module : m_aModules)
		{
			if (module.GetModuleType() == moduleType)
				return module;
		}
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! inserts module into the settingsManager in case that module is not already present
	void AddModule(notnull SCR_SettingsManagerModuleBase module)
	{
		if (!GetModule(module.GetModuleType()))
			m_aModules.Insert(module);
	}
}

//------------------------------------------------------------------------------------------------
class SCR_SettingsManagerModuleBase
{
	protected ESettingManagerModuleType m_eModuleType;
	
	//------------------------------------------------------------------------------------------------
	ESettingManagerModuleType GetModuleType()
	{
		return m_eModuleType;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetModuleType(ESettingManagerModuleType managerType)
	{
		m_eModuleType = managerType;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_SettingsManagerModuleBase()
	{
		
	}
}