[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkDamageContextHitZoneGroupCharacter : SCR_ScenarioFrameworkDamageContextHitZoneGroupBase
{
	[Attribute(defvalue: "0", desc: "Hit Zone Group", uiwidget: UIWidgets.ComboBox, enumType: ECharacterHitZoneGroup)]
	ECharacterHitZoneGroup m_eHitZoneGroup;
	
	//------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}
}