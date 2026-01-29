class SCR_ExtendedTaskData : SCR_TaskData
{
	RplId m_ParentTask;
	int m_iNodeDepth;
	
	ref array<RplId> m_aChildTasks;
	
	bool m_bLinkDataToRelatedTasks;
	
	float m_fProgress;
	bool m_bEnableProgress;
	
	LocalizedString m_sCustomSubtaskDescription;
	
	// Diag
	ref array<bool> diag_checkedExtendedSettings = 
	{
		false,	// Ctx Menu: Child Tasks
		false	// Ctx Menu: Progress
	};
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] data
	override void Copy(out SCR_TaskData data)
	{
		if (!data)
			data = new SCR_ExtendedTaskData();
		
		super.Copy(data);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(data);
		if (!extendedData)
			return;
		
		extendedData.m_ParentTask = m_ParentTask;
		extendedData.m_iNodeDepth = m_iNodeDepth;

		if (m_aChildTasks && !m_aChildTasks.IsEmpty())
		{
			extendedData.m_aChildTasks = {};
			extendedData.m_aChildTasks.Copy(m_aChildTasks);		
		}
		
		extendedData.m_bLinkDataToRelatedTasks = m_bLinkDataToRelatedTasks;
		
		extendedData.m_fProgress = m_fProgress;
		extendedData.m_bEnableProgress = m_bEnableProgress;
		
		extendedData.m_sCustomSubtaskDescription = m_sCustomSubtaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Used when tasks are parented; Unique values such as ID, name, etc. stay the same
	//! Values such as ownership, assignees etc. will be shared across child/parent task
	//! \param[in] otherData
	//! \return
	SCR_ExtendedTaskData Merge(SCR_ExtendedTaskData otherData)
	{
		SCR_TaskData data;
		otherData.Copy(data);
		
		SCR_ExtendedTaskData extendedData = SCR_ExtendedTaskData.Cast(data);
		if (!extendedData)
			return null;

		extendedData.m_aAssignees = {};
		if (m_aAssignees && !m_aAssignees.IsEmpty())
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aAssignees, extendedData.m_aAssignees);
		
		extendedData.m_aOwnerFactionKeys = {};
		if (m_aOwnerFactionKeys && !m_aOwnerFactionKeys.IsEmpty())
			extendedData.m_aOwnerFactionKeys.Copy(m_aOwnerFactionKeys);
		
		extendedData.m_aOwnerGroupIDs = {};
		if (m_aOwnerGroupIDs && !m_aOwnerGroupIDs.IsEmpty())
			extendedData.m_aOwnerGroupIDs.Copy(m_aOwnerGroupIDs);
		
		extendedData.m_aOwnerExecutors = {};
		if (m_aOwnerExecutors && !m_aOwnerExecutors.IsEmpty())
			SCR_ArrayHelperRefT<ref SCR_TaskExecutor>.CopyReferencesFromTo(m_aOwnerExecutors, extendedData.m_aOwnerExecutors);
		
		extendedData.m_eState = m_eState;
		extendedData.m_eOwnership = m_eOwnership;
		extendedData.m_eVisibility = m_eVisibility;
		
		return extendedData;
	}
}
