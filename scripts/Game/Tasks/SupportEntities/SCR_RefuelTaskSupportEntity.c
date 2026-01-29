//------------------------------------------------------------------------------------------------
//[Obsolete(), EntityEditorProps(category: "GameScripted/Tasks", description: "Refuel task support entity.", color: "0 0 255 255")]
//class SCR_RefuelTaskSupportEntityClass: SCR_RequestedTaskSupportEntityClass
//{
//};

//------------------------------------------------------------------------------------------------
//[Obsolete()]
//class SCR_RefuelTaskSupportEntity : SCR_RequestedTaskSupportEntity
//{
//	[Attribute("0.5", params: "0 1 0.01", desc: "Player can request refuel once fuel in the tank is below this value. (x Max fuel in tank)")]
//	protected float m_fFuelLimit;
//	
//	//------------------------------------------------------------------------------------------------
//	// Call only if you null checked GetTaskManager()!
//	protected SCR_RefuelTask CreateNewRefuelTask(int requesterID, IEntity targetVehicle = null)
//	{
//		SCR_RefuelTask refuelTask = SCR_RefuelTask.Cast(CreateTask());
//		if (!refuelTask)
//			return null;
//		
//		SCR_BaseTaskExecutor requester = SCR_BaseTaskExecutor.GetTaskExecutorByID(requesterID);
//		
//		SetRequester(refuelTask, requester); //Replicated inside
//		refuelTask.SetTargetVehicle(targetVehicle); //Only set on server
//		
//		return refuelTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//This should only be called on the server!
//	void RequestRefuel(int requesterID, IEntity vehicle, vector position)
//	{
//		if (!GetTaskManager())
//			return;
//		
//		SCR_RefuelTask refuelTask = CreateNewRefuelTask(requesterID, vehicle);
//		
//		if (!refuelTask)
//			return;
//		
//		MoveTask(position, refuelTask.GetTaskID());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override bool CanRequest()
//	{
//		if (!super.CanRequest())
//			return false;
//		
//		SCR_BaseTaskExecutor localTaskExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
//		if (!localTaskExecutor)
//			return false;
//		
//		if (!SCR_RefuelTask.CheckRefuelRequestConditions(localTaskExecutor))
//			return false;
//		
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Request()
//	{
//		PlayerController playerController = GetGame().GetPlayerController();
//		if (!playerController)
//			return;
//		
//		SCR_MapRadialUI ctxMenu = SCR_MapRadialUI.GetInstance();
//		if (!ctxMenu)
//			return;
//		
//		vector position = ctxMenu.GetMenuWorldPosition();
//		
//		// Find task network component to send RPC to server
//		SCR_CampaignTaskNetworkComponent taskNetworkComponent = SCR_CampaignTaskNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignTaskNetworkComponent));
//		if (!taskNetworkComponent)
//			return;
//		
//		if (!SCR_UIRequestEvacTaskComponent.HasSignal(playerController))
//			return;
//		
//		taskNetworkComponent.RequestRefuel(position);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	float GetFuelLimit()
//	{
//		return m_fFuelLimit;
//	}
//};