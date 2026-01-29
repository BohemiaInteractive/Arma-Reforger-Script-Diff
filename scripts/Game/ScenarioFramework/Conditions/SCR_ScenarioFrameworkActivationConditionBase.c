[BaseContainerProps()]
class SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "If set to true, when this Condition performs certain methods, it will break the breakpoint in the Script Editor in respective methods. This can be also set during runtime via Debug Menu > ScenarioFramework > Condition Inspector")]
	bool m_bDebug;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	bool Init(IEntity entity);
	
	//------------------------------------------------------------------------------------------------
	//! Runs logical operator on activationConditions.
	//! Operators exit early when appropriate.
	//! Each operator impliments its own function and loop for performance.
	//! \param[in] operator Operators to apply on whole array. SCR_EScenarioFrameworkLogicOperators.NOT is interpreted as NAND.
	//! \param[in] activationConditions Conditions that succeed or fail. Empty array returns true.
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return If there are no conditions, returns true. Otherwise delegates to relevent logic funtion.
	static bool EvaluateEmptyOrConditions(SCR_EScenarioFrameworkLogicOperators operator, notnull array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		if (activationConditions.IsEmpty())
			return true;
		return EvaluateConditions(operator, activationConditions, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Runs logical operator on activationConditions.
	//! Operators exit early when appropriate.
	//! Each operator impliments its own function and loop for performance.
	//! \param[in] operator Operators to apply on whole array. SCR_EScenarioFrameworkLogicOperators.NOT is interpreted as NAND.
	//! \param[in] activationConditions For logical consistency undefined (empty array) is interpreted as false. Therefore *most* use-cases should use EvaluateEmptyOrConditions` 
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return Delegates to relevent logic funtion.
	static bool EvaluateConditions(SCR_EScenarioFrameworkLogicOperators operator, notnull array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		switch (operator)
		{
			case SCR_EScenarioFrameworkLogicOperators.AND:
				return EvaluateConditionsAND(activationConditions, entity);

			case SCR_EScenarioFrameworkLogicOperators.OR:
				return EvaluateConditionsOR(activationConditions, entity);

			case SCR_EScenarioFrameworkLogicOperators.NOT:
				return EvaluateConditionsNAND(activationConditions, entity);

			case SCR_EScenarioFrameworkLogicOperators.XOR:
				return EvaluateConditionsXOR(activationConditions, entity);
		}

		PrintFormat("[SCR_ScenarioFrameworkActionBasedOnConditions] Unknown operator: %1", operator, level: LogLevel.ERROR);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Runs logical AND on result of activationConditions.
	//! Exits early of first failure.
	//! \param[in] activationConditions Conditions that succeed or fail. Empty array returns true.
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return false if activationConditions is empty or if one condition was unsuccessful.
	//! \return true if all succeed. false if activationConditions is empty.
	static bool EvaluateConditionsAND(array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		if (activationConditions.IsEmpty())
			return false;

		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : activationConditions)
		{
			if (!activationCondition.Init(entity))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Runs logical OR on result of activationConditions.
	//! Exits early of first success.
	//! \param[in] activationConditions Conditions that succeed or fail. Empty array returns true.
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return true if any succeed. false if activationConditions is empty.
	static bool EvaluateConditionsOR(array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : activationConditions)
		{
			if (activationCondition.Init(entity))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Runs logical NAND on result of activationConditions. At least one condition must be false.
	//! Exits early of first failure.
	//! Fun fact, you can construct every binary logic gate with only NAND gates.
	//! \param[in] activationConditions Conditions that succeed or fail. Empty array returns true.
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return true if activationConditions is empty or if one condition was unsuccessful.
	static bool EvaluateConditionsNAND(array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		return !EvaluateConditionsAND(activationConditions, entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Array style XOR. Exactly one condition has to succeed, otherwise false is returned.
	//! Exits early on second success.
	//! \param[in] activationConditions Conditions that succeed or fail. Empty array returns true.
	//! \param[in] entity LayerBase or trigger that conditions are registered on.
	//! \return false if empty or if more than one condition was successful. true if Exactly one condition succeeded.
	static bool EvaluateConditionsXOR(array<ref SCR_ScenarioFrameworkActivationConditionBase> activationConditions, IEntity entity)
	{
		bool priorSuccess = false;
		foreach (SCR_ScenarioFrameworkActivationConditionBase activationCondition : activationConditions)
		{
			bool success = activationCondition.Init(entity);
			if (priorSuccess && success)
				return false;

			priorSuccess |= success;
		}
		return priorSuccess;
	}
}
