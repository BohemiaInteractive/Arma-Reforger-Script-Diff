[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnDamage : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to check for damage.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "Checked Conditions.")]
	ref array<ref SCR_ScenarioFrameworkDamageContextConditionBase> m_aDamageContextConditions;
	
	[Attribute(desc: "Actions to be executed if conditions' evaluation is successful.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Damage Conditions", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Activation")]
	SCR_EScenarioFrameworkLogicOperators m_eActivationConditionLogic;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		if (!ValidateInputEntity(object, m_Getter, m_Entity))
			return;
		
		AIGroup group = AIGroup.Cast(m_Entity);
		if (!group)
		{
			SetupInvoker(m_Entity);
			return;
		}
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		IEntity controlledEntity;
		
		foreach (AIAgent agent : agents)
		{
			controlledEntity = agent.GetControlledEntity();
			if (controlledEntity)
				SetupInvoker(controlledEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetupInvoker(IEntity object)
	{
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(object);
		if (!objectDmgManager)
			return;
		
		objectDmgManager.GetOnDamage().Remove(OnDamage);
		objectDmgManager.GetOnDamage().Insert(OnDamage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnDamage(BaseDamageContext damageContext)
	{
		if (!damageContext)
			return;
		
		if (!EvaluateConditions(damageContext))
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
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
		
		return allActions;
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditions(BaseDamageContext damageContext)
	{	
		switch (m_eActivationConditionLogic)
		{
			case SCR_EScenarioFrameworkLogicOperators.AND:
				return EvaluateConditionsAND(damageContext);

			case SCR_EScenarioFrameworkLogicOperators.OR:
				return EvaluateConditionsOR(damageContext);

			case SCR_EScenarioFrameworkLogicOperators.NOT:
				return EvaluateConditionsNAND(damageContext);

			case SCR_EScenarioFrameworkLogicOperators.XOR:
				return EvaluateConditionsXOR(damageContext);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsAND(BaseDamageContext damageContext)
	{
		if (m_aDamageContextConditions.IsEmpty())
			return false;

		foreach (SCR_ScenarioFrameworkDamageContextConditionBase action : m_aDamageContextConditions)
		{
			if (!action.Init(damageContext))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsOR(BaseDamageContext damageContext)
	{
		foreach (SCR_ScenarioFrameworkDamageContextConditionBase action : m_aDamageContextConditions)
		{
			if (action.Init(damageContext))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsNAND(BaseDamageContext damageContext)
	{
		return !EvaluateConditionsAND(damageContext);
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsXOR(BaseDamageContext damageContext)
	{
		bool priorSuccess = false;
		foreach (SCR_ScenarioFrameworkDamageContextConditionBase action : m_aDamageContextConditions)
		{
			bool success = action.Init(damageContext);
			if (priorSuccess && success)
				return false;

			priorSuccess |= success;
		}
		return priorSuccess;
	}
}