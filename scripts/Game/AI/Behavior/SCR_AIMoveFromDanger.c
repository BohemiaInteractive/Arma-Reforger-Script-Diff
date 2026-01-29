/*!
Behavior for short-lived fast reactions to get away from some danger source.
*/
class SCR_AIMoveFromDangerBehavior : SCR_AIBehaviorBase
{
	ref SCR_BTParam<vector> m_DangerPosition = new SCR_BTParam<vector>("DangerPosition");
	ref SCR_BTParam<IEntity> m_DangerEntity = new SCR_BTParam<IEntity>("DangerEntity");
	ref SCR_BTParam<ECharacterStance> m_Stance = new SCR_BTParam<ECharacterStance>("CharacterStance");
	ref SCR_BTParam<EMovementType> m_MovementType = new SCR_BTParam<EMovementType>("MovementType");
	
	//-----------------------------------------------------------------------------------------------------
	void InitParameters(IEntity dangerEntity, vector dangerPos)
	{
		m_DangerEntity.Init(this, dangerEntity);
		m_DangerPosition.Init(this, dangerPos);
		m_Stance.Init(this, ECharacterStance.STAND);
		m_MovementType.Init(this, EMovementType.RUN); // Don't use sprint! Character can't reload during sprint, it makes it broken.
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.DANGER_HIGH;
	}
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromDangerBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity)
	{
		SetPriority(PRIORITY_BEHAVIOR_MOVE_FROM_DANGER);
		InitParameters(dangerEntity, dangerPos);
				
		if (dangerEntity)
		{
			m_DangerPosition.m_Value = dangerEntity.GetOrigin();
		}
		
		m_sBehaviorTree = "{D12937CF422B639B}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_Position.bt"
	}
	
	//-----------------------------------------------------------------------------------------------------
	//! Returns true if there exists a SCR_AIMoveFromDangerBehavior with m_DangerEntity assigned to 'ent'
	static bool ExistsBehaviorForEntity(SCR_AIUtilityComponent utility, IEntity ent)
	{
		SCR_AIMoveFromDangerBehavior behavior = SCR_AIMoveFromDangerBehavior.Cast(utility.FindActionOfInheritedType(SCR_AIMoveFromDangerBehavior));
		if (behavior && behavior.m_DangerEntity.m_Value == ent)
			return true;
		else
			return false;
	}
}

class SCR_AIMoveFromUnsafeAreaBehavior : SCR_AIMoveFromDangerBehavior
{	
	ref SCR_BTParam<float> m_Distance = new SCR_BTParam<float>("Distance");
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromUnsafeAreaBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity, float distance)
	{
		m_Distance.Init(this, distance);
		m_MovementType.Init(this, EMovementType.RUN);
		m_sBehaviorTree = "{9C0564CA979D57B1}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_UnsafeArea.bt";
	}
}

class SCR_AIMoveFromGrenadeBehavior : SCR_AIMoveFromDangerBehavior
{
	static const float MAX_GRENADE_LOOKAT_TIMEOUT_MS = 600;
	static const float DISTANCE_MAX_SQ = 13*13;
	
	float m_fBehaviorTimeout = 0;
	float m_fDistanceToGrenade = 0;
	vector m_vObserveReactionPosition;
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromGrenadeBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity, float behaviorDelay, vector observePosition)
	{
		m_bAllowLook = false;
		m_fBehaviorTimeout = GetGame().GetWorld().GetWorldTime() + behaviorDelay;
		m_sBehaviorTree = "{478811D2295EAF3E}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_Grenade.bt";
		m_MovementType.m_Value = EMovementType.SPRINT;
		m_fDistanceToGrenade = vector.Distance(utility.m_OwnerEntity.GetOrigin(), dangerPos);
		m_vObserveReactionPosition = observePosition;
		
		SetPriority(0);
		
		m_bUseCombatMove = true;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (CompleteActionCondition())
		{
			Complete();
			return 0;
		}
		
		if (GetGame().GetWorld().GetWorldTime() > m_fBehaviorTimeout)
			return PRIORITY_BEHAVIOR_MOVE_FROM_DANGER;
		
		return 0;
	}
	
	bool CompleteActionCondition()
	{
		if (!m_DangerEntity.m_Value)
			return true;
		
		IEntity myEntity = m_Utility.m_OwnerEntity;
		if (!myEntity)
			return true;
		
		if (vector.DistanceSq(myEntity.GetOrigin(), m_DangerPosition.m_Value) > DISTANCE_MAX_SQ)
			return true;
		
		return false;
	}
}

