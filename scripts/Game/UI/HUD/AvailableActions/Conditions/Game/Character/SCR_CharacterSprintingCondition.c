[BaseContainerProps()]
class SCR_CharacterSprintingCondition : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.EditBox, "Checks if sprint was toggled rather than checking if the button is being held.")]
	protected bool m_bCheckPersistentState;

	//------------------------------------------------------------------------------------------------
	//! Returns true if character is sprinting.
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetIsCharacterSprinting();
		result = result && (!m_bCheckPersistentState && data.GetCharacterSprintingTime() != 0 || m_bCheckPersistentState && data.GetCharacterSprintingTime() == 0);

		return GetReturnResult(result);
	}
}
