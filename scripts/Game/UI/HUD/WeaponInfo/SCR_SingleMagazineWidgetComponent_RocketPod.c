class SCR_SingleMagazineWidgetComponent_RocketPod : SCR_SingleMagazineWidgetComponent_Base
{
	//------------------------------------------------------------------------------------------------
	override void UpdateAmmoCount(BaseWeaponComponent weapon)
	{
		int curAmmo = weapon.GetCurrentMuzzle().GetAmmoCount();
		int maxAmmo = weapon.GetCurrentMuzzle().GetMaxAmmoCount();
		
		if (maxAmmo == 0)
		{
			Print("Division by 0! Weapon Max Ammo not set in " + this, LogLevel.ERROR);
			return;
		}
		
		float fillPercent = curAmmo / maxAmmo;
		m_wMagazineProgress.SetMaskProgress(fillPercent);
		
		m_wMagazineEmpty.SetVisible(curAmmo == 0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected EAmmoType GetWeaponCurrentRocket(BaseWeaponComponent weapon)
	{
		SCR_RocketEjectorMuzzleComponent muzzle = SCR_RocketEjectorMuzzleComponent.Cast(weapon.GetCurrentMuzzle());
		if (!muzzle)
			return 0;
		
		IEntity rocket = muzzle.GetBarrelProjectile(muzzle.GetCurrentBarrelIndex());
		if (!rocket)
			return 0;
		
		MagazineComponent magInfoComp = MagazineComponent.Cast(rocket.FindComponent(MagazineComponent));
		if (!magInfoComp)
			return 0;
			
		MagazineUIInfo rocketInfoUI = MagazineUIInfo.Cast(magInfoComp.GetUIInfo());
		if (!rocketInfoUI)
			return 0;
		
		return rocketInfoUI.GetAmmoTypeFlags();
	}
}