[EntityEditorProps(category: "GameScripted/GameMode/Components", description: "Component for map config selection")]
class SCR_MapConfigComponentClass : ScriptComponentClass
{
}

//! Attached to BaseGameMode, used for map config selection
class SCR_MapConfigComponent : ScriptComponent
{
	[Attribute("{1B8AC767E06A0ACD}Configs/Map/MapFullscreen.conf", UIWidgets.ResourceNamePicker, desc: "Gadget map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sGadgetMapConfigPath;
	
	[Attribute("{418989FA279F1257}Configs/Map/MapSpawnMenu.conf", UIWidgets.ResourceNamePicker, desc: "Spawn screen map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sSpawnMapConfigPath;
	
	[Attribute("{19C76194B21EC3E1}Configs/Map/MapEditor.conf", UIWidgets.ResourceNamePicker, desc: "Game master map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sEditorMapConfigPath;
	
	[Attribute("{118AAD2C9ECADB7A}Configs/Map/MapCombatSupportStation.conf", UIWidgets.ResourceNamePicker, desc: "Combat support map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sCombatSupportStationMapConfigPath;

	[Attribute("{D3D77C237D240371}Configs/Map/MapOperationsStation.conf", UIWidgets.ResourceNamePicker, desc: "Operations station map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sOperationsStationMapConfigPath;

	[Attribute("{8C31CBE04E80BF5B}Configs/Map/MapLogisticsStation.conf", UIWidgets.ResourceNamePicker, desc: "Logistics station map config", "conf class=SCR_MapConfig")]
	protected ResourceName m_sLogisticsStationMapConfigPath;

	protected EMapEntityMode m_eCommandPostMapEntityMode;

	//------------------------------------------------------------------------------------------------
	//! Get config of the ingame gadget map
	ResourceName GetGadgetMapConfig()
	{
		return m_sGadgetMapConfigPath;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get config of the respawn screen map
	ResourceName GetSpawnMapConfig()
	{
		return m_sSpawnMapConfigPath;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get config of the game master map
	ResourceName GetEditorMapConfig()
	{
		return m_sEditorMapConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCombatSupportStationMapConfig()
	{
		return m_sCombatSupportStationMapConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetOperationsStationMapConfig()
	{
		return m_sOperationsStationMapConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetLogisticsStationMapConfig()
	{
		return m_sLogisticsStationMapConfigPath;
	}

	//------------------------------------------------------------------------------------------------
	//! Get map entity mode for command posts
	EMapEntityMode GetCommandPostMapEntityMode()
	{
		return m_eCommandPostMapEntityMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Set map entity mode for command posts
	void SetCommandPostMapEntityMode(EMapEntityMode mode)
	{
		m_eCommandPostMapEntityMode = mode;
	}
}
