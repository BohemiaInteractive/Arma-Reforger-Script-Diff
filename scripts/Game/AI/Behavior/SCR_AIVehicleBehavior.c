enum EAICompartmentType
{
	None = -1,
	Pilot,
	Turret,
	Cargo,
}

class SCR_AIVehicleBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIVehicleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity)
	{
		m_Vehicle.Init(m_aParams, vehicleEntity);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		// Most of the time we get into vehicle because group told so
		return SCR_EAIBehaviorCause.GROUP_GOAL;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected() 
	{
		super.OnActionSelected();
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.BUSY);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionCompleted();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed() 
	{
		m_Utility.m_AIInfo.SetAIState(EUnitAIState.AVAILABLE);
		super.OnActionFailed();
	}
};

class SCR_AIGetInVehicle : SCR_AIVehicleBehavior
{
	ref SCR_BTParam<EAICompartmentType> m_eRoleInVehicle = new SCR_BTParam<EAICompartmentType>(SCR_AIActionTask.ROLEINVEHICLE_PORT);
	ref SCR_BTParam<BaseCompartmentSlot> m_CompartmentToGetIn = new SCR_BTParam<BaseCompartmentSlot>(SCR_AIActionTask.COMPARTMENT_PORT);
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIGetInVehicle(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity, BaseCompartmentSlot compartmentSlot, EAICompartmentType roleInVehicle = EAICompartmentType.Cargo, float priority = PRIORITY_BEHAVIOR_GET_IN_VEHICLE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_CompartmentToGetIn.Init(this, compartmentSlot);
		m_eRoleInVehicle.Init(this, roleInVehicle);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetInVehicle.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted() 
	{
		if (m_CompartmentToGetIn.m_Value)
		{
			m_CompartmentToGetIn.m_Value.SetCompartmentAccessible(true);
			m_CompartmentToGetIn.m_Value.SetReserved(null);			
		};
		super.OnActionCompleted();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed() 
	{
		if (m_CompartmentToGetIn.m_Value)
		{
			ChimeraCharacter chimchar = ChimeraCharacter.Cast(m_Utility.m_OwnerEntity);
			CompartmentAccessComponent compMan = chimchar.GetCompartmentAccessComponent();
			IEntity vehicle = m_CompartmentToGetIn.m_Value.GetVehicle();
			if (compMan.IsGettingIn())
			{
				SCR_AIGetOutVehicle getOutBehavior = new SCR_AIGetOutVehicle(m_Utility, null , vehicle, 
					priority : PRIORITY_BEHAVIOR_GET_OUT_VEHICLE_HIGH_PRIORITY, priorityLevel : EvaluatePriorityLevel());
				m_Utility.AddAction(getOutBehavior);
			}
			else if (vehicle && vehicle == compMan.GetVehicleIn(chimchar))
				compMan.GetOutVehicle(EGetOutType.TELEPORT, 0, ECloseDoorAfterActions.LEAVE_OPEN, true);
			else 
				compMan.InterruptVehicleActionQueue(true, true, true);
			m_CompartmentToGetIn.m_Value.SetCompartmentAccessible(true);
			m_CompartmentToGetIn.m_Value.SetReserved(null);			
		};
		super.OnActionFailed();
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " moving to " + m_Vehicle.m_Value.ToString() + " inside " + m_CompartmentToGetIn.m_Value.ToString();
	}
};

class SCR_AIGetOutVehicle : SCR_AIVehicleBehavior
{
	ref SCR_BTParam<float> m_fDelay = new SCR_BTParam<float>("Delay");
		
	//------------------------------------------------------------------------------------------------
	void SCR_AIGetOutVehicle(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, IEntity vehicleEntity, float delay_s = 0, float priority = PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Soldier/GetOutVehicle.bt";
		SetPriority(priority);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_fDelay.Init(this, delay_s);
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " leaving " + m_Vehicle.m_Value.ToString();
	}
};
