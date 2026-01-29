[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_KMBClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_KMB : SCR_BaseTutorialStage
{
	protected UniversalInventoryStorageComponent m_Storage;
	protected SCR_InventoryStorageManagerComponent m_InventoryStorageManager;
	protected SCR_InventoryMenuUI m_InventoryMenu;
	protected Widget m_wHighlight;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity storage = GetGame().GetWorld().FindEntityByName("StorageDepo");
		if (!storage)
			return;
		
		m_Storage = UniversalInventoryStorageComponent.Cast(storage.FindComponent(UniversalInventoryStorageComponent));

		RegisterWaypoint(storage, "", "CUSTOM");
		
		m_InventoryStorageManager = m_TutorialComponent.GetPlayerInventory();
		if (!m_InventoryStorageManager)
			return;
		
		OnInventoryOpen(true);
		m_InventoryStorageManager.m_OnInventoryOpenInvoker.Insert(OnInventoryOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInventoryOpen(bool open)
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		SCR_HintManagerComponent hintMan = SCR_HintManagerComponent.GetInstance();
		if (hintMan && hintMan.GetCurrentHint() == m_StageInfo.GetHint())
			hintMan.RepeatHint(true);
		else
			ShowHint();
		
		m_InventoryMenu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (!m_InventoryMenu)
			return;
		
		SCR_InventoryStorageBaseUI lootMenuStorage = m_InventoryMenu.GetLootStorage();
		if (!lootMenuStorage)
			return;
		
		array <SCR_InventorySlotUI> slots = {};
		lootMenuStorage.GetSlots(slots);	
		if (!slots || slots.IsEmpty())
			return;
		
		if (slots[0].m_widget && !m_wHighlight)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(slots[0].m_widget, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemDrag()
	{
		if (!m_InventoryMenu.GetDraggedSlot().IsInherited(SCR_SupplyInventorySlotUI))
			return;
		
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		if (!m_wHighlight)
		{
			SCR_InventoryStorageBaseUI storageList = m_InventoryMenu.GetStorageList();
			if (storageList)
			{
				array <SCR_InventorySlotUI> slots = {};
				storageList.GetSlots(slots);
				
				ShowHint(1);
				
				BaseInventoryStorageComponent storageComp;
				foreach (SCR_InventorySlotUI slot : slots)
				{
					if (!slot)
						 continue;
					
					//TODO> Whatever this inventory mumbo jumbo is, we should try to find most optimal and least retarded way. I went full retard with this one
					if (!slot.GetLoadoutArea() || !slot.GetLoadoutArea().IsInherited(LoadoutBackpackArea))
						continue;
					
					storageComp = slot.GetStorageComponent();
					
					
					SCR_InventoryStorageBaseUI storageUI = m_InventoryMenu.GetStorageUIByBaseStorageComponent(storageComp);
					
					m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(storageUI.GetStorageGrid(), "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (m_InventoryMenu)
		{
			SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.GetCurrentDialog();
			if (m_InventoryMenu.IsDraggingEnabled())
				OnItemDrag();
			else if (dialog)
				ShowHint(1);
			else
				OnInventoryOpen(true);
		}
		
		return GetSuppliesInInventory() >= 10;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		m_TutorialComponent.SetStage(m_TutorialComponent.GetStageIndexByName("VEHTURN") - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_KMB()
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		if (m_InventoryStorageManager)
			m_InventoryStorageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryOpen);
	}
};