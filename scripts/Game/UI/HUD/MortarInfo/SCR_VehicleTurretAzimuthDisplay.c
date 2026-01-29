//! Use this class when working with vehicles as they need SightsTransform instead of MuzzleTransform
class SCR_VehicleTurretAzimuthDisplay : SCR_BaseWeaponAzimuthDisplay
{
	//------------------------------------------------------------------------------------------------
	override vector GetMuzzleDirection()
	{
		if (!m_TurretController)
			return vector.Zero;
		
		BaseWeaponManagerComponent weaponManager = m_TurretController.GetWeaponManager();
		if (!weaponManager)
			return vector.Zero;
		
		WeaponSlotComponent weaponSlot = weaponManager.GetCurrentSlot();
		if (!weaponSlot)
			return vector.Zero;
		
		IEntity weaponEntity = weaponSlot.GetWeaponEntity();
		if (!weaponEntity)
			return vector.Zero;

		vector transformLocal[4];
		weaponEntity.GetTransform(transformLocal);
		
		return transformLocal[2];
	}
}