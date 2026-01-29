class SCR_ScenarioFrameworkTaskClass : SCR_ExtendedTaskClass
{
};

class SCR_ScenarioFrameworkTask : SCR_ExtendedTask
{
	protected IEntity 												m_Asset;
	protected SCR_ScenarioFrameworkLayerTask						m_LayerTask;
	protected SCR_ScenarioFrameworkSlotTask							m_SlotTask;
	
	//------------------------------------------------------------------------------------------------
	SCR_ETaskNotificationSettings GetTaskNotificationSettings()
	{
		if (m_LayerTask)
			return m_LayerTask.m_eTaskNotificationSettings;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] layer Sets the layer task for this task.
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layer)
	{
		m_LayerTask = layer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Layer task linked to this task.
	SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		return m_LayerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Updates linked entity for this task.
	//! \param[in] object Updates blacklist for garbage system, sets task entity for support entity.
	void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		m_Asset = object;
		
		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Asset);
		if (garbageSystem)
			garbageSystem.UpdateBlacklist(m_Asset, true);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Asset entity.
	IEntity GetAsset()
	{
		return m_Asset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask that will be linked to this task.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return linked slot task.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text Sets the briefing for task execution.
	void SetTaskExecutionBriefing(string text)
	{
		Rpc_SetTaskExecutionBriefing(text);
		Rpc(Rpc_SetTaskExecutionBriefing, text);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetTaskExecutionBriefing(string text)
	{
		SCR_ScenarioFrameworkTaskData frameworkData = SCR_ScenarioFrameworkTaskData.Cast(m_TaskData);
		if (!frameworkData)
			return;
		
		frameworkData.m_bCustomBriefing = true;
		frameworkData.m_sTaskExecutionBriefing = text;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Task execution briefing string.
	string GetTaskExecutionBriefing()
	{ 
		SCR_ScenarioFrameworkTaskData frameworkData = SCR_ScenarioFrameworkTaskData.Cast(m_TaskData);
		if (!frameworkData)
			return string.Empty;
		
		return frameworkData.m_sTaskExecutionBriefing;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!super.RplSave(writer))
			return false;
		
		SCR_ScenarioFrameworkTaskData frameworkData = SCR_ScenarioFrameworkTaskData.Cast(m_TaskData);
		if (!frameworkData)
			return true;
		
		writer.WriteString(frameworkData.m_sTaskExecutionBriefing);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!m_TaskData)
			m_TaskData = new SCR_ScenarioFrameworkTaskData();
		
		if (!super.RplLoad(reader))
			return false;
		
		SCR_ScenarioFrameworkTaskData frameworkData = SCR_ScenarioFrameworkTaskData.Cast(m_TaskData);
		if (!frameworkData)
			return true;
		
		reader.ReadString(frameworkData.m_sTaskExecutionBriefing);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void InitializeData()
	{
		if (!m_TaskData)
			m_TaskData = new SCR_ScenarioFrameworkTaskData();
		
		super.InitializeData();
	}
}
