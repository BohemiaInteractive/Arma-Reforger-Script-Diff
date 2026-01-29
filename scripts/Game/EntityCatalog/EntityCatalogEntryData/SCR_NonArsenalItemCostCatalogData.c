//!Base Used for items and attachments which should not be shown in the arsenal but should have a cost to them so any weapon in arsenal with that attachment will get a supply cost increase as well as being able to sell the item in the arsenal
[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Non-Arsenal item cost Data", "DISABLED - Arsenal Data", 1)]
class SCR_NonArsenalItemCostCatalogData : SCR_BaseEntityCatalogData
{	
	[Attribute("10", desc: "Supply cost of the item when the item is sold or is an attachment on an item that is in an arsenal", params: "0 inf 1")]
	protected int m_iSupplyCost;
	
	[Attribute(desc: "Depending on the settings of the arsenal component arsenal items can have an alternative supply cost. So it will take the cost of the alternative rather than the default cost. \n\nIf an Arsenal is not cost type default and the arsenal item does not have that cost type defined than it will still use the default cost.\n\nIf multiple entries have the same value than the last in the array will be used")]
	protected ref array<ref SCR_ArsenalAlternativeCostData> m_aArsenalAlternativeCostData;
	
	//~ Any attachements on the weapon or additional costs are saved for performance on cost calculation
	protected ref array<SCR_ArsenalItem> m_aAdditionalCosts;
	protected ref array<SCR_NonArsenalItemCostCatalogData> m_aNonArsenalAdditionalCosts;
	
	protected ref map<SCR_EArsenalSupplyCostType, int> m_mArsenalAlternativeCostData;
	
	//------------------------------------------------------------------------------------------------
	//! Get supply cost of item
	//! \param[in] supplyCostType What the supply cost is that should be obtained from the arsenal data. If the specific supply cost is not found then Default will be used instead
	//! \param[in] addAdditionalCosts Whether additional costs should be counted in supply cost or not. By default set to true.
	//! \return Supplycost
	int GetSupplyCost(SCR_EArsenalSupplyCostType supplyCostType, bool addAdditionalCosts = true)
	{
		int additionalCost;

		if (addAdditionalCosts)
		{
			//~ Get the cost of any attachments on the item that are not in arsenal but still have a cost
			if (m_aNonArsenalAdditionalCosts)
			{
				foreach (SCR_NonArsenalItemCostCatalogData data : m_aNonArsenalAdditionalCosts)
				{
					additionalCost += data.GetSupplyCost(supplyCostType);
				}
			}
			//~ Get the cost of any attachments on the item that can be in the arsenal
			if (m_aAdditionalCosts)
			{
				foreach (SCR_ArsenalItem data : m_aAdditionalCosts)
				{
					additionalCost += data.GetSupplyCost(supplyCostType);
				}
			}
		}

		if (supplyCostType != SCR_EArsenalSupplyCostType.DEFAULT && m_mArsenalAlternativeCostData != null)
		{
			int returnValue; 
			if (m_mArsenalAlternativeCostData.Find(supplyCostType, returnValue))
				return returnValue;
		}
		
		return m_iSupplyCost + additionalCost;
	}

	//------------------------------------------------------------------------------------------------
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{		
		//~ Save alternative costs in map and delete the array
		if (!m_aArsenalAlternativeCostData.IsEmpty())
		{
			m_mArsenalAlternativeCostData = new map<SCR_EArsenalSupplyCostType, int>();
		
			foreach (SCR_ArsenalAlternativeCostData data : m_aArsenalAlternativeCostData)
			{
				//~ Ignore default as that is m_iSupplyCost defined in the arsenal item
				if (data.m_eAlternativeCostType == SCR_EArsenalSupplyCostType.DEFAULT)
					continue;
				
				m_mArsenalAlternativeCostData.Insert(data.m_eAlternativeCostType, data.m_iSupplyCost);
			}
			
			//~ Delete array
			m_aArsenalAlternativeCostData = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets the calculated cost of the attachments and ammo
	override void PostInitData(notnull SCR_EntityCatalogEntry entry)
	{
		Resource itemResource = Resource.Load(entry.GetPrefab());
		if (!itemResource.IsValid())
			return;

		SCR_ArsenalItem.AddAdditionalCosts(entry, itemResource, m_aAdditionalCosts, m_aNonArsenalAdditionalCosts);
	}
}
