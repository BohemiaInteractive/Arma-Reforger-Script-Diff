[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_GAMEPADClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_GAMEPAD : SCR_BaseTutorialStage
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
		m_InventoryMenu = SCR_InventoryMenuUI.GetInventoryMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HighlightBackpack()
	{	
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		ShowHint(2);
		
		if (!m_wHighlight)
		{
			SCR_InventoryStorageBaseUI storageList = m_InventoryMenu.GetStorageList();
			if (storageList)
			{
				array <SCR_InventorySlotUI> slots = {};
				storageList.GetSlots(slots);
				
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
	protected void HighlightSupplies()
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		ShowHint(1);
		
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
	protected void HighlightVicinity()
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
		
		SCR_InventoryStorageBaseUI storageUI = m_InventoryMenu.GetLootStorage();
		if (!storageUI)
			return;
		
		m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(storageUI.GetButtonWidget(), "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (m_InventoryMenu)
		{
			SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.GetCurrentDialog();
			if (m_InventoryMenu.GetSelectedSlotUI())
			{
				HighlightBackpack();
			}
			else if (dialog)
			{
				ShowHint(2);
			}
			else
			{
				SCR_InventoryStorageBaseUI storageUI = m_InventoryMenu.GetActiveStorageUI();
				if (storageUI)
				{	
					Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
					if (focusedWidget && focusedWidget.GetName() == "ItemButton")
						HighlightSupplies();
					else
						HighlightVicinity();
				}
				else
				{
					HighlightVicinity();
				}
			}
		}
		
		return GetSuppliesInInventory() >= 10;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		m_TutorialComponent.SetStage(m_TutorialComponent.GetStageIndexByName("VEHTURN") - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_VehicleMaintenance_SUPPLY_TRANSFER_GAMEPAD()
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		if (m_InventoryStorageManager)
			m_InventoryStorageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryOpen);
	}
};