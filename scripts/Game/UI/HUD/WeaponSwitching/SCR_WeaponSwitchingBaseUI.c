[EntityEditorProps(category: "GameScripted/UI/HUD/WeaponSwitchingBar", description: "Concept of quick selection bar")]

//---- REFACTOR NOTE START: This script is not made the best way and relies on delayed calls a bit too much. ----

class SCR_WeaponSwitchingBaseUI : SCR_InfoDisplay
{
	protected static ResourceName							s_sItemPreviewManagerPrefab = "{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et"; // This could be attribute
	protected static Widget									s_wQuickSlotStorage;
	protected static ref SCR_InventoryStorageQuickSlotsUI	s_QuickSlotStorage;
	protected static SCR_WeaponSwitchingBaseUI				s_WeaponSwitchingUI;

	static bool												s_bOpened;
	static bool												s_bRadial;

	[Attribute("{A1E61EF091EAC47D}UI/layouts/Menus/Inventory/InventoryQuickSlotsGrid.layout")]
	protected string m_sQuickSlotGridLayout;

	//------------------------------------------------------------------------------------------------
	static void RefreshQuickSlots(int id = -1)
	{
		if (!s_QuickSlotStorage)
			return;

		if (id < 0)
			s_QuickSlotStorage.RefreshQuickSlots();
		else
			s_QuickSlotStorage.RefreshSlot(id);

		s_QuickSlotStorage.HighlightLastSelectedSlot();
	}

	//------------------------------------------------------------------------------------------------
	static void SelectQuickSlot(int id)
	{
		bool wasOpen = s_bOpened;
		if (!wasOpen && s_WeaponSwitchingUI)
			s_WeaponSwitchingUI.OpenQuickSlots();

		if (s_QuickSlotStorage)
			s_QuickSlotStorage.SelectSlot(id);

		if (!wasOpen && s_WeaponSwitchingUI)
			s_WeaponSwitchingUI.CloseQuickSlots();
	}

	//------------------------------------------------------------------------------------------------
	static void HighlightQuickSlot(int id, bool highlight = true)
	{
		bool wasOpen = s_bOpened;
		if (!wasOpen && s_WeaponSwitchingUI)
			s_WeaponSwitchingUI.OpenQuickSlots();

		if (s_QuickSlotStorage)
			s_QuickSlotStorage.HighlightSlot(id, highlight);

		if (!wasOpen && s_WeaponSwitchingUI)
			s_WeaponSwitchingUI.CloseQuickSlots();
	}

