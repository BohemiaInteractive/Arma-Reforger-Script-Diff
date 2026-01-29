[BaseContainerProps()]
class SCR_VehicleConditionManager
{
	[Attribute()]
	protected ref array<ref SCR_VehicleConditionCheck> m_aChecks;

	protected bool m_bIsUsable;

	protected ref ScriptInvokerBool m_OnVehicleUsabilityChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnVehicleUsabilityChanged()
	{
		if (!m_OnVehicleUsabilityChanged)
			m_OnVehicleUsabilityChanged = new ScriptInvokerBool();

		return m_OnVehicleUsabilityChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_VehicleConditionCheck GetCheck(typename type)
	{
		foreach (SCR_VehicleConditionCheck check : m_aChecks)
		{
			if (check.Type() == type)
				return check;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void AddCheck(notnull SCR_VehicleConditionCheck check)
	{
		if (m_aChecks.Contains(check))
			return;

		m_aChecks.Insert(check);
	}

	//------------------------------------------------------------------------------------------------
	SCR_VehicleConditionCheck GetFirstFailedCheck(Vehicle vehicle)
	{
		foreach (SCR_VehicleConditionCheck check : m_aChecks)
		{
			if (!check.CheckCondition(vehicle))
			{
				return check;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if a vehicle is usable by iterating through condition checks.
	//! \param[in] vehicle that the method checks usability for.
	//! \return Usability of the group after checking all conditions.
	bool IsVehicleUsable(Vehicle vehicle)
	{
		foreach (SCR_VehicleConditionCheck check : m_aChecks)
		{
			if (!check.CheckCondition(vehicle))
			{
				SetIsUsable(false);
				return m_bIsUsable;
			}
		}

		SetIsUsable(true);
		return m_bIsUsable;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsUsable()
	{
		return m_bIsUsable;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetIsUsable(bool value)
	{
		if (m_bIsUsable == value)
			return;

		m_bIsUsable = value;

		if (m_OnVehicleUsabilityChanged)
			m_OnVehicleUsabilityChanged.Invoke(m_bIsUsable);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_VehicleConditionManager()
	{
		if (!m_aChecks)
			m_aChecks = {};
	}
}
