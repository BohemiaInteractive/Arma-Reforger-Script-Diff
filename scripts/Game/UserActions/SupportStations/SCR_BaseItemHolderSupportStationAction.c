[BaseContainerProps(), BaseContainerCustomStringTitleField("USE INHERITED VERSION ONLY!")]
class SCR_BaseItemHolderSupportStationAction : SCR_BaseAudioSupportStationAction
{
	[Attribute(desc: "This decides what items will be resupplied when using the action")]
	protected ref SCR_ResupplySupportStationData m_ResupplyData;
	
	[Attribute("#AR-SupportStation_Resupply_ActionInvalid_InvalidNotInStorage", desc: "Text shown on action if player cannot attach the pylon as the storage has no valid pylons", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sInvalidItemNotInStorage;
	
	protected ResourceName m_sItemPrefab;
	
	protected string m_sItemName = "MISSING NAME OR INVALID PREFAB";
	
	//------------------------------------------------------------------------------------------------
	protected override LocalizedString GetInvalidPerformReasonString(ESupportStationReasonInvalid reasonInvalid)
	{
		if (reasonInvalid == ESupportStationReasonInvalid.RESUPPLY_NOT_IN_STORAGE)
			return m_sInvalidItemNotInStorage;
		
		return super.GetInvalidPerformReasonString(reasonInvalid);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		if (m_ResupplyData)
			m_ResupplyData.Init(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetItemName()
	{
		Resource resource = Resource.Load(m_sItemPrefab);
		if (!resource || !resource.IsValid())
			return;
		
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;
		
		UIInfo uiInfo;
		
		//~ Get UI info from item
		IEntityComponentSource inventoryEntitySource = SCR_ComponentHelper.GetInventoryItemComponentSource(entitySource);
		if (inventoryEntitySource)
		{
			SCR_ItemAttributeCollection inventoryEntityUiInfo = SCR_ComponentHelper.GetInventoryItemInfo(inventoryEntitySource);
			if (inventoryEntityUiInfo)
			{
				uiInfo = inventoryEntityUiInfo.GetUIInfo();
				
				if (uiInfo && !uiInfo.GetName().IsEmpty())
				{
					m_sItemName = uiInfo.GetName();
					return;
				}
			}
		}
		
		//~ Get UI info from weapon
		IEntityComponentSource weaponEntitySource = SCR_ComponentHelper.GetWeaponComponentSource(entitySource);
		if (weaponEntitySource)
		{
			uiInfo = SCR_ComponentHelper.GetWeaponComponentInfo(weaponEntitySource);
			
			if (uiInfo && !uiInfo.GetName().IsEmpty())
			{
				m_sItemName = uiInfo.GetName();
				return;
			}
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	/*bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		return m_ResupplyData.GetResupplyItemOrMuzzle(targetCharacter, actionOwner, supportStationGadget, item, muzzle);
	}*/
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetItemPrefab()
	{
		return m_sItemPrefab;
	}
}

[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE!")]
class SCR_ResupplySupportStationData
{
	//------------------------------------------------------------------------------------------------
	void Init(IEntity owner);
	
	//------------------------------------------------------------------------------------------------
	bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected FactionAffiliationComponent GetFactionAffiliationGadgetOrOwner(notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget)
	{
		FactionAffiliationComponent factionAffiliationComponent;
		
		if (supportStationGadget)
		{
			factionAffiliationComponent = FactionAffiliationComponent.Cast(supportStationGadget.GetOwner().FindComponent(FactionAffiliationComponent));
			if (factionAffiliationComponent)
				return factionAffiliationComponent;
		}
			
		factionAffiliationComponent = FactionAffiliationComponent.Cast(actionOwner.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
		{
			IEntity parent = actionOwner.GetParent();
			if (parent)
				factionAffiliationComponent = FactionAffiliationComponent.Cast(parent.FindComponent(FactionAffiliationComponent));
		}
		
		return factionAffiliationComponent;
	}
}

[BaseContainerProps()]
class SCR_ResupplyItemSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(desc: "Prefab of entity", UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sItemPrefab;
	
	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		item = m_sItemPrefab;
		
		return !SCR_StringHelper.IsEmptyOrWhiteSpace(item);
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetItemPrefab()
	{
		return m_sItemPrefab;
	}
}

[BaseContainerProps()]
class SCR_ResupplyCatalogItemSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(desc: "This will go through all the inventory items in the catalog and grab the first one with the given type (needs the SCR_EntityCatalogSupportStationResupplyData)", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_ESupportStationResupplyType))]
	protected SCR_ESupportStationResupplyType m_eResupplyType;
	
	[Attribute("0", desc: "If true will use the item associated with the support station default faction. Otherwise will only take default if no faction is set")]
	protected bool m_bAlwaysTakeDefaultFaction;
	
	protected ref map<FactionKey, ref SCR_EntityCatalogEntry> m_mFactionItems = new map<FactionKey, ref SCR_EntityCatalogEntry>();
		
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity owner)
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		
		SCR_Faction scrFaction;
		SCR_EntityCatalog itemCatalog;
		
		array<SCR_EntityCatalogEntry> filteredEntityList = {};
		array<SCR_BaseEntityCatalogData> dataList = {};
		SCR_EntityCatalogSupportStationResupplyData resupplyData;
		int count;
		
		//~ Go over each faction and set which items will be used for the resupply
		foreach (Faction faction : factions)
		{
			scrFaction = SCR_Faction.Cast(faction);
			if (!scrFaction)
				continue;
			
			itemCatalog = scrFaction.GetFactionEntityCatalogOfType(EEntityCatalogType.ITEM);
			if (!itemCatalog)
				continue;
			
			count = itemCatalog.GetEntityListWithData(SCR_EntityCatalogSupportStationResupplyData, filteredEntityList, dataList);
			if (count <= 0)
				continue;
			
			for (int i = 0; i < count; i++)
			{
				resupplyData = SCR_EntityCatalogSupportStationResupplyData.Cast(dataList[i]);
				if (!resupplyData || resupplyData.GetResupplyType() != m_eResupplyType)
					continue;
				
				m_mFactionItems.Insert(faction.GetFactionKey(), filteredEntityList[i]);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetRequiredRank(notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget)
	{
		SCR_ECharacterRank requiredRank = SCR_ECharacterRank.PRIVATE;

		if (m_mFactionItems.IsEmpty())
			return requiredRank;

		FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
		if (!factionAffiliationComponent)
			return false;

		Faction faction;
		if (!m_bAlwaysTakeDefaultFaction)
			faction = factionAffiliationComponent.GetAffiliatedFaction();

		if (!faction)
			faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();

		if (!faction)
			return requiredRank;

		SCR_EntityCatalogEntry catalogEntry;
		if (!m_mFactionItems.Find(faction.GetFactionKey(), catalogEntry) || !catalogEntry)
			return requiredRank;

		SCR_ArsenalItem itemData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (itemData)
			requiredRank = itemData.GetRequiredRank();

		return requiredRank;
	}

	//------------------------------------------------------------------------------------------------
	int GetRequiredAvailableAllocatedSupplies(notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget)
	{
		if (m_mFactionItems.IsEmpty())
			return 0;

		FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
		if (!factionAffiliationComponent)
			return 0;

		if (m_bAlwaysTakeDefaultFaction)
			return 0;

		Faction faction = factionAffiliationComponent.GetAffiliatedFaction();
		if (!faction)
			return 0;

		SCR_EntityCatalogEntry catalogEntry;
		if (!m_mFactionItems.Find(faction.GetFactionKey(), catalogEntry) || !catalogEntry)
			return 0;

		SCR_ArsenalItem itemData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (itemData && itemData.GetUseMilitarySupplyAllocation())
			return itemData.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		if (m_mFactionItems.IsEmpty())
			return false;
		
		FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
		if (!factionAffiliationComponent)
			return false;
		
		//~ Get faction
		Faction faction;
		if (m_bAlwaysTakeDefaultFaction)
		{
			faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
			if (!faction)
				return false;
		}
		else 
		{
			faction = factionAffiliationComponent.GetAffiliatedFaction();
			if (!faction)
			{
				faction = factionAffiliationComponent.GetDefaultAffiliatedFaction();
				if (!faction)
					return false;
			}
		}	

		SCR_EntityCatalogEntry catalogEntry;
		if (!m_mFactionItems.Find(faction.GetFactionKey(), catalogEntry) || !catalogEntry)
			return false;

		item = catalogEntry.GetPrefab();
		return !SCR_StringHelper.IsEmptyOrWhiteSpace(item);
	}
}

[BaseContainerProps()]
class SCR_ResupplyHeldWeaponSupportStationData : SCR_ResupplySupportStationData
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMuzzleType))]
	protected ref array<EMuzzleType> m_aMuzzleTypes;
	
	[Attribute("0", desc: "If true will check if the found ammo is part of the entity default faction or owner default faction. Only do this check when item has no inventory, else check if item is in storage instead!")]
	protected bool m_bCheckFaction;
	
	//------------------------------------------------------------------------------------------------
	override bool GetResupplyItemOrMuzzle(notnull IEntity targetCharacter, notnull IEntity actionOwner, SCR_SupportStationGadgetComponent supportStationGadget, out ResourceName item, out BaseMuzzleComponent muzzle)
	{
		if (m_aMuzzleTypes.IsEmpty())
			return false;
		
		BaseWeaponManagerComponent weaponsManager = BaseWeaponManagerComponent.Cast(targetCharacter.FindComponent(BaseWeaponManagerComponent));
		if (!weaponsManager)
			return false;		
		
		//~ Get held weapon to resupply
		BaseWeaponComponent baseWeaponComp = weaponsManager.GetCurrentWeapon();
		if (!baseWeaponComp)
			return false;
		
		string weaponSlotType = baseWeaponComp.GetWeaponSlotType();
		if (weaponSlotType != "primary" && weaponSlotType != "secondary")
			return false;

		array<BaseMuzzleComponent> muzzles = {};

		//~ Get base muzzle to only supply magazines
		baseWeaponComp.GetMuzzlesList(muzzles);
		SCR_MuzzleInMagComponent inMagMuzzle;
		
		foreach (BaseMuzzleComponent muzzleComp : muzzles)
		{
			if (m_aMuzzleTypes.Contains(muzzleComp.GetMuzzleType()))
			{
				//~ Cannot be reloaded
				inMagMuzzle = SCR_MuzzleInMagComponent.Cast(muzzleComp);
				if (inMagMuzzle && !inMagMuzzle.CanBeReloaded())
					continue;
				
				//~ Check if has default magazine
				item = muzzleComp.GetDefaultMagazineOrProjectileName();
				if (!item.IsEmpty())
				{
					muzzle = muzzleComp;
					break;
				}
			}
		}
		
		//~ No valid muzzle found
		if (!muzzle)
			return false;
		
		//~ Check if ammo is part of the faction of gadget or action owner
		if (m_bCheckFaction)
		{
			SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
			if (!catalogManager)
				return true;
			
			FactionAffiliationComponent factionAffiliationComponent = GetFactionAffiliationGadgetOrOwner(actionOwner, supportStationGadget);
			if (!factionAffiliationComponent)
				return true;
			
			SCR_Faction faction = SCR_Faction.Cast(factionAffiliationComponent.GetDefaultAffiliatedFaction());
			if (!faction)
				return true;
			
			//~ Not the correct faction so do not allow for resupply
			if (!catalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, item, faction))
				return false;
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	int GetRequiredAvailableAllocatedSupplies(notnull IEntity actionOwner, ResourceName resourceName)
	{
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;

		SCR_EntityCatalogEntry catalogEntry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, resourceName);
		if (!catalogEntry)
			return 0;

		SCR_ArsenalItem arsenalItemData = SCR_ArsenalItem.Cast(catalogEntry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!arsenalItemData)
			return 0;

		if (!arsenalItemData.GetUseMilitarySupplyAllocation())
			return 0;

		return arsenalItemData.GetSupplyCost(SCR_EArsenalSupplyCostType.GADGET_ARSENAL, false);
	}
}