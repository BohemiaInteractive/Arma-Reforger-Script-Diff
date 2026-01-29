[BaseContainerProps()]
class SCR_IdentityItemAttributeCollection : SCR_ItemAttributeCollection
{
	//------------------------------------------------------------------------------------------------
	//~ Hide if character is unconscious
	override bool IsVisible(InventoryItemComponent inventoryItem)
	{
		//~ Is already not visible, or is not in a slot
		if (!m_bVisible || !inventoryItem || !inventoryItem.GetParentSlot())
			return m_bVisible;
		
		//~ Either the item is not in the slot or the item is added when a character falls unconcious. In this case just show the item as normal
		SCR_IdentityManagerComponent identityManager = SCR_IdentityManagerComponent.GetInstance();
		if (!identityManager || !identityManager.IsIdentityItemSlotEnabled() || identityManager.HasIdentityItemGenerationType(SCR_EIdentityItemGenerationType.ON_UNCONSCIOUS))
			return m_bVisible;
		
		ChimeraCharacter parentChar = ChimeraCharacter.Cast(inventoryItem.GetOwner().GetParent());
		if (!parentChar)
			return m_bVisible;
		
		SCR_DamageManagerComponent damageManager = parentChar.GetDamageManager();
		if (!damageManager || damageManager.GetState() == EDamageState.DESTROYED)
			return m_bVisible;
		
		CharacterControllerComponent controller = parentChar.GetCharacterController();
		if (controller && controller.IsUnconscious())
			return false;
		
		return m_bVisible;
	}
}
	