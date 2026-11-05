[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkAssignItemToQuickSlot : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "9", uiwidget: UIWidgets.Slider, desc: "Quickslot number to use if the radio has no assignment (1-10).", params: "1 10 1")]
	protected int m_iQuickSlot;

	[Attribute(desc: "Item to assign to a quickslot.")]
	protected ref SCR_ScenarioFrameworkGet m_ItemGetter;

	//------------------------------------------------------------------------------------------------
	//! Assign specific item to the desired quickslot
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!player)
			return;

		CharacterControllerComponent controller = player.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!inventoryManager)
			return;

		IEntity item;
		if (!ValidateInputEntity(object, m_ItemGetter, item))
			return;

		if (!inventoryManager.Contains(item))
		{
			Print("Inventory item can't be assigned to quickslot because it's missing", LogLevel.WARNING);
			return;
		}

		SCR_CharacterInventoryStorageComponent storage = inventoryManager.GetCharacterStorage();
		if (!storage || storage.GetEntityIndexInQuickslots(item) >= 0)
			return;

		int slotIndex = m_iQuickSlot - 1;
		if (!storage.GetQuickSlotItems().IsIndexValid(slotIndex))
			return;

		int assignedSlot = storage.StoreItemToQuickSlot(item, slotIndex);
		if (assignedSlot >= 0 && SCR_WeaponSwitchingBaseUI.s_bOpened)
			SCR_WeaponSwitchingBaseUI.RefreshQuickSlots(assignedSlot);
	}
}
