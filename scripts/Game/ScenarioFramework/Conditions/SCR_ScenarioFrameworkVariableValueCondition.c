[BaseContainerProps()]
class SCR_ScenarioFrameworkVariableValueCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Name of the variable")]
	string m_sVariableName;

	[Attribute(desc: "Check if the variable has set this value or not.")]
	string m_sVariableValueToCheck;

	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkVariableValueCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return false;
		
		string outValue;
		scenarioFrameworkSystem.GetVariable(m_sVariableName, outValue);
		
		if (outValue.IsEmpty())
			return false;
		
		return outValue == m_sVariableValueToCheck;
	}
}