[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "SpawnerDataOverride", "DISABLED - SpawnerDataOverride", 1)]
class SCR_EntityCatalogSpawnerOverrideData : SCR_EntityCatalogSpawnerData
{
	[Attribute("-1", desc: "Override Minimum required rank to spawn entity. Set empty to have the EditableEntity component decide the rank", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eMinimumRequiredRankOverride;
	
	[Attribute("-1", desc: "Cost to spawn entity, Set -1 to have the EditableEntity component decide the supply cost with the CAMPAIGN budget")]
	protected int m_iSupplyCostOverride;
	
	//---------------------------------------------------------------------------------------------
	//~ Allows for overriding the cost and ranking that would otherwise be obtained from the EditableEntityComponent
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		super.InitData(entry);
		
		if (m_eMinimumRequiredRankOverride >= 0)
			m_eMinimumRequiredRank = m_eMinimumRequiredRankOverride;
		
		if (m_iSupplyCostOverride >= 0)
			m_iSupplyCost = m_iSupplyCostOverride;
		
		SCR_EditableEntityUIInfo uiInfo = SCR_EditableEntityUIInfo.Cast(entry.GetEntityUiInfo());
	}
}
