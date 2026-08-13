//------------------------------------------------------------------------------------------------
class SCR_SupplyInventorySlotUI : SCR_InventorySlotUI
{
	const string SLOT_LAYOUT_SUPPLY = "{AF25F325D2730142}UI/layouts/Menus/Inventory/SupplyInventoryItemSlot.layout";
	const float SLOT_UNAVAILABLE_OPACITY = 0.35;
	
	protected const string RESOURCES_TEXT_WIDGET_NAME = "SuppliesText";
	protected const string STORED_RESOURCES_WIDGET_NAME = "SuppliesStored";
	protected const string AVAILABLE_RESOURCES_WIDGET_NAME = "SuppliesAvailable";
	protected const string COST_RESOURCES_WIDGET_NAME = "SuppliesCost";
	
	protected Widget m_StoredResourcesHolder;
	protected TextWidget m_StoredResourcesText;
	
	protected Widget m_AvailableResourcesHolder;
	protected TextWidget m_AvailableResourcesText;
	
	// What supplies type are displayed
	protected EResourceType m_eResourceType = EResourceType.SUPPLIES;

	//~ ArsenalInventorySlotUI is the cost one
	
	protected LocalizedString m_sCurrentAndMaxResourceFormat = "#AR-Campaign_BaseSuppliesAmount";
	
	protected ref array<RplId> m_aStackedItems = {};
	
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_ResourceConsumer m_ResourceConsumer;
	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandle;

