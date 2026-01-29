class SCR_IdentityItemStorageComponentClass : SCR_EquipmentStorageComponentClass
{
}

class SCR_IdentityItemStorageComponent : SCR_EquipmentStorageComponent
{
	//------------------------------------------------------------------------------------------------
	//~ Make sure the identity item is not shown if 'worn'
	protected override void OnAddedToSlot(IEntity item, int slotID)
	{
		super.OnAddedToSlot(item, slotID);
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;
		
		itemComponent.HideOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Make sure the identity item is shown when removed from the slot
	override void OnRemovedFromSlot(IEntity item, int slotID)
	{
		super.OnRemovedFromSlot(item, slotID);
		
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;
		
		itemComponent.ShowOwner();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Only allow items to be stored in the slot if the identity slot is enabled
	override bool CanStoreItem(IEntity item, int slotID) 
	{		
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		
		return identityManager && identityManager.IsIdentityItemSlotEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Only allow items to be stored in the slot if the identity slot is enabled
	override bool CanStoreResource(ResourceName resourceName, int slotID)
	{
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		
		return identityManager && identityManager.IsIdentityItemSlotEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ When identity slot is enabled it will never allow the item to be removed (Unless the item is also added when character falls Unconscious)
	override bool CanRemoveItem(IEntity item)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return true;
		
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		
		//~ Can remove when Unconscious
		if (identityManager && identityManager.HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType.ON_UNCONSCIOUS))
		{
			//~ Character is unconcious
			SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controllerComponent && controllerComponent.GetLifeState() == ECharacterLifeState.INCAPACITATED)
				return true;
		}
		
		//~ Allowed to remove the item if the character is dead
		return character && character.GetDamageManager().GetState() == EDamageState.DESTROYED;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ When identity slot is enabled it will never allow the item to be replaced
	override bool CanReplaceItem(IEntity nextItem, int slotID)
	{
		return false;
	}
}