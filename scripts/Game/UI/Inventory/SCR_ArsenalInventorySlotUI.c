//------------------------------------------------------------------------------------------------
class SCR_ArsenalInventorySlotUI : SCR_InventorySlotUI
{
	static const string SLOT_LAYOUT_SUPPLY = "{AF25F325D2730142}UI/layouts/Menus/Inventory/SupplyInventoryItemSlot.layout";
	
	protected const string RESOURCES_TEXT_WIDGET_NAME = "SuppliesText";
	protected const string STORED_RESOURCES_WIDGET_NAME = "SuppliesStored";
	protected const string AVAILABLE_RESOURCES_WIDGET_NAME = "SuppliesAvailable";
	protected const string COST_RESOURCES_WIDGET_NAME = "SuppliesCost";
	protected const string RANK_ICON_WIDGET_NAME = "RankIcon";
	protected const string MSAR_ICON_WIDGET_NAME = "MSARIcon";
	protected const string AMMOTYPE_WIDGET_NAME = "AmmoTypeContainer";
	
	protected Widget m_CostResourceHolder;
	protected TextWidget m_CostResourceHolderText;
	
	protected bool m_bIsAvailable = true;
	protected TextWidget m_wResourceText;
	protected Widget m_wResourceDisplay;
	
	protected SCR_ResourceComponent m_ArsenalResourceComponent;
	protected SCR_ResourceContainer m_ResourceContainer;
	
	protected float m_fSupplyCost;
	protected SCR_ECharacterRank m_eRequiredRank;	
	protected ImageWidget m_wMSARIcon;
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceComponent GetArsenalResourceComponent()
	{
		return m_ArsenalResourceComponent;
	}
	
	void SetArsenalResourceComponent(SCR_ResourceComponent component)
	{
		m_ArsenalResourceComponent = component;
	}
	
