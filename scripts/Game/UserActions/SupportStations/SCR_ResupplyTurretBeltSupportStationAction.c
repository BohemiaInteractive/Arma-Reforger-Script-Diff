class SCR_ResupplyTurretBeltSupportStationAction : SCR_ResupplyVehicleWeaponSupportStationAction
{
	[Attribute("0", desc: "WeaponSlotComponent weapon slot index")]
	protected int m_iWeaponSlotID;	
	
	[Attribute("15", desc: "Bullets added to belt per loop")]
	protected int m_iAddBulletsAmount;
	
	protected BaseMuzzleComponent m_GunMuzzleComp;
	protected BaseMagazineComponent m_Magazine;
	
	protected int m_iMaxAmmoCount;
	protected int m_iCurrentAmmoCount;
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		//~ Not valid or has no parent
		if (!m_Magazine)
			return false;
		
		return super.CanBeShownScript(user);		
	}
		
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_Magazine || !super.CanBePerformedScript(user))
			return false;
		
		m_iMaxAmmoCount = m_Magazine.GetMaxAmmoCount();
		m_iCurrentAmmoCount = m_Magazine.GetAmmoCount();
		
		if (m_iCurrentAmmoCount >= m_iMaxAmmoCount)
		{
			SetCanPerform(false, ESupportStationReasonInvalid.RESUPPLY_INVENTORY_FULL);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseMagazineComponent GetCurrentMagazine()
	{
		return m_Magazine;
	}	
	
	//------------------------------------------------------------------------------------------------
	int GetAddedBulletsAmount()
	{
		return m_iAddBulletsAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		super.DelayedInit(owner);
		
		array<Managed> slots = {};
		
		GetOwner().FindComponents(WeaponSlotComponent, slots);
		
		WeaponSlotComponent weaponSlot;
		array<BaseMuzzleComponent> muzzles = {};
		
		foreach (Managed slot : slots)
		{
			muzzles.Clear();
			weaponSlot = WeaponSlotComponent.Cast(slot);
			
			if (weaponSlot.GetWeaponSlotIndex() != m_iWeaponSlotID)
				continue;
			
			weaponSlot.GetMuzzlesList(muzzles);
			if (muzzles.IsEmpty())
				return;
			
			m_GunMuzzleComp = muzzles[0];
			
			if (m_GunMuzzleComp)
				break;
		}
		
		if (!m_GunMuzzleComp)
			return;
		
		m_Magazine = m_GunMuzzleComp.GetMagazine();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override string GetActionStringParam()
	{		
		if (!m_bCanPerform)
			return string.Empty;

		return WidgetManager.Translate(X_OUTOF_Y_FORMATTING, m_iCurrentAmmoCount, m_iMaxAmmoCount);
	}
}