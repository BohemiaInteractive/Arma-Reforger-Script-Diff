//! Class for overriding commands on the map
class SCR_MapFactionCommanderRadialMenu : SCR_MapUIBaseComponent
{
	[Attribute(desc: "overriding Faction commander menu configs")]
	protected ref array<ref SCR_FactionCommanderMenuConfig> m_aFactionCommanderMenuConfigs;

	//------------------------------------------------------------------------------------------------
	//! \return commander menu config
	ResourceName GetCommanderMenuConfig(notnull Faction faction)
	{
		if (!m_aFactionCommanderMenuConfigs)
			return ResourceName.Empty;

		foreach (SCR_FactionCommanderMenuConfig config : m_aFactionCommanderMenuConfigs)
		{
			// if faction is not set, it means that config is for all factions
			if (config.GetFactionKey().IsEmpty() || config.GetFactionKey() == faction.GetFactionKey())
				return config.GetCommanderMenuConfig();
		}

		return ResourceName.Empty;
	}
}

[BaseContainerProps()]
class SCR_FactionCommanderMenuConfig
{
	[Attribute("", desc: "Faction, if faction is not set, it is used for all factions")]
	protected FactionKey m_sFaction;

	[Attribute("{B4F35F09884BF79A}Configs/FactionCommander/FactionCommanderMenuEntries.conf", desc: "Config can be set for every faction especially", params: "conf class=SCR_FactionCommanderMenuHierarchy")]
	protected ResourceName m_sCommanderMenuEntries;

	//------------------------------------------------------------------------------------------------
	//! \return faction key
	FactionKey GetFactionKey()
	{
		return m_sFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! \return commander menu config
	ResourceName GetCommanderMenuConfig()
	{
		return m_sCommanderMenuEntries;
	}
}
