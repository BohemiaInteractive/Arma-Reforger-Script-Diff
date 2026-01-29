class SCR_InventoryOpenedStorageArsenalUI : SCR_InventoryOpenedStorageUI
{
	//------------------------------------------------------------------------------------------------
	override protected void GetAllItems(out notnull array<IEntity> pItemsInStorage, BaseInventoryStorageComponent pStorage = null)
	{
		if (pStorage)
		{
			super.GetAllItems(pItemsInStorage, pStorage);

			return;
		}
		
		ChimeraWorld chimeraWorld = GetGame().GetWorld();
		ItemPreviewManagerEntity itemPreviewManagerEntity = chimeraWorld.GetItemPreviewManager();
		
		SCR_ArsenalComponent arsenalComponent	= SCR_ArsenalComponent.Cast(m_Storage.GetOwner().FindComponent(SCR_ArsenalComponent));
		array<ResourceName> prefabsToSpawn		= new array<ResourceName>();
		
		if (!arsenalComponent)
			return;
		
		arsenalComponent.GetAvailablePrefabs(prefabsToSpawn);
		
		foreach (ResourceName resourceName: prefabsToSpawn)
		{
			pItemsInStorage.Insert(itemPreviewManagerEntity.ResolvePreviewEntityForPrefab(resourceName));
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void RefreshResources()
	{		
		if (!m_wResourceAvailableText && !m_wResourceAvailableDisplay)
			return;
		
		if (!m_ResourceComponent)
		{
			m_wResourceAvailableDisplay.SetVisible(false);
			return;
		}
			
		
		float availableResources;
		
		if (!m_ResourceComponent || !m_ResourceComponent.IsResourceTypeEnabled() || !SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, availableResources))
		{
			m_wResourceAvailableDisplay.SetVisible(false);
			return;
		}
		
		m_wResourceAvailableText.SetTextFormat("#AR-Supplies_Arsenal_Availability", SCR_ResourceSystemHelper.SuppliesToString(availableResources));
		
		m_wResourceAvailableDisplay.SetVisible(true);
		
		if (!m_wResourceAvailableDisplay.IsVisible())
			m_wResourceAvailableDisplay.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		super.Init();
		
		m_wWeightDisplay.SetVisible(false);
		m_wProgressBar.SetVisible(false);
		
		m_wWeightText = null;
		m_wWeightDisplay = null;
		m_wCapacityDisplay = null;
		m_wCapacityPercentageText = null;
		
		m_wResourceStoredDisplay = m_widget.FindAnyWidget("ResourceDisplayStored");
		if (m_wResourceStoredDisplay)
			m_wResourceStoredText = TextWidget.Cast(m_wResourceStoredDisplay.FindAnyWidget("ResourceText"));
		
		m_wResourceAvailableDisplay = m_widget.FindAnyWidget("ResourceDisplayAvailable");
		if (m_wResourceAvailableDisplay)
			m_wResourceAvailableText = TextWidget.Cast(m_wResourceAvailableDisplay.FindAnyWidget("ResourceText"));
		
		if (!m_wResourceStoredDisplay || !m_wResourceAvailableText)
			return;
		
		if (m_Storage && m_Storage.GetOwner())
			m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(m_Storage.GetOwner());
		
		if (!m_ResourceComponent)
		{
			m_wResourceAvailableDisplay.SetVisible(false);
			return;
		}
		
		m_ResourceConsumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(RefreshResources);	
		
		RefreshResources();
		m_wResourceStoredDisplay.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected SCR_InventorySlotUI CreateSlotUI( InventoryItemComponent pComponent, SCR_ItemAttributeCollection pAttributes = null )
	{
		if (!pComponent)
		{
			return new SCR_InventorySlotUI(null, this, false, -1, pAttributes);
		}
		else
		{
			SCR_ArsenalInventorySlotUI slotUI = new SCR_ArsenalInventorySlotUI(pComponent, this, false, -1, null); //creates the slot
			
			slotUI.SetArsenalResourceComponent(m_ResourceComponent);
			SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Insert(slotUI, GetCurrentNavigationStorage().GetOwner());
			
			return slotUI;
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
		
		RefreshResources();
	}
};