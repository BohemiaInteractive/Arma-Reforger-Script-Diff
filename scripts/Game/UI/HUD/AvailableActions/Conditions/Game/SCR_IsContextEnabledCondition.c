[BaseContainerProps()]
class SCR_IsContextEnabledCondition : SCR_AvailableActionCondition
{
	[Attribute("")]
	protected string m_sContext;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		bool result = GetGame().GetInputManager().IsContextActive(m_sContext);
		return GetReturnResult(result);
	}
}