    //------------------------------------------------------------------------------------------------
	void SetStorageUI(SCR_InventoryStorageBaseUI storageUI)
	{
		m_pStorageUI = storageUI;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected string SetSlotSize()
	{
		string slotLayout = SLOT_LAYOUT_SUPPLY;
        
		switch ( m_Attributes.GetItemSize() ) 
		{
			case ESlotSize.SLOT_1x1:
                m_iSizeX = 1;
                m_iSizeY = 1;

                break;
			case ESlotSize.SLOT_2x1:
                m_iSizeX = 2;
                m_iSizeY = 1;

                break;
			case ESlotSize.SLOT_2x2:
                m_iSizeX = 2; 
                m_iSizeY = 2;
                
                break;
			case ESlotSize.SLOT_3x3:
                m_iSizeX = 3;
                m_iSizeY = 3;
                
                break;
		}

		return slotLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);
		
		if (!m_wDimmerEffect)
			m_wDimmerEffect = m_widget.FindAnyWidget("Dimmer");
		
		if (!m_pItem || !m_pItem.GetOwner())
			return;
		
		//~ Never show Stored resources
		Widget resourceStored = m_widget.FindAnyWidget(STORED_RESOURCES_WIDGET_NAME);
		if (resourceStored)
			resourceStored.SetVisible(false);
		
		//~ Never show Available resources
		Widget resourceAvailable = m_widget.FindAnyWidget(AVAILABLE_RESOURCES_WIDGET_NAME);
		if (resourceAvailable)
			resourceAvailable.SetVisible(false);
		
		//~ Get cost widget
		m_CostResourceHolder = m_widget.FindAnyWidget(COST_RESOURCES_WIDGET_NAME);
		if (m_CostResourceHolder)
			m_CostResourceHolderText = TextWidget.Cast(m_CostResourceHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		if (!m_CostResourceHolder || !m_CostResourceHolderText)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(m_pItem.GetOwner());
		SCR_ResourceContainer resourceContainer;
		
		if (resourceComponent)
			resourceContainer = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		
		m_wMSARIcon = ImageWidget.Cast(m_widget.FindAnyWidget(MSAR_ICON_WIDGET_NAME));
		
		UpdateTotalResources(GetTotalResources());
		
		// Check if player has enough Military Allocated Supplies
		CheckPersonalResources(GetPersonalResourceCost());
		
		SetItemRank();
		
		//Widget ammoTypeWidget = m_widget.FindAnyWidget(AMMOTYPE_WIDGET_NAME);
		//if (ammoTypeWidget)
		//	ammoTypeWidget.SetVisible(true);
		
		CheckRequiredRank();
	}
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
		
		if (!m_pItem || !m_pItem.GetOwner())
			return;
		
		UpdateTotalResources(GetTotalResources());
		CheckPersonalResources(GetPersonalResourceCost());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckRequiredRank()
	{
		ImageWidget rankIcon = ImageWidget.Cast(m_widget.FindAnyWidget(RANK_ICON_WIDGET_NAME));
		if (!rankIcon)
			return;

		rankIcon.SetVisible(false);
		
		//~ No rank required
		if (m_eRequiredRank < 0)
			return;
		
		//~ Get the rank icon and set it and disable the slot
		IEntity localPlayerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!localPlayerEntity || m_eRequiredRank <= SCR_CharacterRankComponent.GetCharacterRank(localPlayerEntity))
			return;

		SetItemAvailability(false);
		rankIcon.SetVisible(true);
		
		if (m_wMSARIcon)
			m_wMSARIcon.SetVisible(false);
		
		ResourceName rankIconImageSet = SCR_XPInfoDisplay.GetRankIconImageSet();
		if (rankIconImageSet.IsEmpty())
			return;
		
		FactionAffiliationComponent playerFactionAffiliation = FactionAffiliationComponent.Cast(localPlayerEntity.FindComponent(FactionAffiliationComponent));
		if (!playerFactionAffiliation)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(playerFactionAffiliation.GetAffiliatedFaction());
		if (!playerFaction)
			return;
		
		string rankInsignia = playerFaction.GetRankInsignia(m_eRequiredRank);
		if (rankInsignia.IsEmpty())
			return;
		
		//~ Set rank icon
		rankIcon.LoadImageFromSet(0, rankIconImageSet, rankInsignia);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckPersonalResources(int cost)
	{
		if (m_wMSARIcon)
			m_wMSARIcon.SetVisible(false);

		if (cost == 0)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!playerSupplyAllocationComponent)
			return;

		int msarBalance = playerSupplyAllocationComponent.GetPlayerAvailableAllocatedSupplies() - cost;
		SetItemAvailability(msarBalance >= 0  && m_bIsAvailable);

		if (!m_wMSARIcon)
			return;

		m_wMSARIcon.SetVisible(msarBalance < 0);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetPersonalResourceCost()
	{
		SCR_ArsenalManagerComponent arsenalManagerComponent;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManagerComponent);
		if (!arsenalManagerComponent || !arsenalManagerComponent.IsMilitarySupplyAllocationEnabled())
			return 0;

		ResourceName resourceName = m_pItem.GetOwner().GetPrefabData().GetPrefabName();
		if (resourceName.IsEmpty())
			return 0;

		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		if (!storageEnt)
			return 0;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEnt.FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return 0;

		return arsenalManagerComponent.GetItemMilitarySupplyAllocationCost(resourceName, arsenalComponent, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateTotalResources(float totalResources)
	{
		if (!m_CostResourceHolderText || !m_CostResourceHolder)
			return;
		
		if (totalResources < 0)
		{
			SetItemAvailability(true);
			m_CostResourceHolder.SetVisible(false);
			return;
		}
		else 
		{
			m_CostResourceHolder.SetVisible(true);
		}
		
		m_CostResourceHolderText.SetText(totalResources.ToString());
		
		if (!m_ArsenalResourceComponent)
		{
			SetItemAvailability(true);
			return;
		}
		
		SCR_ResourceConsumer consumer = m_ArsenalResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
		{	
			SetItemAvailability(true);
			return;
		}
		
		SetItemAvailability(totalResources <= consumer.GetAggregatedResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsDraggable()
	{
		return m_bIsAvailable && super.IsDraggable();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If the slot is locked or not
	bool IsAvailable()
	{
		return m_bIsAvailable;
	}
	
 	//------------------------------------------------------------------------------------------------
	float GetTotalResources()
	{
		m_fSupplyCost = 0;
		
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEnt.FindComponent(SCR_ArsenalComponent));
		if (arsenalComponent && !arsenalComponent.IsArsenalUsingSupplies())
		{
			m_fSupplyCost = -1;
			return m_fSupplyCost;
		}
		
		
		if (!m_pItem || !m_pItem.GetOwner())
			return 0;
		
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return 0;
		
		SCR_Faction faction;
		if (arsenalComponent)
			faction = arsenalComponent.GetAssignedFaction();
		
		Resource resource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
		
		SCR_EntityCatalogEntry entry;
		if (faction)
			entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName(), faction);
		else 
			entry = entityCatalogManager.GetEntryWithPrefabFromCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName());
		
		if (!entry)
			return 0;
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return 0;
		
		if (arsenalComponent)
			m_fSupplyCost = data.GetSupplyCost(arsenalComponent.GetSupplyCostType());
		else 
			m_fSupplyCost = data.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT);
		
		if (!m_ArsenalResourceComponent)
			return m_fSupplyCost;
		
		
		SCR_ResourceConsumer consumer = m_ArsenalResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!consumer)
			return m_fSupplyCost;
		
		m_fSupplyCost = m_fSupplyCost * consumer.GetBuyMultiplier();
		
		return m_fSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemAvailability(bool isAvailable)
	{
		if (!m_wDimmerEffect)
			return;
		
		m_bIsAvailable = isAvailable;
		
		if (!m_wDimmerEffect.IsVisible() == isAvailable)
			return;
		
		m_wDimmerEffect.SetVisible(!isAvailable);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetItemSupplyCost()
	{
		return m_fSupplyCost;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetItemRank()
	{
		m_eRequiredRank = -1;
		
		if (!m_pItem || !m_pItem.GetOwner())
			return;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager) || !arsenalManager.AreItemsRankLocked())
			return;
		
		//~ TODO: This should be merged with total Resources to only update once!!!
		SCR_EntityCatalogManagerComponent entityCatalogManager = SCR_EntityCatalogManagerComponent.GetInstance();
		if (!entityCatalogManager)
			return;
		
		IEntity storageEnt = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEnt.FindComponent(SCR_ArsenalComponent));
		
		SCR_Faction faction;
		if (arsenalComponent)
			faction = arsenalComponent.GetAssignedFaction();
		
		Resource resource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
		if (!resource.IsValid())
			return;
		
		SCR_EntityCatalogEntry entry;
		if (faction)
			entry = entityCatalogManager.GetEntryWithPrefabFromFactionCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName(), faction);
		else 
			entry = entityCatalogManager.GetEntryWithPrefabFromAnyCatalog(EEntityCatalogType.ITEM, resource.GetResource().GetResourceName());
		
