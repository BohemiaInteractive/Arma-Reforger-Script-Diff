//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Transport task support entity.", color: "0 0 255 255")]
//class SCR_TransportTaskSupportEntityClass: SCR_RequestedTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_TransportTaskSupportEntity : SCR_RequestedTaskSupportEntity
//{
//	[Attribute("#AR-Tasks_RequestFromTransport")]
//	protected LocalizedString m_sRequestFromText;
//	
//	[Attribute("#AR-Tasks_RequestToTransport")]
//	protected LocalizedString m_sRequestToText;
//	
//	[Attribute("50", "How far can the assignee be from the requester when the task is evaluated.")]
//	protected float m_fMaxDistanceAssignee;
//	
//	[Attribute("50", "How far from the destination is the task evaluated.")]
//	protected float m_fMaxDistanceDestination;
//	
//	protected bool m_bSetFromPosition;
//	protected vector m_vFromPosition;
//	protected vector m_vToPosition;
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMaxDistanceAssigneeSq()
//	{
//		return m_fMaxDistanceAssignee * m_fMaxDistanceAssignee;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetMaxDistanceDestinationSq()
//	{
//		return m_fMaxDistanceDestination * m_fMaxDistanceAssignee;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_TransportTask CreateNewTransportTask(int requesterID, vector fromPosition, vector toPosition)
//	{
//		SCR_TransportTask task = SCR_TransportTask.Cast(CreateTask());
//		if (!task)
//			return null;
//		
//		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		
//		SetRequester(task, requester); // Internally replicated
//		SetTargetPosition(task, toPosition); // Internally replicated
//		
//		return task;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetPosition(notnull SCR_TransportTask task, vector position)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		int taskID = task.GetTaskID();
//		
//		RPC_SetTargetPosition(taskID, position);
//		Rpc(RPC_SetTargetPosition, taskID, position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void TransportTaskNextPhase(notnull SCR_TransportTask transportTask)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		if (!transportTask.GetVolunteerMet())
//		{
//			transportTask.SetVolunteerMet(true);
//			
//			MoveTask(transportTask.GetTargetPosition(), transportTask.GetTaskID());
//			Rpc(RPC_TransportTaskNextPhase, transportTask.GetTaskID());
//		}
//		else
//			FinishTask(transportTask);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	protected void RPC_TransportTaskNextPhase(int taskID)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_TransportTask transportTask = SCR_TransportTask.Cast(GetTaskManager().GetTask(taskID));
//		
//		if (transportTask)
//			transportTask.SetVolunteerMet(true);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
//	void RPC_SetTargetPosition(int taskID, vector position)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_TransportTask task = SCR_TransportTask.Cast(GetTaskManager().GetTask(taskID));
//		if (!task)
//			return;
//		
//		task.SetTargetPosition(position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void RequestTransport(int requesterID, vector fromPosition, vector toPosition)
//	{
//		CreateNewTransportTask(requesterID, fromPosition, toPosition);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected override void RegisterContextualMenuCallbacks()
//	{
//		if (m_sRequestButtonText.IsEmpty())
//			return;
//		
//		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapRadialUI));
//		if (!radialMenu)
//			return;
//			
//		SCR_MapMenuRequestedTaskEntry entry = new SCR_MapMenuRequestedTaskEntry();
//		if (!entry)
//			return;
//		
//		entry.SetName(m_sRequestButtonText);
//		entry.SetSupportClass(this);
//		entry.GetOnPerform().Insert(Request);
//		//entry.m_OnShow.Insert(OnContextualEntryShow);
//		
//		radialMenu.InsertCustomRadialEntry(entry);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void SetToPosition()
//	{
//		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapRadialUI));
//		if (!radialMenu)
//			return;
//		
//		m_vToPosition = radialMenu.GetMenuWorldPosition();
//		m_bSetFromPosition = false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void SetFromPosition()
//	{
//		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapRadialUI));
//		if (!radialMenu)
//			return;
//		
//		m_vFromPosition = radialMenu.GetMenuWorldPosition();
//		m_bSetFromPosition = true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool CanRequest()
//	{
//		return super.CanRequest();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetRequestButtonText(notnull TextWidget textWidget)
//	{
//		if (!m_bSetFromPosition)
//			textWidget.SetTextFormat(m_sRequestFromText);
//		else
//			textWidget.SetTextFormat(m_sRequestToText);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Request()
//	{
//		if (!m_bSetFromPosition)
//		{
//			SetFromPosition();
//			return;
//		}
//		
//		SetToPosition();
//		
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return;
//		
//		// Find task network component to send RPC to server
//		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
//		if (!taskNetworkComponent)
//			return;
//		
//		if (!SCR_UIRequestEvacTaskComponent.HasSignal(playerController))
//			return;
//		
//		taskNetworkComponent.RequestTransport(m_vFromPosition, m_vToPosition);
//	}
//};