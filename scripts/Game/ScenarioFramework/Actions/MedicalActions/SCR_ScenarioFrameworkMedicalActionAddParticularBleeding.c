[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddParticularBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(desc: "Which hit zone will start bleeding", uiwidget: UIWidgets.EditComboBox, enums: SCR_AttributesHelper.ParamFromTitles("Health;Blood;Resilience;Head;Chest;Abdomen;Hips;RArm;LArm;RForearm;LForearm;RHand;LHand;RThigh;LThigh;RCalf;LCalf;RFoot;LFoot;Neck"))];
	string m_sHitZoneName;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.AddParticularBleeding(m_sHitZoneName);
	}
}