[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_TAKE_MAGAZINE_KMBClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_TAKE_MAGAZINE_KMB : SCR_BaseTutorialStage
{
	UniversalInventoryStorageComponent m_Storage;
	SCR_InventoryStorageManagerComponent m_InventoryStorageManager;
	SCR_InventoryMenuUI m_InventoryMenu;
	Widget m_wHighlight;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity box = GetGame().GetWorld().FindEntityByName("Ammobox");
		if (!box)
			return;
		
		m_Storage = UniversalInventoryStorageComponent.Cast(box.FindComponent(UniversalInventoryStorageComponent));

		RegisterWaypoint(box, "", "AMMO");
		
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
		
		ShowHint();
		
		m_InventoryMenu = SCR_InventoryMenuUI.GetInventoryMenu();
		if (!m_InventoryMenu)
			return;
		
		SCR_InventoryStorageBaseUI lootMenuStorage = m_InventoryMenu.GetLootStorage();
		if (!lootMenuStorage)
			return;
		
		int slotID = lootMenuStorage.GetSlotIdForItem("{D8F2CA92583B23D3}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_M855_M856_Last_5Tracer.et");
		
		array <SCR_InventorySlotUI> slots = {};
		lootMenuStorage.GetSlots(slots);	
		if (!slots || slots.IsEmpty())
			return;
		
		if (slots[slotID].m_widget && !m_wHighlight)
			m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(slots[slotID].m_widget, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemDrag()
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		if (!m_wHighlight)
		{
			SCR_InventoryStorageBaseUI storageList = m_InventoryMenu.GetWeaponStorage();
			if (storageList)
			{
				array <SCR_InventorySlotUI> slots = {};
				storageList.GetSlots(slots);
				
				ShowHint(1);
				
				foreach (SCR_InventorySlotUI slot : slots)
				{
					if (slot.GetStorageComponent().GetOwner().GetName() != "M16")
						continue;
					
					m_wHighlight = SCR_WidgetHighlightUIComponent.CreateHighlight(slot.m_widget, "{D574871D2C37B255}UI/layouts/Common/WidgetHighlight.layout");
					break;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsMagazineLoaded()
	{
		IEntity weapon = GetGame().GetWorld().FindEntityByName("M16");
		if (!weapon)
			return false;
		
		BaseWeaponComponent weaponComponent = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
		if (!weaponComponent)
			return false;
		
		return weaponComponent.GetCurrentMagazine();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (m_InventoryMenu)
		{
			if (m_InventoryMenu.IsDraggingEnabled())
				OnItemDrag();
			else
				OnInventoryOpen(true);
		}
		
		if (IsMagazineLoaded())
			return true;
		
		return m_TutorialComponent.FindPrefabInPlayerInventory("{D8F2CA92583B23D3}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_M855_M856_Last_5Tracer.et");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		m_TutorialComponent.SetStage("CLOSE_INVENTORY");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_ShootingRange_TAKE_MAGAZINE_KMB()
	{
		if (m_wHighlight)
			m_wHighlight.RemoveFromHierarchy();
		
		if (m_InventoryStorageManager)
			m_InventoryStorageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryOpen);
	}
};