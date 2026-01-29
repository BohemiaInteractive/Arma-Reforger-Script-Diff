[EntityEditorProps(category: "GameScripted/UI/RadialMenu", description: "Radial menu for item selection")]
class SCR_RadialMenuItemsComponentClass : ScriptComponentClass
{
}

class SCR_RadialMenuItemsComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_RadialMenuController m_MenuController;

	protected const int FIRST_ITEM_SLOT = 3;
	protected const int SLOT_COUNT = 10;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;

		if (!GetGame().InPlayMode())
			return;

		// Will setup radial menu and take control over the menu
		SetEventMask(owner, EntityEvent.INIT);
		
		m_MenuController.GetOnTakeControl().Insert(OnControllerTakeControl);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_MenuController)
			return;

		m_MenuController.GetOnControllerChanged().Insert(OnControllerChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpen(SCR_SelectionMenu menu)
	{
		SCR_RadialMenu radialMenu = SCR_RadialMenu.Cast(menu);
		
		radialMenu.ClearEntries();
		CreateEntries(radialMenu);
	}
		
	//------------------------------------------------------------------------------------------------
	protected void Update(SCR_RadialMenu radialMenu)
	{
		radialMenu.UpdateEntries();
	}

	//------------------------------------------------------------------------------------------------
	//! Call this when menu controller starts to control the menu
	protected void OnControllerTakeControl(SCR_RadialMenuController controller)
	{
		SCR_RadialMenu radialMenu = controller.GetRadialMenu();
		if (!radialMenu)
			return;
		
		radialMenu.GetOnOpen().Insert(OnMenuOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControllerChanged(SCR_RadialMenuController controller, bool hasControl)
	{
		SCR_RadialMenu radialMenu = controller.GetRadialMenu();
		if (!radialMenu)
			return;
		
		radialMenu.GetOnOpen().Remove(OnMenuOpen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateEntries(notnull SCR_RadialMenu radialMenu)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (!storage)
			return;

		// Receive items assigned to quick slots
		array<ref SCR_SelectionMenuEntry> entries = {};
		for (int i = FIRST_ITEM_SLOT; i < SLOT_COUNT; i++)
		{
			SCR_ItemSelectionMenuEntry itemEntry = new SCR_ItemSelectionMenuEntry(storage, i);
			entries.Insert(itemEntry);
		}

		radialMenu.AddEntries(entries);
	}
}
