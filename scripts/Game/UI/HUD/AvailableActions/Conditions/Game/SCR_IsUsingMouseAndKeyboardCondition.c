[BaseContainerProps()]
class SCR_IsUsingMouseAndKeyboardCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}
}
