class SCR_ScenarioFrameworkTaskData : SCR_ExtendedTaskData
{
	bool	m_bCustomBriefing;
	string 	m_sTaskExecutionBriefing;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] data
	override void Copy(out SCR_TaskData data)
	{
		if (!data)
			data = new SCR_ScenarioFrameworkTaskData();
		
		super.Copy(data);
		
		SCR_ScenarioFrameworkTaskData extendedData = SCR_ScenarioFrameworkTaskData.Cast(data);
		if (!extendedData)
			return;
		
		extendedData.m_bCustomBriefing = m_bCustomBriefing;
		extendedData.m_sTaskExecutionBriefing = m_sTaskExecutionBriefing;
	}
}
