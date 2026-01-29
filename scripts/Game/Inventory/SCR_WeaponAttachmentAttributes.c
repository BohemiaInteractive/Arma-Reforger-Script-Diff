class SCR_WeaponAttachmentAttributes : WeaponAttachmentAttributes
{
	/*! Apply Modifiers to weapon on attach
	
	This is called when an attachment has been attached to a weapon. 
	\param statsManager The stats manager that called this function. All modifiers should be set through this
	\param muzzleIndex The index of the muzzle, if applicable. This has no real meaning to this function except
	that it should be passed on to muzzle related functions. Can be -1 (which indicates that this attachment
	was done on a non-muzzle attachment slot) or a non-zero integer. See below
	\param attachedEntity The entity being attached
	
	The muzzle index can be interpreted only as a far as -1 means that the attachment was not attached to
	a muzzle. Other than that, the index should be passed on to functions without modification.
	
	*/
	bool ApplyModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		return false;
	}
	
	/*! Clear the modifiers from a weapon on detach
	This is called before an attachment is removed from a weapon.
	\param statsManager The stats manager that called this function. All modifiers should be set through this
	\param muzzleIndex The index of the muzzle, if applicable. This has no real meaning to this function except
	that it should be passed on to muzzle related functions. Can be -1 (which indicates that this attachment
	was done on a non-muzzle attachment slot) or a non-zero integer. 
	\param attachedEntity The entity being detached
	
	See also ApplyModifiers
	
	*/
	void ClearModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
	}

}