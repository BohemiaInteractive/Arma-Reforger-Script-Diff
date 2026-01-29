[BaseContainerProps()]
class SCR_AIGroupAutonomyCheck : SCR_AIGroupConditionCheck
{
	protected SCR_AIGroupInfoComponent m_GroupInfo;

	//------------------------------------------------------------------------------------------------
	override bool CheckCondition(SCR_AIGroup group)
	{
		if (!super.CheckCondition(group))
			return false;

		if (!m_GroupInfo)
		{
			m_GroupInfo = SCR_AIGroupInfoComponent.Cast(group.FindComponent(SCR_AIGroupInfoComponent));
			if (!m_GroupInfo)
				return true;
		}

		if (m_GroupInfo.GetGroupControlMode() == EGroupControlMode.AUTONOMOUS)
			return false;

		return true;
	}
}
