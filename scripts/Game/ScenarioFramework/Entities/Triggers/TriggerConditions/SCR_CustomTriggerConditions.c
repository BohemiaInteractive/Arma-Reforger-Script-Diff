//Baseclass that is supposed to be extended and filled with custom conditions
[BaseContainerProps()]
class SCR_CustomTriggerConditions : SCR_ScenarioFrameworkActivationConditionBase
{
	//------------------------------------------------------------------------------------------------
	//! Used to prepare the class data according to the needs of the condition
	//! \param[in] entity
	void Prepare(IEntity entity);
}