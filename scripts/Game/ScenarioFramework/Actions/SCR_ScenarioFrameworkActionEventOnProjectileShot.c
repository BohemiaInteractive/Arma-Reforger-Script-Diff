[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionEventOnProjectileShot : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to watch for firing.")]
	ref SCR_ScenarioFrameworkGet m_Getter; 
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Activation Conditions", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Activation")]
	SCR_EScenarioFrameworkLogicOperators m_eActivationConditionLogic;
		
	[Attribute(desc: "Conditions to evaluate on projectile fired")]
	ref array<ref SCR_ScenarioFrameworkOnProjectileShotConditionBase> m_aProjectileConditions;

	[Attribute(desc: "Actions to be executed if conditions' evaluation is successful.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
		
	protected CharacterWeaponManagerComponent characterWeaponManager;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		EventHandlerManagerComponent eventHandler = EventHandlerManagerComponent.Cast(entity.FindComponent(EventHandlerManagerComponent));
		if (!eventHandler)
			return;
		
		eventHandler.RegisterScriptHandler("OnProjectileShot", entity, OnWeaponFired);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeaponFired(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{	
		if (!EvaluateConditions(playerID, weapon, entity))
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(entity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditions(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{	
		switch (m_eActivationConditionLogic)
		{
			case SCR_EScenarioFrameworkLogicOperators.AND:
				return EvaluateConditionsAND(playerID, weapon, entity);

			case SCR_EScenarioFrameworkLogicOperators.OR:
				return EvaluateConditionsOR(playerID, weapon, entity);

			case SCR_EScenarioFrameworkLogicOperators.NOT:
				return EvaluateConditionsNAND(playerID, weapon, entity);

			case SCR_EScenarioFrameworkLogicOperators.XOR:
				return EvaluateConditionsXOR(playerID, weapon, entity);
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsAND(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (m_aProjectileConditions.IsEmpty())
			return true;

		foreach (SCR_ScenarioFrameworkOnProjectileShotConditionBase action : m_aProjectileConditions)
		{
			if (!action.Init(playerID, weapon, entity))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsOR(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		foreach (SCR_ScenarioFrameworkOnProjectileShotConditionBase action : m_aProjectileConditions)
		{
			if (action.Init(playerID, weapon, entity))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsNAND(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		return !EvaluateConditionsAND(playerID, weapon, entity);
	}

	//------------------------------------------------------------------------------------------------
	bool EvaluateConditionsXOR(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		bool priorSuccess = false;
		foreach (SCR_ScenarioFrameworkOnProjectileShotConditionBase action : m_aProjectileConditions)
		{
			bool success = action.Init(playerID, weapon, entity);
			if (priorSuccess && success)
				return false;

			priorSuccess |= success;
		}
		return priorSuccess;
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}