// Script File
class ThrowGrenadeAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		GenericEntity user = GenericEntity.Cast(pUserEntity);
		CharacterControllerComponent characterController = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));		
		BaseWeaponManagerComponent weaponMgr = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		
		if (characterController && weaponMgr)
		{
			// grenade
			GrenadeSlotComponent grenadeSlot = GrenadeSlotComponent.Cast(weaponMgr.GetCurrent());
			if (grenadeSlot)
			{
				Print("### GRENADE");
				vector groundWeaponMat[4];
				pOwnerEntity.GetTransform(groundWeaponMat);
				IEntity weaponToDrop = weaponMgr.SetSlotWeapon(grenadeSlot, pOwnerEntity);
				Print(weaponToDrop);

				
				if (weaponToDrop) 
				{
					weaponToDrop.SetTransform(groundWeaponMat);
					
					// Wake me up inside
					// Wake me up inside
					// Call my name and save me from the dark
					// (wake rigidbody up, bring it to life!)
					Physics weaponPhysics = weaponToDrop.GetPhysics();
					if (weaponPhysics)
					{
						weaponPhysics.SetActive(1);
					}
				}
			}
			
		}
	}

};