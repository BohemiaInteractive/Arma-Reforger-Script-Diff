class SCR_AICombatMoveLogic_MoveFromIncomingVehicle : AITaskScripted
{
	// Inputs
	protected static const string PORT_VEHICLE_ENTITY = "VehicleEntity";
	
	protected SCR_AIUtilityComponent m_Utility;
	protected SCR_AICombatMoveState m_State;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_Utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (m_Utility)
			m_State = m_Utility.m_CombatMoveState;
	}
	
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State || !m_Utility)
			return ENodeResult.FAIL;
		
		IEntity vehicleEntity;
		GetVariableIn(PORT_VEHICLE_ENTITY, vehicleEntity);
		
		if (!vehicleEntity)
			return ENodeResult.FAIL;
		
		// Ensure the node doesn't send new commands in situation when the whole behavior should not run any more
		if (!SCR_AIMoveFromIncomingVehicleBehavior.ExecuteBehaviorCondition(m_Utility, vehicleEntity))
			return ENodeResult.FAIL;
		
		CombatMoveLogic(vehicleEntity);
		
		return ENodeResult.SUCCESS;
	}
	
	//--------------------------------------------------------------------------------------------
	protected void CombatMoveLogic(IEntity vehicleEntity)
	{		
		// By now it has already been checked that vehicle is approaching us and we are not in a valid cover
		// It was checked by SCR_AIMoveFromIncomingVehicleBehavior.ExecuteBehaviorCondition()
		
		
		if (ConditionPushMoveRequest(vehicleEntity))
		{
			SCR_AICombatMoveRequest_Move rq = new SCR_AICombatMoveRequest_Move();
			
			rq.m_eReason = SCR_EAICombatMoveReason.MOVE_FROM_DANGER;
			
			rq.m_vTargetPos = vehicleEntity.GetOrigin();
			rq.m_vMovePos = rq.m_vTargetPos;
			rq.m_bTryFindCover = true;
			rq.m_bUseCoverSearchDirectivity = false; // We don't care about cover directivity, we want to find nearest
			rq.m_bCheckCoverVisibility = false;
			rq.m_bFailIfNoCover = false;
			rq.m_eStanceMoving = ECharacterStance.STAND;
			rq.m_eStanceEnd = ECharacterStance.STAND;
			rq.m_eMovementType = EMovementType.SPRINT;
			rq.m_fCoverSearchDistMax = 9;
			rq.m_fCoverSearchDistMin = 0;
			rq.m_fMoveDuration_s = rq.m_fCoverSearchDistMax / SCR_AICombatMoveUtils.CHARACTER_SPEED_STAND_SPRINT;
			rq.m_eDirection = CalculateVehicleAvoidDirection(vehicleEntity);
			rq.m_fCoverSearchSectorHalfAngleRad = -Math.PI; // Full circle
			
			rq.m_bAimAtTarget = false;
			rq.m_bAimAtTargetEnd = true;
			
			m_State.ApplyNewRequest(rq);
		}
	}
	
	//--------------------------------------------------------------------------------------------
	protected bool ConditionPushMoveRequest(notnull IEntity vehicleEntity)
	{
		if (m_State.IsMoving())
		{
			// Moving somewhere
			
			// It is not our request, send a new request
			SCR_AICombatMoveRequest_Move rqMove = SCR_AICombatMoveRequest_Move.Cast(m_State.GetRequest());
			if (rqMove && rqMove.m_eReason != SCR_EAICombatMoveReason.MOVE_FROM_DANGER)
				return true;
			
			if (m_State.IsMovingToCover())
			{
				// Moving to cover
				
				if (!m_State.GetAssignedCover().IsValid())
					return true; // Cover was invalidated, send new request
				
				if (SCR_AIMoveFromIncomingVehicleBehavior.IsCoverSafeAgainstVehicle(m_State.GetAssignedCover(), vehicleEntity))
					return false;
				else
					return true; // Cover will not protect against vehicle, find another one
			}
		}	
		else if (m_State.m_bInCover && m_State.IsAssignedCoverValid())
		{
			// Not moving, but we are in cover
			if (!SCR_AIMoveFromIncomingVehicleBehavior.IsCoverSafeAgainstVehicle(m_State.GetAssignedCover(), vehicleEntity))
				return true; // Cover is not good, find a new one
			else
				return false;
		}
		else
		{
			// Not moving, not in cover, run somewhere
			return true;
		}
		
		return false;
	}
	
	//--------------------------------------------------------------------------------------------
	protected SCR_EAICombatMoveDirection CalculateVehicleAvoidDirection(IEntity vehicleEntity)
	{
		// Get my pos in vehicle space
		vector myPos = m_Utility.m_OwnerEntity.GetOrigin();
		vector myPosVehicleSpace = vehicleEntity.CoordToLocal(myPos);
		
		// Vehicle's aside vector, Y component removed
		vector vehicleAside = vehicleEntity.GetTransformAxis(0);
		vehicleAside[1] = 0;
		if (vehicleAside.LengthSq() < 0.01)
		{
			// If vehicle aside vector is not in XZ plane, probably vehicle is flipped sideways
			// In this case let's move away from vehicle
			
			return SCR_EAICombatMoveDirection.BACKWARD;
		}
		else
		{
			vehicleAside.Normalize();
			
			// Decide which direction we should move
			vector myOffsetDirection = vehicleAside; // Direction where we will move
			if (myPosVehicleSpace[0] < 0)	// I am left of vehicle (in vehicle space)
				return SCR_EAICombatMoveDirection.RIGHT; // From my point of view, I should move right, relative to vector towards vehicle
			else
				return SCR_EAICombatMoveDirection.LEFT;
		}
	}
	
	//------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = { PORT_VEHICLE_ENTITY };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}