[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ResupplyContextAction : SCR_SelectedEntitiesContextAction
{
	
	[Attribute("4", desc: "Amount of magazines the action will ensure are in the inventory (fill to this amount)")]
	private int m_MagazineQuantityInInventory;
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (selectedEntity == null || selectedEntity.GetEntityType() != EEditableEntityType.CHARACTER)
			return false;

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		
		if (!character)
			return false;
		
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
			return false;
		
		return inventoryManager.IsResupplyMagazinesAvailable(m_MagazineQuantityInInventory);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		if (!character)
			return;
		
		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
			return;
		
		inventoryManager.ResupplyMagazines(m_MagazineQuantityInInventory);
	}
};