	//------------------------------------------------------------------------------------------------
	SCR_ResourceComponent GetResourceComponent()
	{
		return m_ResourceComponent;
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
	override void SetItemFunctionality()
	{
		if (m_pItem.GetOwner().FindComponent(BaseInventoryStorageComponent))
			m_eSlotFunction = ESlotFunction.TYPE_STORAGE;			
	}
	
	//------------------------------------------------------------------------------------------------	
	override void UseItem(IEntity player, SCR_EUseContext context)
	{
		if (!m_pItem)
			return;
		
		IEntity item = m_pItem.GetOwner();
		if (!item)
			return;
		
		SCR_CharacterInventoryStorageComponent storage = GetCharacterStorage(player);
		if (!storage)
			return;

		ESlotFunction overrideSlotFunction = m_eSlotFunction;
		if (item.FindComponent(SCR_GadgetComponent))
			overrideSlotFunction = ESlotFunction.TYPE_GADGET;

		if (storage.UseItem(item, overrideSlotFunction, context))
			Refresh();
	}
	
	//------------------------------------------------------------------------------------------------	
	override bool CanUseItem(IEntity player)
	{
		if (!m_pItem)
			return false;
		
		IEntity item = m_pItem.GetOwner();
		if (!item)
			return false;
		
		SCR_CharacterInventoryStorageComponent storage = GetCharacterStorage(player);
		if (!storage)
			return false;
		
		ESlotFunction overrideSlotFunction = m_eSlotFunction;
		if (item.FindComponent(SCR_GadgetComponent))
			overrideSlotFunction = ESlotFunction.TYPE_GADGET;

		return storage.CanUseItem(item, overrideSlotFunction);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDraggable()
	{
		if (!super.IsDraggable())
			return false;

		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		return !m_ResourceComponent || char && m_ResourceComponent.CanBeUsedByCharacter(char);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Refresh()
	{
		super.Refresh();
		
		UpdateConsumer();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSlotVisible(bool bVisible)
	{
		super.SetSlotVisible(bVisible);
		
		if (!m_wDimmerEffect)
			m_wDimmerEffect = m_widget.FindAnyWidget("Dimmer");
		
		m_StoredResourcesHolder = m_widget.FindAnyWidget(STORED_RESOURCES_WIDGET_NAME);
		if (m_StoredResourcesHolder)
			m_StoredResourcesText = TextWidget.Cast(m_StoredResourcesHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		m_AvailableResourcesHolder = m_widget.FindAnyWidget(AVAILABLE_RESOURCES_WIDGET_NAME);
		if (m_AvailableResourcesHolder)
			m_AvailableResourcesText = TextWidget.Cast(m_AvailableResourcesHolder.FindAnyWidget(RESOURCES_TEXT_WIDGET_NAME));
		
		// Never show cost
		Widget resourceCost = m_widget.FindAnyWidget(COST_RESOURCES_WIDGET_NAME);
		if (resourceCost)
			resourceCost.SetVisible(false);
		
		Refresh();
    }
	
	//------------------------------------------------------------------------------------------------
	void UpdateConsumer()
	{
		if (!m_ResourceConsumer)
			return;
		
		float totalResources, maxResources;
		
		// Stored supplies
		bool showUI = SCR_ResourceSystemHelper.GetStoredAndMaxResources(m_ResourceComponent, totalResources, maxResources, m_eResourceType);
		UpdateStoredResources(showUI, totalResources, maxResources);
		
		// Available Supplies
		showUI = m_ResourceComponent.IsResourceTypeEnabled() && SCR_ResourceSystemHelper.GetAvailableResources(m_ResourceComponent, totalResources, m_eResourceType);
		UpdateAvailableResources(showUI, totalResources);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateStoredResources(bool showUI, float totalResources, float maxResources)
	{
		if (!m_StoredResourcesHolder || !m_StoredResourcesText)
			return;
		
		if (!showUI)
			return m_StoredResourcesHolder.SetVisible(false);
		
		m_StoredResourcesText.SetTextFormat(
			m_sCurrentAndMaxResourceFormat, 
			SCR_ResourceSystemHelper.SuppliesToString(totalResources), 
			SCR_ResourceSystemHelper.SuppliesToString(maxResources));
		m_StoredResourcesHolder.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateAvailableResources(bool showUI, float totalResources)
	{
		if (!m_AvailableResourcesHolder || !m_AvailableResourcesText)
			return;
		
		if (!showUI)
		{
			m_AvailableResourcesHolder.SetVisible(false);
			return;
		}
		
		// Update supplies
		m_AvailableResourcesText.SetText(SCR_ResourceSystemHelper.SuppliesToString(totalResources));
		m_AvailableResourcesHolder.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(m_pItem.GetOwner());
		
		if (!m_ResourceComponent)
			return Debug.Error(string.Format("%1 is being used on an entity without SCR_ResourceComponent", Type().ToString()));
		
		m_ResourceConsumer = SCR_ResourceSystemHelper.GetFirstValidConsumer(m_ResourceComponent);
		
		if (!m_ResourceConsumer)
			return Debug.Error(string.Format("%1 is being used on an entity without an SCR_ResourceComponent with no valid SCR_ResourceConsumer", Type().ToString()));
		
		m_ResourceComponent.TEMP_GetOnInteractorReplicated().Insert(OnResourceComponentUpdated);
		
		SCR_ResourcePlayerControllerInventoryComponent resourceInventoryPlayerComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
		
		if (!resourceInventoryPlayerComponent)
			return;
		
		RplId resourceInventoryPlayerComponentRplId = Replication.FindItemId(resourceInventoryPlayerComponent);
		
		if (!resourceInventoryPlayerComponentRplId.IsValid())
			return;
		
		m_ResourceSubscriptionHandle = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(m_ResourceConsumer, resourceInventoryPlayerComponentRplId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ResourceConsumer)
			m_ResourceComponent.TEMP_GetOnInteractorReplicated().Remove(OnResourceComponentUpdated);
		
		m_ResourceSubscriptionHandle = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that triggers when the resource component tied to the UI slot updates/replicates.
	protected void OnResourceComponentUpdated()
	{
		// We need to gather the consumer again due to it being recreated by replication when 
		// replicated through the RplProp. Meaning that the previous reference is lost.
		m_ResourceConsumer = SCR_ResourceSystemHelper.GetFirstValidConsumer(m_ResourceComponent);
		
		Refresh();
	}
};