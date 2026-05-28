[BaseContainerProps()]
class SCR_ConditionCheckUIEntry
{
	[Attribute()]
	protected ref SCR_UIInfo m_FailedConditionUIInfo;

	protected int m_iConditionType = -1;

	//------------------------------------------------------------------------------------------------
	int GetConditionType()
	{
		return m_iConditionType;
	}

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetFailedConditionUIInfo()
	{
		return m_FailedConditionUIInfo;
	}
}
