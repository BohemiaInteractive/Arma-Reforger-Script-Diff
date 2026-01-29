/*!
Behavior which makes AI walk around a position while keeping beyond some minimal distance.
*/

class SCR_AIFindFirePositionBehavior : SCR_AIBehaviorBase
{
	static const float SNIPER_MIN_DISTANCE = 70.0;
	static const float SNIPER_MAX_DISTANCE = 180.0;
	static const float SNIPER_DURATION_S = 3*60.0;
	
	ref SCR_BTParam<vector> m_vTargetPosition = new SCR_BTParam<vector>("TargetPos");
	ref SCR_BTParam<float> m_fMinDistance = new SCR_BTParam<float>("MinDist");
	ref SCR_BTParam<float> m_fMaxDistance = new SCR_BTParam<float>("MaxDist");
	ref SCR_BTParam<float> m_fDuration_s = new SCR_BTParam<float>("Duration_s");
	
	EAIUnitType m_eTargetUnitType;
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}
	
	//----------------------------------------------------------------------------------------------------------------
	void InitParameters(vector targetPos, float minDistance, float maxDistance, float duration)
	{
		m_vTargetPosition.Init(this, targetPos);
		m_fMinDistance.Init(this, minDistance);
		m_fMaxDistance.Init(this, maxDistance);
		m_fDuration_s.Init(this, duration);
	}
	//----------------------------------------------------------------------------------------------------------------
	void SCR_AIFindFirePositionBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity, vector targetPos,
		float minDistance, float maxDistance, EAIUnitType targetUnitType, float duration, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(targetPos, minDistance, maxDistance, duration);
		
		m_eTargetUnitType = targetUnitType;
		
		m_sBehaviorTree = "{905124EEBF36D180}AI/BehaviorTrees/Chimera/Soldier/FindFirePosition.bt";
		SetPriority(PRIORITY_BEHAVIOR_FIND_FIRE_POSITION);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_bAllowLook = true;
	}
	
	//----------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		// Complete when we have found some target to attack
		if (m_Utility.m_CombatComponent.GetCurrentTarget())
		{
			Complete();
			return 0;
		}
		
		return GetPriority();
	}
	
	//-----------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		m_Utility.m_CombatComponent.SetExpectedEnemyType(m_eTargetUnitType);
	}
}

class SCR_AIGetFindFirePositionNextPos : SCR_AIActionTask
{
	protected static const string PORT_NEXT_POSITION = "NextPosition";
	
	// When searching for next position, it will be within this range from our current position
	protected const float NEXT_POS_MIN_DIST = 13.0;
	protected const float NEXT_POS_MAX_DIST = 20.0;
	
	// When walking towards or away from target, the position will be randomized by this radius
	protected const float WALK_DIRECTLY_RANDOM_RADIUS = 4.0;
	
	//-----------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComp)
			return ENodeResult.FAIL;
		
		SCR_AIActionBase executedAction = GetExecutedAction();
		SCR_AIFindFirePositionBehavior behavior = SCR_AIFindFirePositionBehavior.Cast(executedAction);
		if (!behavior)
			return ENodeResult.FAIL;
		
		vector targetPos = behavior.m_vTargetPosition.m_Value;
		IEntity myEntity = owner.GetControlledEntity();
		vector myPos = myEntity.GetOrigin();
		
		vector vFromTarget = vector.Direction(targetPos, myPos);
		vFromTarget[1] = 0;
		float distanceToTarget = vFromTarget.Length();
		vector vDirFromTarget = vFromTarget.Normalized();	// Direction vector from target to us
		vDirFromTarget.Normalize();
		
		vector nextMovePos;
		float minDist = behavior.m_fMinDistance.m_Value;
		float maxDist = behavior.m_fMaxDistance.m_Value;
		if (distanceToTarget > minDist &&
			distanceToTarget < maxDist)
		{
			// We are within optimal range, walk around
			nextMovePos = s_AIRandomGenerator.GenerateRandomPointInRadius(NEXT_POS_MIN_DIST, NEXT_POS_MAX_DIST, myPos);
		}
		else
		{
			float randomDistance = Math.RandomFloat(10, 20);
			float dirMultiplier = 1;
			if (distanceToTarget > maxDist)
			{
				// We are too far, move closer
				dirMultiplier = -1;
			}
			
			vector nextMovePosCenter = myPos + dirMultiplier * randomDistance * vDirFromTarget;
			nextMovePos = s_AIRandomGenerator.GenerateRandomPointInRadius(0, WALK_DIRECTLY_RANDOM_RADIUS, nextMovePosCenter);
		}
		
		SetVariableOut(PORT_NEXT_POSITION, nextMovePos);
		
		return ENodeResult.SUCCESS;
	}
	
	//-----------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//-----------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_NEXT_POSITION };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
}

class SCR_AIGetFirePositionBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIFindFirePositionBehavior(null, null, vector.Zero, 0, 0, 0, 0)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};