class SCR_AIMoveFromIncomingVehicleBehavior : SCR_AIMoveFromDangerBehavior
{
	protected static const float MIN_COS_ANGLE_COVER_TO_VEHICLE = -0.707; // Cos 135 deg
	
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromIncomingVehicleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity)
	{
		m_sBehaviorTree = "{2488649728730886}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_Vehicle.bt";
		SetPriority(PRIORITY_BEHAVIOR_MOVE_FROM_DANGER);
		m_MovementType.m_Value = EMovementType.SPRINT;
		m_bUseCombatMove = true;
		m_bAllowLook = false;
	}
	
	//-----------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (!ExecuteBehaviorCondition(m_Utility, m_DangerEntity.m_Value))
		{
			Complete();
			return 0;
		}
		
		return GetPriority();
	}
	
	//-----------------------------------------------------------------------------------------------------
	// Condition for the behavior to run
	static bool ExecuteBehaviorCondition(notnull SCR_AIUtilityComponent utilityComp, IEntity vehicle)
	{
		if (!vehicle)
			return false;
		
		// If vehicle is not moving towards us
		IEntity myEntity = utilityComp.m_OwnerEntity;
		if (!IsVehicleMovingTowardsMe(myEntity, vehicle))
			return false;
		
		// If we are already in good cover
		SCR_AICombatMoveState cmStace = utilityComp.m_CombatMoveState;
		SCR_AICoverLock cover = utilityComp.m_CombatMoveState.GetAssignedCover();
		if (cmStace.m_bInCover && cover && cover.IsValid() && IsCoverSafeAgainstVehicle(cover, vehicle))
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------
	static bool IsCoverSafeAgainstVehicle(notnull SCR_AICoverLock cover, notnull IEntity vehicle)
	{
		float cosAngleCoverToTgt = cover.CosAngleToThreat(vehicle.GetOrigin());
		return cosAngleCoverToTgt > MIN_COS_ANGLE_COVER_TO_VEHICLE;
	}
	
	//-----------------------------------------------------------------------------------------------------
	protected static bool IsVehicleMovingTowardsMe(notnull IEntity myEntity, notnull IEntity vehicle)
	{
		Physics phy = vehicle.GetPhysics();
		
		if (!phy)
			return false;
		
		// Bail if velocity is around zero
		vector velocityWorld = phy.GetVelocity();
		float velocityAbsSq = velocityWorld.LengthSq();
		if (velocityAbsSq <= 0.001)
			return false;
		
		vector velocityDir = velocityWorld.Normalized();
		
		vector vecToMe = vector.Direction(vehicle.GetOrigin(), myEntity.GetOrigin());
		vecToMe.Normalize();
		
		float cosAngle = vector.Dot(vecToMe, velocityDir);
		
		return cosAngle > 0.707; // 45deg
	}
}

class SCR_AIMoveFromVehicleHornBehavior : SCR_AIMoveFromDangerBehavior
{
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIMoveFromVehicleHornBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity)
	{
		m_sBehaviorTree = "{10A3DFFBC3629A79}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_VehicleHorn.bt";
		SetPriority(PRIORITY_BEHAVIOR_MOVE_FROM_VEHICLE_HORN);
	}
}

//! Special behavior for pilot when he's about to collide with another vehicle
class SCR_AIPilotMoveFromIncomingVehicleBehavior : SCR_AIMoveFromDangerBehavior
{
	ref SCR_BTParam<vector> m_vMovePos = new SCR_BTParam<vector>("MovePos");
	
	protected const float EXTRAPOLATE_POS_FORWARD_TIME_S = 0.2;
	protected const float SIDE_STEP_LEN = 8;
	protected const float SIDE_STEP_FORWARD = 12;
	//-----------------------------------------------------------------------------------------------------
	void SCR_AIPilotMoveFromIncomingVehicleBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector dangerPos, IEntity dangerEntity)
	{
		m_sBehaviorTree = "{4CD41D1E2C9CD745}AI/BehaviorTrees/Chimera/Soldier/MoveFromDanger_PilotVehicle.bt";
		m_vMovePos.Init(this, vector.Zero);
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		// Initialize m_vMovePos
		if (!m_DangerEntity.m_Value)
		{
			Fail();
			return;
		}
		
		IEntity myVehicle = m_Utility.m_OwnerEntity.GetRootParent();
		Physics phy = myVehicle.GetPhysics();
		if (!phy)
		{
			Fail();
			return;
		}

		Physics phyDanger = m_DangerEntity.m_Value.GetPhysics();
		if (!phyDanger)
			return; // shouldn't happen as it triggered the even in first place
		
		vector dangerVelocity = phyDanger.GetVelocity().Normalized();
		
		vector myVelWorld = phy.GetVelocity();
		vector myTransform[4];
		myVehicle.GetTransform(myTransform);

		// basicaly the rule is when danger comes from front or back go to right side
		// if comes from side - go forward 
		float dot = vector.DotXZ(dangerVelocity, myTransform[2]);
		if (dot > 0.5 || dot < -0.5)
		{
			m_vMovePos.m_Value = myTransform[3] + EXTRAPOLATE_POS_FORWARD_TIME_S * myVelWorld + SIDE_STEP_LEN * myTransform[0] + SIDE_STEP_FORWARD * myTransform[2];
		}
		else
		{
			m_vMovePos.m_Value = myTransform[3] + EXTRAPOLATE_POS_FORWARD_TIME_S * myVelWorld + SIDE_STEP_LEN * myTransform[2];
		}
	}
}

class SCR_AIGetPilotMoveFromIncomingVehicleBehaviorParameters : SCR_AIGetActionParameters
{
	protected static ref TStringArray s_aVarsOut = (new SCR_AIPilotMoveFromIncomingVehicleBehavior(null, null, vector.Zero, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
}