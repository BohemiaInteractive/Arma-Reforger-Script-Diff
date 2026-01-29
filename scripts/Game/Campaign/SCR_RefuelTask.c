//[EntityEditorProps(category: "GameScripted/Campaign", description: "Refuel task.", color: "0 0 255 255")]
//class SCR_RefuelTaskClass: SCR_RequestedTaskClass
//{
//};
//
////------------------------------------------------------------------------------------------------
//class SCR_RefuelTask : SCR_RequestedTask
//{
//	IEntity m_targetVehicle;
//	
//	//------------------------------------------------------------------------------------------------
//	static Vehicle GetVehicleExecutorIsIn(SCR_TaskExecutor taskExecutor)
//	{
//		if (!taskExecutor)
//			return null;
//		
//		SCR_TaskExecutorPlayer taskExecutorPlayer = SCR_TaskExecutorPlayer.Cast(taskExecutor);
//		if (!taskExecutorPlayer)
//			return null;
//		
//		int playerID = taskExecutorPlayer.GetPlayerID();
//		
//		ChimeraCharacter character = ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
//		if (!character)
//			return null;
//		
//		if (!character.IsInVehicle())
//			return null;
//		
//		CompartmentAccessComponent compartmentAccessComponent = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
//		if (!compartmentAccessComponent)
//			return null;
//		
//		BaseCompartmentSlot compartmentSlot = compartmentAccessComponent.GetCompartment();
//		if (!compartmentSlot)
//			return null;
//		
//		Vehicle vehicle = Vehicle.Cast(compartmentSlot.GetOwner());
//		
//		return vehicle;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static bool CheckRefuelRequestConditions(notnull SCR_TaskExecutor taskExecutor)
//	{
//		Vehicle vehicle = GetVehicleExecutorIsIn(taskExecutor);
//		if (!vehicle)
//			return false;
//		
//		SCR_FuelNode fuelNode = SCR_FuelNode.Cast(vehicle.FindComponent(SCR_FuelNode));
//		if (!fuelNode)
//			return false;
//		
//		float maxFuel = fuelNode.GetMaxFuel();
//		float currentFuel = fuelNode.GetFuel();
//		float fuelPercent = currentFuel / maxFuel;
//		
//		return fuelPercent <= GetFuelLimit();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	static float GetFuelLimit()
//	{
//		if (!GetTaskManager())
//			return 0.5;
//		
//		SCR_RefuelTaskSupportEntity supportEntity = SCR_RefuelTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RefuelTaskSupportEntity));
//		if (supportEntity)
//			return supportEntity.GetFuelLimit();
//		
//		return 0.5;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTargetVehicle(IEntity targetVehicle)
//	{
//		// Only set on server!
//		m_targetVehicle = targetVehicle;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	IEntity GetTargetVehicle()
//	{
//		return m_targetVehicle;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequester(SCR_TaskExecutor requester, vector startOrigin)
//	{
//		super.SetRequester(requester);
//		
//		ShowAvailableTask();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//Called whenever any fuel component finished refueling (even half way through)
//	void OnRefuelingFinished(SCR_FuelNode fuelNode, IEntity refueler)
//	{
//		if (!fuelNode || !refueler)
//			return;
//		
//		IEntity vehicle = fuelNode.GetOwner();
//		if (!vehicle)
//			return;
//		
//		if (GetTargetVehicle() != vehicle)
//			return;
//		
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		SCR_BaseTaskExecutor assignee = GetAssignee();
//		if (!assignee)
//		{
//			supportEntity.FailTask(this);
//			return;
//		}
//		
//		IEntity assigneeControlledEntity = assignee.GetControlledEntity();
//		if (!assigneeControlledEntity || assigneeControlledEntity != refueler)
//		{
//			supportEntity.FailTask(this);
//			return;
//		}
//		
//		float currentFuel, maxFuel;
//		currentFuel = fuelNode.GetFuel();
//		maxFuel = fuelNode.GetMaxFuel();
//		float percent = currentFuel / maxFuel;
//		
//		if (percent >= SCR_RefuelTask.GetFuelLimit())
//			supportEntity.FinishTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_RefuelTask(IEntitySource src, IEntity parent)
//	{
//		if (SCR_Global.IsEditMode(this))
//			return;
//		
//		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
//		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
//		
//		SetIndividual(true);
//		
//		if (!GetTaskManager().IsProxy())
//			SCR_FuelNode.s_OnRefuelingFinished.Insert(OnRefuelingFinished);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_RefuelTask()
//	{
//		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
//			return;
//		
//		if (!GetTaskManager().IsProxy())
//			SCR_FuelNode.s_OnRefuelingFinished.Remove(OnRefuelingFinished);
//	}
//
//};
