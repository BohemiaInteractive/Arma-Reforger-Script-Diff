[BaseContainerProps()]
class SCR_AIGroupConditionManager
{
	[Attribute()]
	protected ref array<ref SCR_AIGroupConditionCheck> m_aChecks;

	protected bool m_bIsGroupAvailable;

	protected ref ScriptInvokerBool m_OnAIGroupAvailabilityChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnAIGroupAvailabilityChanged()
	{
		if (!m_OnAIGroupAvailabilityChanged)
			m_OnAIGroupAvailabilityChanged = new ScriptInvokerBool();

		return m_OnAIGroupAvailabilityChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroupConditionCheck GetCheck(typename type)
	{
		foreach (SCR_AIGroupConditionCheck check : m_aChecks)
		{
			if (check.Type() == type)
				return check;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void AddCheck(notnull SCR_AIGroupConditionCheck check)
	{
		if (m_aChecks.Contains(check))
			return;

		m_aChecks.Insert(check);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if an AI group is available by iterating through condition checks.
	//! \param[in] group of AI entities that the method checks availability for.
	//! \return Availability of the group after checking all conditions.
	bool IsAIGroupAvailable(SCR_AIGroup group)
	{
		foreach (SCR_AIGroupConditionCheck check : m_aChecks)
		{
			if (!check.CheckCondition(group))
			{
				SetIsAvailable(false);
				return m_bIsGroupAvailable;
			}
		}

		SetIsAvailable(true);
		return m_bIsGroupAvailable;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsAvailable()
	{
		return m_bIsGroupAvailable;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIsAvailable(bool value)
	{
		if (m_bIsGroupAvailable == value)
			return;

		m_bIsGroupAvailable = value;

		if (m_OnAIGroupAvailabilityChanged)
			m_OnAIGroupAvailabilityChanged.Invoke(m_bIsGroupAvailable);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AIGroupConditionManager()
	{
		if (!m_aChecks)
			m_aChecks = {};
	}
}
