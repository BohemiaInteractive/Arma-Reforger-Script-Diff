class SCR_TransportUnitHelpTaskEntityClass : SCR_TaskClass
{
}

class SCR_TransportUnitHelpTaskEntity : SCR_Task
{
	protected SCR_TransportUnitComponent m_TransportUnit;

	//------------------------------------------------------------------------------------------------
	SCR_TransportUnitComponent GetTransportUnit()
	{
		return m_TransportUnit;
	}

	//------------------------------------------------------------------------------------------------
	void SetTransportUnit(SCR_TransportUnitComponent transportUnit)
	{
		if (m_TransportUnit == transportUnit)
			return;

		if (m_TransportUnit)
			m_TransportUnit.GetOnStateChanged().Remove(OnTransportUnitStateChanged);

		m_TransportUnit = transportUnit;
		if (!m_TransportUnit)
			return;

		m_TransportUnit.GetOnStateChanged().Insert(OnTransportUnitStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransportUnitStateChanged(SCR_ETransportUnitState state)
	{
		if (state == SCR_ETransportUnitState.ON_TASK)
		{
			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			m_TaskSystem.DeleteTask(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_TransportUnitHelpTaskEntity()
	{
		if (m_TransportUnit)
			m_TransportUnit.GetOnStateChanged().Remove(OnTransportUnitStateChanged);
	}
}