	//------------------------------------------------------------------------------------------------
	protected void Init(IEntity owner)
	{
		if (s_WeaponSwitchingUI)
		{
			Print("More than one SCR_WeaponSwitchingBaseUI used in " + __FILE__ + " L" + __LINE__, LogLevel.WARNING);
			return;
		}

		s_WeaponSwitchingUI = this;
		s_bOpened = false;

		if (!owner || !m_wRoot)
			return;

		ChimeraWorld world = ChimeraWorld.CastFrom(owner.GetWorld());
		if (world)
		{
			//instantiate the preview manager
			if (!world.GetItemPreviewManager())
			{
				Resource rsc = Resource.Load(s_sItemPreviewManagerPrefab);
				if (rsc.IsValid())
					GetGame().SpawnEntityPrefabLocal(rsc, world);
			}
		}

		InputManager inputManager = GetGame().GetInputManager();
		inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_QuickSlotsBar);
		inputManager.AddActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_QuickSlotsBar);
		inputManager.AddActionListener("Inventory_WeaponSwitchingRadial", EActionTrigger.DOWN, Action_QuickSlotsRadial);
		inputManager.AddActionListener("Inventory_WeaponSwitchingRadial", EActionTrigger.UP, Action_QuickSlotsRadial);

		for (int i; i < 10; ++i)
		{
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_SwitchSlot);
			inputManager.AddActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_QuickSlotsBar);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.DOWN, Action_QuickSlotsBar);
		inputManager.RemoveActionListener("Inventory_WeaponSwitching", EActionTrigger.UP, Action_QuickSlotsBar);
		inputManager.RemoveActionListener("Inventory_WeaponSwitchingRadial", EActionTrigger.DOWN, Action_QuickSlotsRadial);
		inputManager.RemoveActionListener("Inventory_WeaponSwitchingRadial", EActionTrigger.UP, Action_QuickSlotsRadial);

		for (int i; i < 10; ++i)
		{
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.DOWN, Action_SwitchSlot);
			inputManager.RemoveActionListener("SwitchWeaponCategory" + i.ToString(), EActionTrigger.UP, Action_QuickSlotsBar);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OpenQuickSlots()
	{
		if (s_bOpened)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		CharacterControllerComponent controller;

		if (character)
			controller = character.GetCharacterController();

		if (controller && controller.GetLifeState() != ECharacterLifeState.ALIVE)
			return;

		GetGame().GetInputManager().AddActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_ScrollSlot);
		s_bOpened = true;
		GetGame().GetCallqueue().Remove(ShowQuickSlots);		// if there's a delayed Show method from the previous quick bar usage, purge it

		if (!m_wRoot)
			return;

		if (s_wQuickSlotStorage)
		{
			s_wQuickSlotStorage.RemoveHandler(s_wQuickSlotStorage.FindHandler(SCR_InventoryStorageQuickSlotsUI));
			s_wQuickSlotStorage.RemoveFromHierarchy();
		}
		Widget parent = m_wRoot.FindAnyWidget("QuickSlots");
		s_wQuickSlotStorage = GetGame().GetWorkspace().CreateWidgets(m_sQuickSlotGridLayout, parent);
		if (!s_wQuickSlotStorage)
			return;

		s_QuickSlotStorage = new SCR_InventoryStorageQuickSlotsUI(null, null, null);
		s_wQuickSlotStorage.AddHandler(s_QuickSlotStorage);
		s_QuickSlotStorage.SetInitialQuickSlot();
		s_QuickSlotStorage.HighlightLastSelectedSlot();

		Show(true, UIConstants.FADE_RATE_DEFAULT);
		BlurWidget wBlur = BlurWidget.Cast(m_wRoot.FindAnyWidget("wBlur"));

		if (wBlur)
			wBlur.SetVisible(true);

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_HOTKEY_OPEN);

		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (inventory)
		{
			inventory.m_OnQuickBarOpenInvoker.Invoke(true);
			// Delay by 1 frame so the selected slot gets updated first
			GetGame().GetCallqueue().Call(ShowCommand, inventory);
		}
	}

	//------------------------------------------------------------------------------------------------
	void CloseQuickSlots()
	{
		if (s_QuickSlotStorage)
			s_QuickSlotStorage.SetQuickBarClosed();

		GetGame().GetInputManager().RemoveActionListener("CharacterSwitchWeapon", EActionTrigger.VALUE, Action_ScrollSlot);
		s_bOpened = false;

		if (!m_wRoot)
			return;

		BlurWidget wBlur = BlurWidget.Cast(m_wRoot.FindAnyWidget("wBlur"));
		if (wBlur)
			wBlur.SetVisible(false);

		if (s_QuickSlotStorage && s_QuickSlotStorage.UseItemInSlot())
			GetGame().GetCallqueue().CallLater(ShowQuickSlots, 2000, false, false, UIConstants.FADE_RATE_DEFAULT, true);
		else
			Show(false, UIConstants.FADE_RATE_DEFAULT);

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (inventory)
			inventory.m_OnQuickBarOpenInvoker.Invoke(false);
		
		SCR_PlayerControllerCommandingComponent commandComp = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
		if (commandComp)
			commandComp.HideCommandPreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowCommand(notnull SCR_InventoryStorageManagerComponent inventory)
	{
		SCR_CharacterInventoryStorageComponent characterStorage = inventory.GetCharacterStorage();
		if (!characterStorage)
			return;

		SCR_QuickslotCommandContainer commandQuickSlotContainer = SCR_QuickslotCommandContainer.Cast(characterStorage.GetContainerFromQuickslot(s_QuickSlotStorage.GetLastSelectedSlotIndex()));
		if (!commandQuickSlotContainer)
			return;
		
		SCR_PlayerControllerCommandingComponent commandComp = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
		if (!commandComp)
			return;
		
		commandComp.ShowCommandPreview(commandQuickSlotContainer.GetCommandName());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Action_QuickSlotsRadial(float value = 0.0, EActionTrigger reason = 0)
	{
		bool radial = reason == EActionTrigger.DOWN;

		if (!s_bOpened)
		{
			s_bRadial = radial;
			return;
		}
		

		// Restart the quick slots bar
		if (radial != s_bRadial)
		{
			s_bRadial = radial;
			CloseQuickSlots();
			OpenQuickSlots();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_QuickSlotsBar(float value = 0.0, EActionTrigger reason = 0)
	{
		if (reason == EActionTrigger.DOWN)
			OpenQuickSlots();
		else
			CloseQuickSlots();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_SwitchSlot(float value = 0.0, EActionTrigger reason = 0)
	{
		int targetSlot = -1;

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager.GetActionTriggered("SwitchWeaponCategory1"))
			targetSlot = 0;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory2"))
			targetSlot = 1;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory3"))
			targetSlot = 2;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory4"))
			targetSlot = 3;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory5"))
			targetSlot = 4;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory6"))
			targetSlot = 5;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory7"))
			targetSlot = 6;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory8"))
			targetSlot = 7;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory9"))
			targetSlot = 8;
		else if (inputManager.GetActionTriggered("SwitchWeaponCategory0"))
			targetSlot = 9;

		if (targetSlot < 0)
			return;

		if (s_bOpened && s_bRadial)
			CloseQuickSlots();

		s_bRadial = false;
		OpenQuickSlots();

		SelectQuickSlot(targetSlot);
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_ScrollSlot(float value = 0.0, EActionTrigger reason = 0)
	{
		if (!s_bOpened || !s_QuickSlotStorage)
			return;

		if (float.AlmostEqual(value, 0))
			return;

		s_QuickSlotStorage.SelectSlot(value);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!s_bOpened)
			return;

		GetGame().GetInputManager().ActivateContext("WeaponSelectionContext");
	}

	protected void ShowQuickSlots(bool show, float speed = UIConstants.FADE_RATE_DEFAULT)
	{
		Show(show, speed);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnStartDraw(IEntity owner)
	{
		super.OnStartDraw(owner);
		Init(owner);
		Show(false, 0);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnStopDraw(IEntity owner)
	{
		super.OnStopDraw(owner);
		RemoveActionListeners();
		s_QuickSlotStorage = null;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_WeaponSwitchingBaseUI GetWeaponSwitchingBaseUI()
	{
		return s_WeaponSwitchingUI;
	}
}

class SCR_QuickSlotRefreshCB : ScriptedInventoryOperationCallback
{
	override void OnComplete()
	{
		SCR_WeaponSwitchingBaseUI.RefreshQuickSlots();
	}
}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----