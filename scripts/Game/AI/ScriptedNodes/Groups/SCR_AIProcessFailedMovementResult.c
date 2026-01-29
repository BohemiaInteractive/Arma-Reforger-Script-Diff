
class SCR_AIProcessFailedMovementResult : AITaskScripted
{
	static const string PORT_MOVE_RESULT = "MoveResult";
	static const string PORT_HANDLER_ID = "FailedHandlerId";
	static const string PORT_IS_WAYPOINT_RELATED = "IsWaypointRelated";
	static const string PORT_MOVE_LOCATION = "MoveLocation";

	protected SCR_AIGroup m_Group;
	protected SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	protected bool m_bProducedError; // the node will not produce another error unless you run the tree with "always reinit" true 
	protected bool m_bReturnRunning; // when returning running we dont want to produce another errors
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
			return;
		
		m_Group = group;
		m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bReturnRunning = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int moveResult;
		if (m_bReturnRunning)
			return ENodeResult.RUNNING;
		
		if (!GetVariableIn(PORT_MOVE_RESULT, moveResult))
		{
			NodeErrorOnce(owner, "Missing move result for SCR_AIProcessFailedMovementResult node");
			m_bReturnRunning = true;
			return ENodeResult.RUNNING; 
		}
		
#ifdef WORKBENCH
		PrintDebug(owner, string.Format("Move failed with result %1", SCR_Enum.GetEnumName(EMoveError, moveResult)));
#endif
				
				
		int failedHandlerId;
		GetVariableIn(PORT_HANDLER_ID, failedHandlerId);
		
		bool isWaypointRelated;
		GetVariableIn(PORT_IS_WAYPOINT_RELATED, isWaypointRelated);
		
		vector moveLocation, startLocation;
		GetVariableIn(PORT_MOVE_LOCATION, moveLocation);
		if (m_Group)
			startLocation = m_Group.GetLeaderEntity().GetOrigin(); // safer than obtaining position of the group
		
		// we failed inside one of our vehicles -> leave the vehicle of the handlerId and try again
		IEntity vehicleUsed;
		SCR_AIGroupVehicle groupVehicle;
		if (failedHandlerId != AIGroupMovementComponent.DEFAULT_HANDLER_ID) 
		{
			groupVehicle = m_GroupUtilityComponent.m_VehicleMgr.FindVehicleBySubgroupId(failedHandlerId);
			if (groupVehicle)
				vehicleUsed = groupVehicle.GetEntity();
		}
		
		// Navlink is being negotiated, try again
		if (moveResult == EMoveError.WAITING_ON_NAVLINK)
		{
			return ENodeResult.FAIL;
		}
		
		// Ivoke event about failed movement
		m_GroupUtilityComponent.OnMoveFailed(moveResult, vehicleUsed, isWaypointRelated, moveLocation);
		
		// Failed movement
		if (moveResult == EMoveError.UNKNOWN)
		{
			if (isWaypointRelated)
			{
				SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(m_GroupUtilityComponent.GetCurrentAction());
				
				// Just fail action if failed move is a result or removed waypoint
				if (activity && !activity.m_RelatedWaypoint)
				{
#ifdef WORKBENCH
					PrintDebug(owner, string.Format("Failed move from %1 as result of deleted waypoint %2", startLocation, moveLocation));
#endif
					FailAction();
					return ENodeResult.FAIL;
				}
				
				// If wp related and wp is the same we just return running
				// As a result group will be stuck to allow us to debug it
				NodeErrorOnce(owner, string.Format("Failed move from %1 to waypoint %2", startLocation, moveLocation));
				m_bReturnRunning = true;
				return ENodeResult.RUNNING;
			}
			
			FailAction();
#ifdef WORKBENCH
			PrintDebug(owner, string.Format("Failed move from %1 while following the entity %2", startLocation, moveLocation));
#endif
			// If result is not waypoint related we just fail
			return ENodeResult.FAIL;
		}
		
