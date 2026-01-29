
/**
Info for Player Loadout
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "LoadoutData", "DISABLED - LoadoutData", 1)]
class SCR_EntityCatalogLoadoutData: SCR_BaseEntityCatalogData
{
	[Attribute("50", desc: "Supply cost for players to spawn with this loadout. Cost will be multiplied with Arsenal Spawn cost multiplier. The system will get the campaign cost if no SCR_EntityCatalogLoadoutData is assigned", params: "0, inf")]
	protected float m_fSpawnSupplyCost;

	[Attribute(defvalue: SCR_ECharacterRank.PRIVATE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Rank required to spawn with the loadout", params: "", enumType: SCR_ECharacterRank)]
	protected SCR_ECharacterRank m_eRequiredRank;

	//------------------------------------------------------------------------------------------------
	//! \return Get loadout spawn cost assigned to this loadout
	float GetLoadoutSpawnCost()
	{
		return m_fSpawnSupplyCost;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetRequiredRank()
	{
		return m_eRequiredRank;
	}

	//------------------------------------------------------------------------------------------------
	void SetRequiredRank(SCR_ECharacterRank rank)
	{
		m_eRequiredRank = rank;
	}
};