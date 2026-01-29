/*
The faction alias component allows using a mission specific alias for a side when placing down unit spawning slots.
The mission creator or admin playing the mission can quickly change which factions each alias represents.
*/

[ComponentEditorProps(category: "GameScripted/FactionManager/Components", description: "Base for FactionManager scripted component.")]
class SCR_FactionAliasComponentClass : ScriptComponentClass
{
}

class SCR_FactionAliasComponent : ScriptComponent
{
	[Attribute(desc: "Global faction aliases to allow quick switching of factions for slots.")]
	ref array<ref SCR_ScenarioFrameworkFactionAliasConfig> m_aFactionAliases;

	ref map<FactionKey, ref SCR_ScenarioFrameworkFactionAliasConfig> m_mAliasToFactionKeyMap = new map<FactionKey, ref SCR_ScenarioFrameworkFactionAliasConfig>();

	static SCR_FactionAliasComponent m_FactionAliasComponentInstanceForWB;

	
	//------------------------------------------------------------------------------------------------
	//! \param[in] owner The owner represents the entity that initiated the OnPostInit event, which is being passed as an argument for this method to perform
	override void OnPostInit(IEntity owner)
	{
		InitAliasMap();
	}

	
	//------------------------------------------------------------------------------------------------
	//! Initializes alias map from both component instance and overriden by the mission header.
	void InitAliasMap()
	{
		m_mAliasToFactionKeyMap.Clear();
		foreach (SCR_ScenarioFrameworkFactionAliasConfig aliasConfig : m_aFactionAliases)
		{
			m_mAliasToFactionKeyMap.Set(aliasConfig.m_sAlias, aliasConfig);
		}

		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (!header)
			return;
		
		SCR_MissionHeaderScenarioFramework scenarioFrameworkHeader = SCR_MissionHeaderScenarioFramework.Cast(header);
		if (!scenarioFrameworkHeader)
			return;
		
		foreach (SCR_ScenarioFrameworkFactionAliasConfig aliasConfig : scenarioFrameworkHeader.m_aFactionAliases)
		{
			m_mAliasToFactionKeyMap.Set(aliasConfig.m_sAlias, aliasConfig);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return a copy of the Faction alias map configuration.
	map<FactionKey, ref SCR_ScenarioFrameworkFactionAliasConfig> GetFactionAliasMap()
	{
		return map<FactionKey, ref SCR_ScenarioFrameworkFactionAliasConfig>.Cast(m_mAliasToFactionKeyMap.Clone());
	}

	//------------------------------------------------------------------------------------------------
	//! Resolves input alias for a faction key, returning original input if no alias found.
	//! \param[in] inputFactionKey Resolves alias for input faction key.
	//! \return the alias for the input facton key if it exists, otherwise returns the input facton key itself.
	FactionKey ResolveFactionAlias(FactionKey inputFactionKey)
	{
		SCR_ScenarioFrameworkFactionAliasConfig aliasConfig;
		if (m_mAliasToFactionKeyMap.Find(inputFactionKey, aliasConfig))
			return aliasConfig.m_sFactionKey;
		else
			return inputFactionKey;
	}
	
	//------------------------------------------------------------------------------------------------
	array<string> GetAllFactionKeyAliases(FactionKey inputFactionKey)
	{
		array<ref SCR_ScenarioFrameworkFactionAliasConfig> aliasConfigs = SCR_MapHelperT<FactionKey, ref SCR_ScenarioFrameworkFactionAliasConfig>.GetElements(m_mAliasToFactionKeyMap);
		array<string> aliases = {};
		
		foreach (SCR_ScenarioFrameworkFactionAliasConfig config : aliasConfigs)
		{
			if (config.m_sFactionKey == inputFactionKey)
				aliases.Insert(config.m_sAlias);
		}
		
		return aliases;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes the facton alias component instance for Workbench object previews.
	//! \param[in] owner The owner represents the entity that initializes the script.
	//! \param[in,out] mat Mat is a 4x4 matrix representing the world space transformation for the entity.
	//! \param[in] src Source entity providing initialization data for the component.
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		m_FactionAliasComponentInstanceForWB = this;
	}

	//------------------------------------------------------------------------------------------------
	//! Only use for object previews at WorkBench time.
	//! In game time, use `GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent)`
	//! \return Faction alias component for Worldbench object previews.
	static SCR_FactionAliasComponent GetFactionAliasComponentForWB()
	{
#ifndef WORKBENCH
		Print("ScenarioFramework [SCR_FactionAliasComponent] GetFactionAliasComponentForWB called outside of WORKBENCH. In game time, use `GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent)`", LogLevel.WARNING);
#endif
		return m_FactionAliasComponentInstanceForWB;
	}
}
