//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Campaign task support entity.", color: "0 0 255 255")]
//class SCR_CampaignTaskSupportEntityClass: SCR_CampaignBaseTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_CampaignTaskSupportEntity : SCR_CampaignBaseTaskSupportEntity
//{
//	//------------------------------------------------------------------------------------------------
//	//! Returns the found task if it exists.
//	//TODO: Remove this method after no longer needed.
//	SCR_CampaignTask GetTask(SCR_CampaignMilitaryBaseComponent targetBase, Faction targetFaction, SCR_CampaignTaskType type)
//	{
//		if (!GetTaskManager())
//			return null;
//		
//		array<SCR_BaseTask> tasks = {};
//		int count = GetTaskManager().GetTasks(tasks);
//		
//		for (int i = count - 1; i >= 0; i--)
//		{
//			SCR_CampaignTask castTask = SCR_CampaignTask.Cast(tasks[i]);
//			
//			if (!castTask)
//				continue;
//			
//			if (castTask.GetType() == type && castTask.GetTargetBase() == targetBase && castTask.GetTargetFaction() == targetFaction)
//				return castTask;
//		}
//		return null;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Server side
//	//! Called after a new base has been captured or a mobile HQ deployed
//	//! Creates capture tasks for small, major and main bases in newly gained range
//	void GenerateCaptureTasks(notnull SCR_CampaignMilitaryBaseComponent base)
//	{
//		if (!base.IsInitialized())
//			return;
//
//		if ((base.IsHQ() || (base.CanBeHQ() && base.GetDisableWhenUnusedAsHQ())))
//			return;
//
//		SCR_CampaignFaction baseFaction = base.GetCampaignFaction();
//		
//		if (!baseFaction)
//			return;
//
//		array<Faction> factions = {};
//		GetGame().GetFactionManager().GetFactionsList(factions);
//		SCR_CampaignFaction campaignFaction;
//		
//		foreach (Faction faction : factions)
//		{
//			campaignFaction = SCR_CampaignFaction.Cast(faction);
//			
//			if (!campaignFaction || !campaignFaction.IsPlayable())
//				continue;
//
//			if (baseFaction == campaignFaction)
//				continue;
//
//			if (!base.IsHQRadioTrafficPossible(campaignFaction))
//				continue;
//
//			if (GetTask(base, campaignFaction, SCR_CampaignTaskType.CAPTURE))
//				continue;
//
//			CreateNewCampaignTask(SCR_CampaignTaskType.CAPTURE, base, campaignFaction);
//		}
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! Creates a new conflict task.
//	protected SCR_CampaignTask CreateNewCampaignTask(SCR_CampaignTaskType type, notnull SCR_CampaignMilitaryBaseComponent base, notnull Faction faction)
//	{
//		SCR_CampaignTask task = SCR_CampaignTask.Cast(CreateTask());
//		if (!task)
//			return null;
//		
//		SetTargetFaction(task, faction); // Replicated internally
//		SetTargetBase(task, base); // Replicated internally
//		SetTaskType(task, type); // Replicated internally
//		
//		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (campaign && SCR_FactionManager.SGetLocalPlayerFaction() == faction)
//		{
//			string text = task.TASK_AVAILABLE_TEXT + " " + task.GetTitle();
//			string baseName;
//			
//			if (task.GetTargetBase())
//				baseName = task.GetTargetBase().GetBaseNameUpperCase();
//
//			SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: task.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
//		}
//		
//		return task;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTaskType(notnull SCR_CampaignTask task, SCR_CampaignTaskType type)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		int taskID = task.GetTaskID();
//		
//		Rpc(RPC_SetTaskType, taskID, type);
//		RPC_SetTaskType(taskID, type);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTaskType(int taskID, SCR_CampaignTaskType type)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_CampaignTask task = SCR_CampaignTask.Cast(GetTaskManager().GetTask(taskID));
//		if (!task)
//			return;
//		
//		task.SetType(type);
//	}
//};