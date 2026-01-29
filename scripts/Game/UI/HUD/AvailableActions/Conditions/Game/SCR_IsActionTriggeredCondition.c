//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_IsActionTriggeredCondition : SCR_AvailableActionCondition
{
	[Attribute("")]
	protected string m_sAction;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		bool result = GetGame().GetInputManager().GetActionTriggered(m_sAction);
		return GetReturnResult(result);
	}
};
