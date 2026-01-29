[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBasedOnConditions : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Action activation conditions")]
	protected ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aActivationConditions;

	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Activation Conditions", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Activation")]
	SCR_EScenarioFrameworkLogicOperators m_eActivationConditionLogic;

	[Attribute(desc: "Actions to be executed if conditions' evaluation is successful.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	[Attribute(desc: "Actions to be executed if conditions' evaluation is failed.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aFailedActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eActivationConditionLogic, m_aActivationConditions, object))
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(object);
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aFailedActions)
			{
				actions.OnActivate(object);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase> allActions = {};
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
		{
			allActions.Insert(action);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aFailedActions)
		{
			allActions.Insert(action);
		}
		
		return allActions;
	}
}
