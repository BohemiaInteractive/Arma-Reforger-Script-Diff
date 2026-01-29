[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_BaseItemSupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
}

class SCR_BaseItemSupportStationComponent : SCR_BaseSupportStationComponent
{
	[Attribute(SCR_EArsenalSupplyCostType.DEFAULT.ToString(), desc: "Cost type of items. If it is not DEFAULT than it will try to get the diffrent supply cost if the item has it assigned" , uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSupplyCostType), category: "Resupply Support Station")]
	protected SCR_EArsenalSupplyCostType m_eSupplyCostType;
	
	[Attribute("1", desc: "Fallback item supply cost. If for some reason the item that was supplied had no cost or could not be found then the fallback cost is used",  category: "Resupply Support Station", params: "1 inf 1")]
	protected int m_iFallbackItemSupplyCost;
	
	protected SCR_EntityCatalogManagerComponent m_EntityCatalogManager;
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		m_EntityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		super.DelayedInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_EArsenalSupplyCostType GetArsenalSupplyCostType()
	{
		return m_eSupplyCostType;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool InitValidSetup()
	{
		if (!m_EntityCatalogManager)
		{
			Print("'SCR_BaseResupplySupportStationComponent' needs a entity catalog manager!", LogLevel.ERROR);
			return false;
		}
		
		return super.InitValidSetup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyAmountAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!AreSuppliesEnabled())
			return 0;
		
		SCR_BaseItemHolderSupportStationAction itemHolder = SCR_BaseItemHolderSupportStationAction.Cast(action);
		if (!itemHolder)
			return 0;
		
		SCR_EntityCatalogEntry catalogEntry = m_EntityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, itemHolder.GetItemPrefab(), GetFaction());
		if (!catalogEntry)
		{
			Print("'SCR_BaseResupplySupportStationComponent' could not find SCR_EntityCatalogEntry for '" + itemHolder.GetItemPrefab() + "' so will use fallback cost!", LogLevel.WARNING);
			return Math.ClampInt(m_iFallbackItemSupplyCost + m_iBaseSupplyCostOnUse, 0, int.MAX);
		}
		
		//~ Could not find arsenal data use fallback cost
		SCR_ArsenalItem arsenalData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!arsenalData)
		{
			Print("'SCR_BaseResupplySupportStationComponent' could not find SCR_ArsenalItem '" + itemHolder.GetItemPrefab() + "' so will use fallback cost!", LogLevel.WARNING);
			return Math.ClampInt(m_iFallbackItemSupplyCost + m_iBaseSupplyCostOnUse, 0, int.MAX);
		}
		
		return Math.ClampInt(arsenalData.GetSupplyCost(m_eSupplyCostType) + m_iBaseSupplyCostOnUse, 0, int.MAX);
	}
}