//! A single available action condition representation
[BaseContainerProps()]
class SCR_AvailableActionCondition
{	
	[Attribute("1", UIWidgets.CheckBox)]
	protected bool m_bEnabled;	

	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bNegateCondition;
	
	//------------------------------------------------------------------------------------------------
	//! Getter for an enabled flag, disabled conditions are skipped, not evaluated at all
	//! \return true if enabled, false otherwise
	bool IsEnabled()
	{
		return m_bEnabled;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Based on the state of m_bNegateCondition returns our desired result
	//! \return desiredResult if !m_bNegateCondition, !desiredResult if m_bNegateCondition
	protected bool GetReturnResult(bool desiredResult)
	{
		if (m_bNegateCondition)
			return !desiredResult;
		
		return desiredResult;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override and implement this method in any inherited conditions.
	//! Use GetReturnResult prior to returning the value to take m_bNegateCondition into account
	//! \param data
	//! \return availability
	bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(true);
	}
}
