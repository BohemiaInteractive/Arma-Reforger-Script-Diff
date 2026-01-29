//------------------------------------------------------------------------------------------------
//! Implementation of base group for switching that can be created via passing in a list of WeaponSlotComponents
//! When selection is set weapon is selected via the character controller component
class SCR_GroupWeaponEntry : SCR_BaseGroupEntry
{
	//! Owner of this group
	protected IEntity m_pOwner;
	//! The weapon this entry represents
	protected WeaponSlotComponent m_pWeaponSlotComponent;
	
	//------------------------------------------------------------------------------------------------
	//! Selects this item entry as the current one
	//! \return Returns true if selection was sucessful, false otherwise.
	override void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{		
		// We have no means of switching the weapons for our character
		// if the controller component is null
		auto pGenericOwner = GenericEntity.Cast(m_pOwner);
		if (!pGenericOwner)
			return;
			
		auto pCharacterController = CharacterControllerComponent.Cast(pGenericOwner.FindComponent(CharacterControllerComponent));
		if (!pCharacterController)
			return;
		
		//! Select the weapon via character controller
		pCharacterController.SelectWeapon(m_pWeaponSlotComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Anything that should be done when item is deselected should be handled here
	override void OnDonePerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
	
	}
	
	//! Can this entry be performed?
	protected override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		if (m_pWeaponSlotComponent && m_pWeaponSlotComponent.GetWeaponEntity())
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! If target slot is valid, return its UI info
	protected override UIInfo GetUIInfoScript()
	{
		if (m_pWeaponSlotComponent)
			return m_pWeaponSlotComponent.GetUIInfo();
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create new group representing a single weapon
	void SCR_GroupWeaponEntry(IEntity pOwner, WeaponSlotComponent pWeaponSlot)
	{
		// Assign the owner
		m_pOwner = pOwner;
		
		// Assign weapon component
		m_pWeaponSlotComponent = pWeaponSlot;
	}
	
};