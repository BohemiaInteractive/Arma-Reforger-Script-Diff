class SCR_ResupplyTaskSolverContext : SCR_TaskSolverContext
{
	protected Vehicle m_Vehicle;
	protected SCR_CampaignMilitaryBaseComponent m_SourceBase;

	//------------------------------------------------------------------------------------------------
	//! Retrieves the source base used for supplies loading.
	//! \return The source base.
	SCR_CampaignMilitaryBaseComponent GetSourceBase()
	{
		return m_SourceBase;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves the vehicle used for the resupply task.
	//! \return The vehicle associated with the task.
	Vehicle GetVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! Constructor to initialize the resupply task context.
	//! \param[in] vehicle The vehicle used in the task.
	//! \param[in] sourceBase The transport unit's source base.
	void SCR_ResupplyTaskSolverContext(notnull Vehicle vehicle, SCR_CampaignMilitaryBaseComponent sourceBase)
	{
		Init(vehicle, sourceBase);
	}

	//------------------------------------------------------------------------------------------------
	protected void Init(notnull Vehicle vehicle, SCR_CampaignMilitaryBaseComponent sourceBase)
	{
		m_Vehicle = vehicle;
		m_SourceBase = sourceBase;
	}
}
