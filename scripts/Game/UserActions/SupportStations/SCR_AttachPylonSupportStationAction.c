class SCR_AttachPylonSupportStationAction : SCR_BaseItemHolderSupportStationAction
{	
	[Attribute("0", params: "0 inf")]
	protected int m_iPylonIndex;

	protected WeaponSlotComponent m_WeaponSlot;
	
	//------------------------------------------------------------------------------------------------
	WeaponSlotComponent GetLinkedWeaponSlot()
	{
		return m_WeaponSlot;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{	
		//~ Not valid or already occupied
		if (m_sItemPrefab.IsEmpty() || !m_WeaponSlot || m_WeaponSlot.GetWeaponEntity())
			return false;
		
		return super.CanBeShownScript(user);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(IEntity owner)
	{
		if (!owner)
			return;
		
		super.DelayedInit(owner);
		
		SCR_ResupplyItemSupportStationData itemData = SCR_ResupplyItemSupportStationData.Cast(m_ResupplyData);
		if (!itemData)
			return;
		
		m_sItemPrefab = itemData.GetItemPrefab();
		
		SetItemName();
		
		array<Managed> weaponSlots = {};
		GetOwner().FindComponents(WeaponSlotComponent, weaponSlots);
		WeaponSlotComponent weaponSlot;
		
		foreach (Managed slot : weaponSlots)
		{
			weaponSlot = WeaponSlotComponent.Cast(slot);
			if (!weaponSlot)
				continue;
			
			if (weaponSlot.GetWeaponSlotIndex() == m_iPylonIndex)
				m_WeaponSlot = weaponSlot;
		}
		
		if (!m_WeaponSlot)
			Print("SCR_AttachPylonSupportStationAction: Could not find weapon slot!", LogLevel.ERROR);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override ESupportStationType GetSupportStationType()
	{
		return ESupportStationType.VEHICLE_WEAPON;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
			return super.GetActionNameScript(outName);
					
		outName = WidgetManager.Translate(uiInfo.GetName(), m_sItemName);
		
		return super.GetActionNameScript(outName);
	}
}