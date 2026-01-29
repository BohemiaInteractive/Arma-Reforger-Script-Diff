[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetPermitUnconsciousness : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed for this particular character")]
	bool m_bPermitUnconsciousness;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.SetPermitUnconsciousness(m_bPermitUnconsciousness, true);
	}
}