[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnCharacterEnteredOrLeftCompartment : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If true, we execute actions On Compartmented Entered. Otherwise On Compartment Left")]
	bool m_bEnteredOrLeft;

	[Attribute(desc: "Target entity (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "(Optional) If used, it will get executed only when specific entity enters the compartment")]
	ref SCR_ScenarioFrameworkGet m_OccupantGetter;
	
}