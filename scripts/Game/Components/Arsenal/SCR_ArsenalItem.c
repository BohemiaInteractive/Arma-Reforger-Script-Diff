[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
class SCR_ArsenalItem : SCR_BaseEntityCatalogData
{	
	[Attribute("2", desc: "Type of the arsenal item. An arsenal will only spawn items of types that it allows to be spawned. The item will not show up if it is not allowed. Eg: FieldDressing = HEAL", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemType))]
	protected SCR_EArsenalItemType m_eItemType;
	
	[Attribute("2", desc: "Item mode of arsenal, set this to what the behaviour is for the item. EG: FieldDressing = CONSUMABLE as it is used up or M16 with attachments = WEAPON_VARIENTS as it is not a default M16. Check other items in the faction config to see how it works.", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalItemMode))]
	protected SCR_EArsenalItemMode m_eItemMode;
	
	[Attribute(SCR_Enum.GetFlagValues(SCR_EArsenalGameModeType).ToString(), desc: "This value dictates in what (game)modes the arsenal item is available. All items with an Arsenal Data are always available if the Game Mode set in the ArsenalManager is Unrestricted", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EArsenalGameModeType))]
	protected SCR_EArsenalGameModeType m_eArsenalGameModeTypes;
	
	[Attribute("10", desc: "Supply cost of item when using the resupply action and/or taking them from the arsenal inventory. Note in overwrite arsenal config this value is ignored and still taken from catalog.\n\nAny weapon should have their base cost as the attachment cost will be calculated by the system on init and on refund (Supports Item mode WEAPON and WEAPON_VARIANTS only to save performance)", params: "0 inf 1")]
	protected int m_iSupplyCost;
	
	[Attribute(desc: "Display data for SCR_ArsenalDisplayComponent. If Arsenal item has display data of the correct type for the entity with SCR_ArsenalDisplayComponent then it can be displayed on said entity")]
	protected ref array<ref SCR_ArsenalItemDisplayData> m_aArsenalDisplayData;
	
	[Attribute(desc: "Depending on the settings of the arsenal component arsenal items can have an alternative supply cost. So it will take the cost of the alternative rather than the default cost. \n\nIf an Arsenal is not cost type default and the arsenal item does not have that cost type defined than it will still use the default cost.\n\nIf multiple entries have the same value than the last in the array will be used")]
	protected ref array<ref SCR_ArsenalAlternativeCostData> m_aArsenalAlternativeCostData;
	
	[Attribute(SCR_ECharacterRank.PRIVATE.ToString(), desc: "Player must meet or exceed this rank in order to purchase this item", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRequiredRank;
	
	[Attribute("1", desc: "When true, buying the item consumes player's military allocated supplies. The cost is equal to supply cost of the item.")]
	protected bool m_bUseMilitarySupplyAllocation;
	
	//~ Any attachements on the weapon or additional costs are saved for performance on cost calculation
	protected ref array<SCR_ArsenalItem> m_aAdditionalCosts;
	protected ref array<SCR_NonArsenalItemCostCatalogData> m_aNonArsenalAdditionalCosts;
	
	protected ref map<SCR_EArsenalSupplyCostType, ref SCR_ArsenalAlternativeCostData> m_mArsenalAlternativeCostData;
	
	protected SCR_EntityCatalogEntry m_EntryParent;
	protected ref Resource m_ItemResource;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemType GetItemType()
	{
		return m_eItemType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_EArsenalItemMode GetItemMode()
	{
		return m_eItemMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get the valid arsenal mode types
	SCR_EArsenalGameModeType GetArsenalGameModeTypes()
	{
		return m_eArsenalGameModeTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetItemResourceName()
	{
		if (!m_EntryParent)
			return string.Empty;	
		
		return m_EntryParent.GetPrefab();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Resource GetItemResource()
	{
		return m_ItemResource;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get display data of given type for displaying items on arsenal display
	//! \param[in] displayType type of display data to find
	//! \return Display data, can be null if not found
	SCR_ArsenalItemDisplayData GetDisplayDataOfType(EArsenalItemDisplayType displayType)
	{
		foreach (SCR_ArsenalItemDisplayData displayData : m_aArsenalDisplayData)
		{
			if (displayData.GetDisplayType() == displayType)
				return displayData;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Rank required of player in order to obtain the item from the arsenal. Only valid when Item's ranks are required
	SCR_ECharacterRank GetRequiredRank()
	{
		return m_eRequiredRank;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Whether item uses Military Supply Allocation or not
	bool GetUseMilitarySupplyAllocation()
	{
		return m_bUseMilitarySupplyAllocation;
	}

	//--------------------------------- Direct Getter general or any faction ---------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Get supply cost of arsenal item
	//! \param[in] supplyCostType What the supply cost is that should be obtained from the arsenal data. If the specific supply cost is not found then Default will be used instead
	//! \param[in] addAdditionalCosts Will add any additional cost to the item, in general these are weapon attachments. You want to turn this false if refunding an item as it should reevaluate the cost depending on the attachments
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
			SCR_ArsenalAlternativeCostData alternativeCost; 
			if (m_mArsenalAlternativeCostData.Find(supplyCostType, alternativeCost))
				return alternativeCost.m_iSupplyCost + additionalCost;
		}
		
		return m_iSupplyCost + additionalCost;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetSupplyRefundAmount(SCR_EArsenalSupplyCostType supplyCostType, bool addAdditionalCosts = true)
	{
		int additionalCost;
		
		if (addAdditionalCosts)
		{
			//~ Get the cost of any attachments on the item that are not in arsenal but still have a cost
			if (m_aNonArsenalAdditionalCosts)
			{
				foreach (SCR_NonArsenalItemCostCatalogData data : m_aNonArsenalAdditionalCosts)
				{
					additionalCost += data.GetSupplyCost(supplyCostType, false);
				}
			}
			//~ Get the cost of any attachments on the item that can be in the arsenal
			if (m_aAdditionalCosts)
			{
				foreach (SCR_ArsenalItem data : m_aAdditionalCosts)
				{
					additionalCost += data.GetSupplyRefundAmount(supplyCostType);
				}
			}
		}
		
		if (supplyCostType != SCR_EArsenalSupplyCostType.DEFAULT && m_mArsenalAlternativeCostData != null)
		{
			SCR_ArsenalAlternativeCostData alternativeCost; 
			if (m_mArsenalAlternativeCostData.Find(supplyCostType, alternativeCost))
				return alternativeCost.GetRefundAmount() + additionalCost;
		}
		
		return GetRefundAmountValue() + additionalCost;
	}

	//------------------------------------------------------------------------------------------------
	int GetMilitarySupplyAllocationCost(SCR_EArsenalSupplyCostType supplyCostType, bool addAdditionalCosts = true)
	{
		if (!m_bUseMilitarySupplyAllocation)
			return 0;

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
					if (!data.GetUseMilitarySupplyAllocation())
						continue;

					additionalCost += data.GetSupplyCost(supplyCostType);
				}
			}
		}

		if (supplyCostType != SCR_EArsenalSupplyCostType.DEFAULT && m_mArsenalAlternativeCostData != null)
		{
			SCR_ArsenalAlternativeCostData alternativeCost; 
			if (m_mArsenalAlternativeCostData.Find(supplyCostType, alternativeCost))
				return alternativeCost.m_iSupplyCost + additionalCost;
		}

		return m_iSupplyCost + additionalCost;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetRefundAmountValue()
	{
		return m_iSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitData(notnull SCR_EntityCatalogEntry entry)
	{
		m_EntryParent = entry;
		
		m_ItemResource = Resource.Load(m_EntryParent.GetPrefab());
		
		//~ Save alternative costs in map and delete the array
		if (!m_aArsenalAlternativeCostData.IsEmpty())
		{
			m_mArsenalAlternativeCostData = new map<SCR_EArsenalSupplyCostType, ref SCR_ArsenalAlternativeCostData>();
		
			foreach (SCR_ArsenalAlternativeCostData data : m_aArsenalAlternativeCostData)
			{
				//~ Ignore default as that is m_iSupplyCost defined in the arsenal item
				if (data.m_eAlternativeCostType == SCR_EArsenalSupplyCostType.DEFAULT)
					continue;
				
				m_mArsenalAlternativeCostData.Insert(data.m_eAlternativeCostType, data);
			}
			
			//~ Delete array
			m_aArsenalAlternativeCostData = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ Gets the calculated cost of the attachments if the item is a weapon
	override void PostInitData(notnull SCR_EntityCatalogEntry entry)
	{
		if (!m_ItemResource || !m_ItemResource.IsValid())
			return;
		
		//~ Get attachemts only from weapons to save performance
		if (m_eItemMode != SCR_EArsenalItemMode.WEAPON && m_eItemMode != SCR_EArsenalItemMode.WEAPON_VARIANTS && m_eItemMode != SCR_EArsenalItemMode.ATTACHMENT)
			return;
		
		AddAdditionalCosts(entry, m_ItemResource, m_aAdditionalCosts, m_aNonArsenalAdditionalCosts);
	}

	//------------------------------------------------------------------------------------------------
	//~ Gets the calculated cost of the attachments if the item is a weapon
	static void AddAdditionalCosts(notnull SCR_EntityCatalogEntry entry, Resource resource, inout array<SCR_ArsenalItem> arsenalCosts, inout array<SCR_NonArsenalItemCostCatalogData> nonArsenalCosts)
	{
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;

		SCR_EntityCatalog catalog = entry.GetCatalogParent();
		array<IEntityComponentSource> componentSources = {};
		array<SCR_BaseEntityCatalogData> entityDataList = {};

		SCR_ArsenalItem arsenalData;
		SCR_NonArsenalItemCostCatalogData nonArsenalData;
		
		//~ Get all attachments on the weapon
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(entitySource, AttachmentSlotComponent, true, componentSources) > 0)
		{
			SCR_EntityCatalogEntry attachmentEntry;
			ResourceName attachmentPrefab;
			bool attachmentEnabled;
			EntitySlotInfo slotInfo;
			BaseContainer attachSlot;

			foreach (IEntityComponentSource attachmentSource : componentSources)
			{
				attachSlot = attachmentSource.GetObject("AttachmentSlot");
				if (!attachSlot)
					continue;
				
				//~ Check if the slot is enabled
				attachSlot.Get("Enabled", attachmentEnabled);
				if (!attachmentEnabled)
					continue;
				
				//~ Get the prefab on the slot and see if it is not empty
				attachSlot.Get("Prefab", attachmentPrefab);
				if (attachmentPrefab.IsEmpty())
					continue;
				
				//~ Get the catalog entry with the given prefab
				attachmentEntry = catalog.GetEntryWithPrefab(attachmentPrefab);
				if (!attachmentEntry)
				{
					#ifdef WORKBENCH
					//~ Print if in workbench
					Print("Catalog Entry Arsenal Item: '" + WidgetManager.Translate(entry.GetEntityName()) + "' has an attachment which is not in the same catalog thus cannot get the supply cost of. Attachment: '" + attachmentPrefab + "'", LogLevel.VERBOSE);
					#endif
					continue;
				}
				
				bool foundData;
				
				attachmentEntry.GetEntityDataList(entityDataList);
				foreach (SCR_BaseEntityCatalogData data : entityDataList)
				{
					arsenalData = SCR_ArsenalItem.Cast(data);
					if (arsenalData)
					{
						if (!arsenalCosts)
							arsenalCosts = {};
						
						arsenalCosts.Insert(arsenalData);
						foundData = true;
						break;
					}
					
					nonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(data);
					if (nonArsenalData)
					{
						if (!nonArsenalCosts)
							nonArsenalCosts = {};
						
						nonArsenalCosts.Insert(nonArsenalData);
						foundData = true;
						break;
					}
				}
				
				if (!foundData)
					Print("Catalog Entry Arsenal Item: '" + WidgetManager.Translate(entry.GetEntityName()) + "' has an attachment which has no 'SCR_ArsenalItem' nor 'SCR_NonArsenalItemCostCatalogData' data in the catalog, thus it cannot get the supply cost from it. Attachment: '" + attachmentPrefab + "'", LogLevel.VERBOSE);
			}
		}

		//~ Get all ammunition in the weapon
		if (SCR_BaseContainerTools.FindComponentSourcesOfClass(entitySource, BaseMuzzleComponent, true, componentSources) > 0)
		{
			SCR_EntityCatalogEntry ammoEntry;
			ResourceName ammoPrefab;

			foreach (IEntityComponentSource muzzleSource : componentSources)
			{
				muzzleSource.Get("MagazineTemplate", ammoPrefab);
				if (ammoPrefab.IsEmpty())
				{
					muzzleSource.Get("AmmoTemplate", ammoPrefab);
					if (ammoPrefab.IsEmpty())
						continue;
				}
				
				//~ Get the catalog entry with the given prefab
				ammoEntry = catalog.GetEntryWithPrefab(ammoPrefab);
				if (!ammoEntry)
					continue;
				
				ammoEntry.GetEntityDataList(entityDataList);
				foreach (SCR_BaseEntityCatalogData data : entityDataList)
				{
					arsenalData = SCR_ArsenalItem.Cast(data);
					if (arsenalData)
					{
						if (!arsenalCosts)
							arsenalCosts = {};
						
						arsenalCosts.Insert(arsenalData);
						break;
					}
					
					nonArsenalData = SCR_NonArsenalItemCostCatalogData.Cast(data);
					if (nonArsenalData)
					{
						if (!nonArsenalCosts)
							nonArsenalCosts = {};
						
						nonArsenalCosts.Insert(nonArsenalData);
						break;
					}
				}
			}
		}
	}
}

[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iSupplyCost", "1", "%1 - Supply cost: %2")]
class SCR_ArsenalAlternativeCostData
{
	[Attribute(SCR_EArsenalSupplyCostType.GADGET_ARSENAL.ToString(), desc: "Cost type if system searches for the cost. Do not use DEFAULT as this will be ignored", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EArsenalSupplyCostType))]
	SCR_EArsenalSupplyCostType m_eAlternativeCostType;
	
	[Attribute("1", desc: "Alternative supply cost", params: "0 inf")]
	int m_iSupplyCost;
	
	//------------------------------------------------------------------------------------------------
	int GetRefundAmount()
	{
		return m_iSupplyCost;
	}
}

[BaseContainerProps(), BaseContainerCustomEnumWithValue(SCR_EArsenalSupplyCostType, "m_eAlternativeCostType", "m_iSupplyCost", "1", "%1 - Supply cost: %2")]
class SCR_ArsenalAlternativeCostSellAmountData : SCR_ArsenalAlternativeCostData
{
	[Attribute("1", desc: "Alternative supply refund amount. Supply refundmultiplier is still added to it", params: "0 inf")]
	protected int m_iSupplyRefundAmount;
	
	//------------------------------------------------------------------------------------------------
	override int GetRefundAmount()
	{
		return m_iSupplyRefundAmount;
	}
}
