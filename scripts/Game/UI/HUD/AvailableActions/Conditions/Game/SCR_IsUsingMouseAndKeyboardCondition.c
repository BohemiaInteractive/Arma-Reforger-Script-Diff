[BaseContainerProps()]
class SCR_IsUsingMouseAndKeyboardCondition : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}
};
