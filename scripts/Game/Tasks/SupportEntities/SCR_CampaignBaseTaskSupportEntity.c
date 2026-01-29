//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Campaign base task support entity.", color: "0 0 255 255")]
//class SCR_CampaignBaseTaskSupportEntityClass: SCR_BaseTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_CampaignBaseTaskSupportEntity : SCR_BaseTaskSupportEntity
//{
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTargetBase(int taskID, int baseID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_CampaignBaseTask task = SCR_CampaignBaseTask.Cast(GetTaskManager().GetTask(taskID));
//		if (!task)
//			return;
//		
//		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseID);
//		if (!base)
//			return;
//		
//		task.SetTargetBase(base);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnTaskUpdate(SCR_BaseTask task)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetBase(notnull SCR_CampaignBaseTask task, notnull SCR_CampaignMilitaryBaseComponent base)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		int taskID, baseID;
//		
//		taskID = task.GetTaskID();
//		baseID = base.GetCallsign();
//		
//		Rpc(RPC_SetTargetBase, taskID, baseID);
//		RPC_SetTargetBase(taskID, baseID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnInit(IEntity owner)
//	{
//		super.EOnInit(owner);
//		
//		if (!SCR_BaseTaskManager.s_OnTaskUpdate)
//			return;
//		
//		SCR_BaseTaskManager.s_OnTaskUpdate.Insert(OnTaskUpdate);
//	}
//}