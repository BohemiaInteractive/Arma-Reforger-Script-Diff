class SCR_VehicleHitZone : SCR_DestructibleHitzone
{
	[Attribute(EVehicleHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleHitZoneGroup))]
	protected EVehicleHitZoneGroup m_eHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}
}
