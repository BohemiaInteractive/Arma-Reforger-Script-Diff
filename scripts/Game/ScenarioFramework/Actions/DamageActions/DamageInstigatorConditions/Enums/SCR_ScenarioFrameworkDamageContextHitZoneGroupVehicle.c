[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextHitZoneGroupVehicle : SCR_ScenarioFrameworkDamageContextHitZoneGroupBase
{
	[Attribute(defvalue: "0", desc: "Hit Zone Group", uiwidget: UIWidgets.ComboBox, enumType: EVehicleHitZoneGroup)]
	EVehicleHitZoneGroup m_eHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}
}