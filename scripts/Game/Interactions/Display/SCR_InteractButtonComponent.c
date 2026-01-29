class SCR_ActionMenuInputButtonComponent : SCR_InputButtonComponent
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] state should this action be a hold action ?
	//! \param[in] duration How long should the Hold duration be ? 
	void SetHoldAction(bool state, float duration = 1)
	{
		m_bIsHoldAction = state;
		m_fMaxHoldtime = duration;

		m_ButtonDisplay.ForceSetHoldAction(m_bIsHoldAction);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state force the button to be held
	void ForceSetHoldAction(bool state)
	{
		m_ButtonDisplay.ActionPressed(state);
	}
	
	override protected void CreateComboWidget()
	{
	}
}
