// Un arm NPCs (Disabled)
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_RemoveWeaponContextAction : SCR_SelectedEntitiesContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		//Disabled for now
		return false;
		
		if (selectedEntity.GetEntityType() == EEditableEntityType.CHARACTER)
		{
			DamageManagerComponent damageComponent = DamageManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(DamageManagerComponent));
			if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
				return false;
			
			return SCR_PossessingManagerComponent.GetPlayerIdFromMainEntity(selectedEntity.GetOwner()) <= 0;
		}
		return false;
	}
	
	//Check if character has any weapons
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		if (character && character.IsInVehicle())
			return false;
		
		BaseWeaponManagerComponent wpnManager = BaseWeaponManagerComponent.Cast(selectedEntity.GetOwner().FindComponent(BaseWeaponManagerComponent));
		if (!wpnManager)
			return false;
		
		array<IEntity> weapons = {};
		return wpnManager.GetWeaponsList(weapons) > 0;
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		//Check if anything changed while context action was active (Such as character died or got possessed)
		if (!CanBeShown(selectedEntity, cursorWorldPosition, 0))
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(selectedEntity.GetOwner());
		if (character && character.IsInVehicle())
			return;

		IEntity owner = selectedEntity.GetOwner();
		
		BaseWeaponManagerComponent wpnManager = BaseWeaponManagerComponent.Cast(owner.FindComponent(BaseWeaponManagerComponent));
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_InventoryStorageManagerComponent));
		
		if (!wpnManager || !inventory)
			return;
		
		array<IEntity> weapons = {};
		wpnManager.GetWeaponsList(weapons);
		
		foreach (IEntity weapon: weapons)
		{
			InventoryItemComponent pInvComp = InventoryItemComponent.Cast( weapon.FindComponent( InventoryItemComponent ) );
			if (!pInvComp)
				continue;
			InventoryStorageSlot parentSlot = pInvComp.GetParentSlot();
			if (!parentSlot)
				continue;
		
			inventory.TryRemoveItemFromStorage(weapon, parentSlot.GetStorage());
		}
	}
};