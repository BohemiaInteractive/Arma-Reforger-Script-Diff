[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveGroupBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to stop bleeding from", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.RemoveGroupBleeding(m_eCharacterHitZoneGroup);
	}
}