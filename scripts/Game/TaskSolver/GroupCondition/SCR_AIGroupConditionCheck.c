[BaseContainerProps()]
class SCR_AIGroupConditionCheck
{
	[Attribute()]
	protected ref SCR_UIInfo m_FailedConditionUIInfo;

	//------------------------------------------------------------------------------------------------
	//! Checks if provided SCR_AIGroup object is not null.
	//! \param[in] group entity that is checked
	//! \return true if group is not null, false otherwise.
	bool CheckCondition(SCR_AIGroup group)
	{
		return group != null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetFailedConditionUIInfo()
	{
		return m_FailedConditionUIInfo;
	}
}
