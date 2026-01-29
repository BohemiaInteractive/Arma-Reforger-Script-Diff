class SCR_WeaponRackSlotInfo : RegisteringComponentSlotInfo
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Weapons rack that game should assume belongs to this slot by default", "et")]
	protected ResourceName m_sDefaultWeapon;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetDefaultWeaponRack()
	{
		return m_sDefaultWeapon;
	}
}