//! Returns true if character is freelooking with specified conditions
[BaseContainerProps()]
class SCR_CharacterFreelookCondition : SCR_AvailableActionCondition
{
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Available only if freelook is toggled")]
	protected bool m_bToggled;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Available only if freelook is not toggled")]
	protected bool m_bNotToggled;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Available only if freelook is forced")]
	protected bool m_bForced;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Available only if freelook is not forced")]
	protected bool m_bNotForced;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is freelooking
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		bool result = data.GetIsCharacterFreelook();

		if (result)
		{
			bool toggled = data.GetIsCharacterFreelookToggled();
			if (m_bToggled)
				result = result && toggled;

			if (m_bNotToggled)
				result = result && !toggled;

			bool forced = data.GetIsCharacterFreelookForced();
			if (m_bForced)
				result = result && forced;

			if (m_bNotForced)
				result = result && !forced;
		}

		return GetReturnResult(result);
	}
}
