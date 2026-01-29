[BaseContainerProps(insertable: false)]
class SCR_FieldManualPiece
{
	[Attribute(defvalue: SCR_EInputTypeCondition.ALL_INPUTS.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Define if this piece should be displayed for keyboard users, gamepad users or both", enums: ParamEnumArray.FromEnum(SCR_EInputTypeCondition))]
	protected SCR_EInputTypeCondition m_eInputDisplayCondition;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] parent
	// To be overridden by child pieces
	void CreateWidget(notnull Widget parent);

	//------------------------------------------------------------------------------------------------
	//! Check if the widget can be created (input method filter or anything else)
	//! \return true if the widget corresponds to the input method, false if not
	bool CanCreateWidget()
	{
		// I am very proud of this ugly one-liner (that is technically a two-liner)
		return m_eInputDisplayCondition == SCR_EInputTypeCondition.ALL_INPUTS ||
			GetGame().GetInputManager().IsUsingMouseAndKeyboard() == (m_eInputDisplayCondition == SCR_EInputTypeCondition.KEYBOARD_ONLY);
	}

	//------------------------------------------------------------------------------------------------
	//! Used to check if the entry requires a refresh (on e.g KBM/pad input change)
	//! \return true if an update can be used, false otherwise
	bool CanRefresh()
	{
		return m_eInputDisplayCondition != SCR_EInputTypeCondition.ALL_INPUTS;
	}
}
