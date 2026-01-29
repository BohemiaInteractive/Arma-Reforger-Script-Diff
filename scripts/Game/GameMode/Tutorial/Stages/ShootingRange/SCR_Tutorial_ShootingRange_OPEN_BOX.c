[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_OPEN_BOXClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_OPEN_BOX : SCR_BaseTutorialStage
{
	IEntity m_Box;
	SCR_InventoryStorageManagerComponent m_InventoryStorageManager;
	SCR_InventoryMenuUI m_InventoryMenu;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_TutorialComponent.EnableArsenal("Ammobox", true);
		
		m_Box = GetGame().GetWorld().FindEntityByName("Ammobox");
		if (!m_Box)
			return;

		RegisterWaypoint(m_Box, "", "AMMO");
		
		m_InventoryStorageManager = m_TutorialComponent.GetPlayerInventory();
		if (!m_InventoryStorageManager)
			return;
		
		m_InventoryStorageManager.m_OnInventoryOpenInvoker.Insert(OnInventoryOpen);
		
		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	//TODO> Could probably move this to TutorialGamemodeComponent
	protected void OnInventoryOpen(bool open)
	{
		m_InventoryMenu = SCR_InventoryMenuUI.GetInventoryMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_InventoryMenu)
			return false;
		
		SCR_InventoryStorageBaseUI storageBaseUI = m_InventoryMenu.GetLootStorage();
		if (!storageBaseUI)
			return false;
		
		array <BaseInventoryStorageComponent> aTraverseStorage = {};
		storageBaseUI.GetTraverseStorage(aTraverseStorage);
		if (aTraverseStorage.IsEmpty())
			return false;
		
		foreach (BaseInventoryStorageComponent storage : aTraverseStorage)
		{
			if (storage.GetOwner() != m_Box)
				continue;
			
			m_InventoryStorageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryOpen);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			m_TutorialComponent.InsertStage("KMB");
		else
			m_TutorialComponent.InsertStage("GAMEPAD");
		
		SCR_TutorialLogic_ShootingRange logic = SCR_TutorialLogic_ShootingRange.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
		{
			GetGame().OnInputDeviceIsGamepadInvoker().Insert(logic.OnInputChanged);
		}
		
		super.OnStageFinished();
	}
};