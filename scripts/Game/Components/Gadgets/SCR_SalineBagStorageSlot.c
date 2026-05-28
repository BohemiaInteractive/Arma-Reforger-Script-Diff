class SCR_SalineBagStorageSlot : SCR_EquipmentStorageSlot
{
	[Attribute("", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup), desc: "Which hitzone does this slot belong to", category: "Equipped consumables")]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ECharacterHitZoneGroup GetAssociatedHZGroup()
	{
		return m_eHitZoneGroup;
	}
}
