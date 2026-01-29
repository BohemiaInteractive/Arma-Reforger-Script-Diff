class SCR_TaskData
{
	string m_sID;
	ref SCR_TaskUIInfo m_UIInfo;
	vector m_vPosition;
	
	int m_iAuthorId;
	
	ref array<ref SCR_TaskExecutor> m_aAssignees;
	ref array<string> m_aOwnerFactionKeys;
	ref array<int> m_aOwnerGroupIDs;
	ref array<ref SCR_TaskExecutor> m_aOwnerExecutors;
	
	SCR_ETaskState m_eState;
	SCR_ETaskOwnership m_eOwnership;
	SCR_ETaskVisibility m_eVisibility;
	SCR_ETaskUIVisibility m_eUIVisibility;
	
	// Diag
	bool diag_isChecked = false;
	ref array<bool> diag_checkedSettings = 
	{
		false,	// Ctx Menu: Name
		false,  // Ctx Menu: Description
		false,	// Ctx Menu: State
		false,	// Ctx Menu: Ownership
		false,	// Ctx Menu: Visibility
		false,	// Ctx Menu: UI Visibility
		false,	// Ctx Menu: Assignees
		false,	// Ctx Menu: Owner Executors
		false,	// Ctx Menu: Owner Factions
		false,	// Ctx Menu: Owner Groups
		false 	// Ctx Menu: Position
	};
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] data
	void Copy(out SCR_TaskData data)
	{
		if (!data)
			data = new SCR_TaskData();
		
		data.m_sID = m_sID;
		
		data.m_UIInfo = new SCR_TaskUIInfo();
		data.m_UIInfo.CopyFrom(m_UIInfo);
			
		data.m_vPosition = m_vPosition;
		
		data.m_iAuthorId = m_iAuthorId;

		if (m_aAssignees && !m_aAssignees.IsEmpty())
		{
			data.m_aAssignees = {};
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aAssignees, data.m_aAssignees);
		}
		
		if (m_aOwnerFactionKeys && !m_aOwnerFactionKeys.IsEmpty())
		{
			data.m_aOwnerFactionKeys = {};
			data.m_aOwnerFactionKeys.Copy(m_aOwnerFactionKeys);
		}
		
		if (m_aOwnerGroupIDs && !m_aOwnerGroupIDs.IsEmpty())
		{
			data.m_aOwnerGroupIDs = {};
			data.m_aOwnerGroupIDs.Copy(m_aOwnerGroupIDs);
		}
		
		if (m_aOwnerExecutors && !m_aOwnerExecutors.IsEmpty())
		{
			data.m_aOwnerExecutors = {};
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aOwnerExecutors, data.m_aOwnerExecutors);
		}
		
		data.m_eState = m_eState;
		data.m_eOwnership = m_eOwnership;
		data.m_eVisibility = m_eVisibility;
		data.m_eUIVisibility = m_eUIVisibility;
	}
}
