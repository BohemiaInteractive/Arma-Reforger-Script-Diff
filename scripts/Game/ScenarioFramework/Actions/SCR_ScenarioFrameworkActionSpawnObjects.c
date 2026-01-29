[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjects : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "These objects will spawn once the trigger becomes active.")]
	ref array<string> 	m_aNameOfObjectsToSpawnOnActivation;
	
	[Attribute(defvalue: SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION.ToString(), uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType))]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SpawnObjects(m_aNameOfObjectsToSpawnOnActivation, m_eActivationType);
	}
}