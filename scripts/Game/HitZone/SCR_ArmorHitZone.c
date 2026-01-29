//------------------------------------------------------------------------------------------------
class SCR_ArmorHitZone : SCR_HitZone
{
	[Attribute(ECharacterHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;	
};