		if (!entry)
			return;
		
		SCR_ArsenalItem data = SCR_ArsenalItem.Cast(entry.GetEntityDataOfType(SCR_ArsenalItem));
		if (!data)
			return;
		
		m_eRequiredRank = data.GetRequiredRank();
	}

	//------------------------------------------------------------------------------------------------
	//! \return Get the required rank for the item in the arsenal slot
	SCR_ECharacterRank GetRequiredRank()
	{
		return m_eRequiredRank;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSupplyAllocationChanged(int amount)
	{
		UpdateTotalResources(GetTotalResources());
		CheckPersonalResources(GetPersonalResourceCost());
		CheckRequiredRank();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_ArsenalInventorySlotUI(InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, bool bVisible = true, int iSlotIndex = -1, SCR_ItemAttributeCollection pAttributes = null)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!playerSupplyAllocationComponent)
			return;

		playerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Insert(OnPlayerSupplyAllocationChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ArsenalInventorySlotUI()
	{
		SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Remove(this);
		
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerSupplyAllocationComponent playerSupplyAllocationComponent = SCR_PlayerSupplyAllocationComponent.Cast(playerController.FindComponent(SCR_PlayerSupplyAllocationComponent));
		if (!playerSupplyAllocationComponent)
			return;

		playerSupplyAllocationComponent.GetOnAvailableAllocatedSuppliesChanged().Remove(OnPlayerSupplyAllocationChanged);
	}
};