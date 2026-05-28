[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionInputOnControllableDestroyed : SCR_ScenarioFrameworkActionInputBase
{
	[Attribute(desc: "Checked Instigator Conditions.")]
	ref array<ref SCR_ScenarioFrameworkInstigatorContextConditionBase> m_aInstigatorContextConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Damage Conditions", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Activation")]
	SCR_EScenarioFrameworkLogicOperators m_eActivationConditionLogic;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_ScenarioFrameworkLogicInput input)
	{
		super.Init(input);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		ScriptInvokerBase<SCR_BaseGameMode_OnControllableDestroyed> instigator = gameMode.GetOnControllableDestroyed();
		if (instigator)
			instigator.Insert(OnControllableDestroyed);
	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnControllableDestroyed(SCR_InstigatorContextData insigatorData)
	{
		if (m_aInstigatorContextConditions && !m_aInstigatorContextConditions.IsEmpty())
		{
			if (EvaluateConditions(insigatorData))
				m_Input.OnActivate(1, null);
			
			return;
		}
		
		//No conditions set, activate whenever controllable is killed
		m_Input.OnActivate(1, null);
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditions(SCR_InstigatorContextData insigatorData)
	{	
		switch (m_eActivationConditionLogic)
		{
			case SCR_EScenarioFrameworkLogicOperators.AND:
				return EvaluateConditionsAND(insigatorData);

			case SCR_EScenarioFrameworkLogicOperators.OR:
				return EvaluateConditionsOR(insigatorData);

			case SCR_EScenarioFrameworkLogicOperators.NOT:
				return EvaluateConditionsNAND(insigatorData);

			case SCR_EScenarioFrameworkLogicOperators.XOR:
				return EvaluateConditionsXOR(insigatorData);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsAND(SCR_InstigatorContextData insigatorData)
	{
		foreach (SCR_ScenarioFrameworkInstigatorContextConditionBase action : m_aInstigatorContextConditions)
		{
			if (!action.Init(insigatorData))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsOR(SCR_InstigatorContextData insigatorData)
	{		
		foreach (SCR_ScenarioFrameworkInstigatorContextConditionBase action : m_aInstigatorContextConditions)
		{
			if (action.Init(insigatorData))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsNAND(SCR_InstigatorContextData insigatorData)
	{
		return !EvaluateConditionsAND(insigatorData);
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsXOR(SCR_InstigatorContextData insigatorData)
	{
		bool priorSuccess = false;
		foreach (SCR_ScenarioFrameworkInstigatorContextConditionBase action : m_aInstigatorContextConditions)
		{
			bool success = action.Init(insigatorData);
			if (priorSuccess && success)
				return false;

			priorSuccess |= success;
		}
		return priorSuccess;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ScenarioFrameworkActionInputOnControllableDestroyed()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		
		ScriptInvokerBase<SCR_BaseGameMode_OnControllableDestroyed> instigator = gameMode.GetOnControllableDestroyed();
		if (instigator)
			instigator.Remove(OnControllableDestroyed);
	}
}