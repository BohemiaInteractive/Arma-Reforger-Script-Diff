[BaseContainerProps()]
class SCR_ConditionCheckUIEntry
{
	[Attribute()]
	protected string m_sConditionCheckTypename;

	[Attribute()]
	protected ref SCR_UIInfo m_FailedConditionUIInfo;

	//------------------------------------------------------------------------------------------------
	string GetConditionCheckTypename()
	{
		return m_sConditionCheckTypename;
	}

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetFailedConditionUIInfo()
	{
		return m_FailedConditionUIInfo;
	}
}
