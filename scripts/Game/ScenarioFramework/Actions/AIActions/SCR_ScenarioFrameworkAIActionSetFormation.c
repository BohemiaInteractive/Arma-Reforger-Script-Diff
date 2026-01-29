[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionSetFormation : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "Common")]
	SCR_EAIGroupFormation m_eAIGroupFormation;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (!formComp)
		{
			Print(string.Format("ScenarioFramework Action: AI Formation Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, m_eAIGroupFormation));
	}
}