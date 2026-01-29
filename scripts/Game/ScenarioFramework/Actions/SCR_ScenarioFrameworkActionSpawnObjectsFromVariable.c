[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSpawnObjectsFromVariable : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "", UIWidgets.EditComboBox, desc: "These variables will be checked for object names once the trigger becomes active.")]
	ref array<string> 	m_aNameOfVariablesToSpawnObjectsFromOnActivation;
	
	[Attribute(defvalue: SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION.ToString(), uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType))]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		string value;
		array <string> names = {};
		foreach (string variable : m_aNameOfVariablesToSpawnObjectsFromOnActivation)
		{
			scenarioFrameworkSystem.GetVariable(variable, value);
			
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(value))
				continue;
			
			names.Insert(value);
		}
		
		SpawnObjects(names, m_eActivationType);
	}
}