		// If stopped or vehicle is not used, complete waypoint if related and fail action
		if (moveResult == EMoveError.STOPPED || !vehicleUsed)
		{
#ifdef WORKBENCH
			PrintDebug(owner, string.Format("Move failed with result %1 from %2 to location %3, failing action.", SCR_Enum.GetEnumName(EMoveError, moveResult), startLocation, moveLocation));
#endif
			if (isWaypointRelated)
				CompleteWaypoint();
			
			// Fail action
			FailAction();
			m_bReturnRunning = true;
			return ENodeResult.RUNNING;
		}
#ifdef WORKBENCH
		PrintDebug(owner, string.Format("Move failed with result %1 from %2 to location %3, retrying without vehicle.", SCR_Enum.GetEnumName(EMoveError, moveResult), startLocation, moveLocation));
#endif	
		// Exit vehicle and try again
		if (vehicleUsed)
		{
			// If failed because the driver is dead, we dont need to leave vehicle
			if (moveResult != EMoveError.ENTITY_CANT_MOVE) 
			{
				// higher priority to overwrite conflicting forced move priorities
				SCR_AIGetOutActivity getOut = new SCR_AIGetOutActivity(m_GroupUtilityComponent, null, vehicleUsed, priority: SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE_HIGH_PRIORITY, priorityLevel: SCR_AIActionBase.PRIORITY_LEVEL_PLAYER);
				m_GroupUtilityComponent.AddAction(getOut);
			}
			
			// If vehicle got stuck turn the hazard lights on
			if (moveResult == EMoveError.STUCK)
			{
				// Turn them only if there's no enemy around
				TryTurnOnHazardLightsOnStuck(groupVehicle);
			}
		}
		
		// Fail a node to enable tree to try again
		return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryTurnOnHazardLightsOnStuck(notnull SCR_AIGroupVehicle groupVehicle)
	{
		IEntity driver = groupVehicle.GetDriver();
		
		if (!driver)
			return;
		
		bool turnOnLights = m_GroupUtilityComponent && m_GroupUtilityComponent.m_Perception && !m_GroupUtilityComponent.m_Perception.m_MostDangerousCluster;
		if (!turnOnLights)
			return;
		
		// We want to turn on hazard lights
		
		// Bail if it's forbidden by settings of driver
		SCR_AICharacterSettingsComponent settingsComp = SCR_AICharacterSettingsComponent.FindOnControlledEntity(driver);
		if (settingsComp)
		{
			SCR_AICharacterLightInteractionSettingBase setting = SCR_AICharacterLightInteractionSettingBase.Cast(settingsComp.GetCurrentSetting(SCR_AICharacterLightInteractionSettingBase));
			if (setting && !setting.IsLightInterractionAllowed())
				return;
		}
		
		SCR_AIVehicleUsability.TurnOnVehicleHazardLights(groupVehicle.GetEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	void PrintDebug(AIAgent owner, string message)
	{
#ifdef WORKBENCH		
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_DEBUG))
			return;
	
		Print(string.Format("%1: %2", owner.ToString(), message));
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void FailAction()
	{
		AIActionBase currentAction = m_GroupUtilityComponent.GetExecutedAction();
		if (currentAction)
			currentAction.Fail();
	}
	
	//------------------------------------------------------------------------------------------------
	void CompleteWaypoint()
	{
		if (!m_Group)
			return;
		AIWaypoint waypoint = m_Group.GetCurrentWaypoint();
		if (waypoint)
			m_Group.CompleteWaypoint(waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected bool CanReturnRunning() { return true; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_MOVE_RESULT, PORT_HANDLER_ID, PORT_IS_WAYPOINT_RELATED, PORT_MOVE_LOCATION};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Processes failed movement result for proper BT reaction, calls group's failed move invoker";
	}
	
	//------------------------------------------------------------------------------------------------
	protected ENodeResult NodeErrorOnce(AIAgent owner, string errorMessage)
	{
		if (!m_bProducedError)
			NodeError(this, owner, errorMessage);
		m_bProducedError = true;
		return ENodeResult.FAIL;
	}
}