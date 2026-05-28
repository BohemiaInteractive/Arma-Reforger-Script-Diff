[BaseContainerProps()]
class SCR_IsActionTriggeredCondition : SCR_AvailableActionCondition
{
	[Attribute("")]
	protected string m_sAction;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(GetGame().GetInputManager().GetActionTriggered(m_sAction));
	}
}
