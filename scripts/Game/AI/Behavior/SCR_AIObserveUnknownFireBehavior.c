/*!
Behavior to observe supposed location of where gunshot came from.

Looking lasts for some time (duration).
There is also a timeout value, after which the action will fail even if never started.
*/

[Obsolete("This behavior is no longer used and might be deleted in future. It is replaced by SCR_AIObserveThreatSystemBehavior.")]
class SCR_AIObserveUnknownFireBehavior : SCR_AIBehaviorBase
{
	protected const float TIMEOUT_S = 16.0;
	protected const float DURATION_MIN_S = 3.0;			// Min duration of behavior
	protected const float DIRECTION_SPAN_DEG = 32.0;	
	protected const float DURATION_S_PER_METER = 0.1;	// How duration depends on distance
	protected const float USE_BINOCULARS_DISTANCE_THRESHOLD = 70;
	
	protected const float HIGH_PRIORITY_MAX_DISTANCE = 50; // Max distance at which we consider observing unknown fire a high priority
	
	protected const float DELAY_MIN_S = 0.15;			// Min delay before we start looking at the position
	protected const float DELAY_S_PER_METER = 0.0015;	// How the delay increases depending on distance
	
	protected float m_fPriority;

	ref SCR_BTParam<vector> m_vPosition = new SCR_BTParam<vector>("Position");
	ref SCR_BTParam<float> m_fDuration = new SCR_BTParam<float>("Duration");
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>("Radius");
	ref SCR_BTParam<bool> m_bUseBinoculars = new SCR_BTParam<bool>("UseBinoculars");
	ref SCR_BTParam<float> m_fDelay = new SCR_BTParam<float>("Delay");
	ref SCR_BTParam<bool> m_bUseMovement = new SCR_BTParam<bool>("UseMovement");
	ref SCR_BTParam<bool> m_bLookedOnce = new SCR_BTParam<bool>("LookedOnce"); // We set it to true after we've looked at pos. once
	
	protected float m_fDeleteActionTime_ms;
	
	protected float m_fCombatMoveLogicTimeout = 0;
	
	void SCR_AIObserveUnknownFireBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity,	vector posWorld, bool useMovement, float priorityLevel = PRIORITY_LEVEL_NORMAL, float priority = PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE)
	{
		InitParameters(posWorld, useMovement);
		
		if (!utility || !utility.GetAIAgent())
			return;
				
		m_sBehaviorTree = "{AD1A56AE2A7ADFE8}AI/BehaviorTrees/Chimera/Soldier/ObservePositionBehavior.bt";
		m_bAllowLook = false; // Disable standard looking
		m_bResetLook = true;
		m_bUseCombatMove = useMovement;
		SetIsUniqueInActionQueue(true);
		m_fThreat = 1.01 * SCR_AIThreatSystem.VIGILANT_THRESHOLD;
		m_fPriority = priority;
		m_fPriorityLevel.m_Value = priorityLevel;
		
		// Calculate duration depending on distance
		IEntity controlledEntity = utility.GetAIAgent().GetControlledEntity();
		float distance;
		if (controlledEntity)
		{
			distance = vector.Distance(controlledEntity.GetOrigin(), posWorld);
			
			if (distance <= HIGH_PRIORITY_MAX_DISTANCE)
				m_fPriority = SCR_AIActionBase.PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE_HIGH_PRIORITY;
		}
			
		InitTiming(distance);
		
		if (controlledEntity)
		{
			float radius = distance * Math.Tan(Math.DEG2RAD * DIRECTION_SPAN_DEG);
			m_fRadius.m_Value = radius;
			bool inVehicleOrTurret = utility.m_AIInfo.HasUnitState(EUnitState.IN_VEHICLE) || utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET);
			m_bUseBinoculars.m_Value = !inVehicleOrTurret && distance > USE_BINOCULARS_DISTANCE_THRESHOLD;
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.COMBAT;
	}	
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(vector position, bool useMovement)
	{
		m_vPosition.Init(this, position);
		m_fDuration.Init(this, 0);
		m_fRadius.Init(this, 0);
		m_bUseBinoculars.Init(this, false);
		m_fDelay.Init(this, 0.0);
		m_bUseMovement.Init(this, useMovement);
		m_bLookedOnce.Init(this, false);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitTiming(float distance)
	{
		float duration_s = Math.Max(DURATION_MIN_S, DURATION_S_PER_METER * distance);	// Linearly increase with distance
		duration_s = Math.RandomFloat(0.7*duration_s, 1.3*duration_s);	
		m_fDuration.m_Value = duration_s;
		
		float timeout_s = Math.Max(TIMEOUT_S, duration_s);	// Timeout is quite big, but it should be smaller than duration
		InitTimeout(timeout_s);
		
		float delay_s = Math.Max(DELAY_MIN_S, DELAY_S_PER_METER * distance); // Linearly increase with distance
		delay_s = Math.RandomFloat(0.7*delay_s, 1.3*delay_s);
		m_fDelay.m_Value = delay_s;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitTimeout(float timeout_s)
	{
		float currentTime_ms = GetGame().GetWorld().GetWorldTime(); // Milliseconds!
		m_fDeleteActionTime_ms = currentTime_ms + 1000 * timeout_s;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SetUseMovement(bool value)
	{
		m_bUseMovement.m_Value = value;
		m_bUseCombatMove = value;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		if (Math.RandomFloat01() < 0.2)
		{
			if (!m_Utility.m_CommsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_UNDER_FIRE, null, vector.Zero, 0, false, true, SCR_EAITalkRequestPreset.IRRELEVANT);
				m_Utility.m_CommsHandler.AddRequest(rq);
			}
		}
		
		// If combat move is not used at all here, allow aiming immediately
		// Because aiming is blocked by combat move aiming decorator
		if (!m_bUseMovement.m_Value)
		{
			m_Utility.m_CombatMoveState.EnableAiming(true);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		// Fail action if timeout has been reached
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms > m_fDeleteActionTime_ms)
		{
			Fail();
			return 0;
		}
		
		// If we already looked at this once, lower the priority
		if (m_bLookedOnce.m_Value)
			return PRIORITY_BEHAVIOR_OBSERVE_LOW_PRIORITY;
		
		return m_fPriority;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	static bool IsNewPositionMoreRelevant(vector myWorldPos, vector oldWorldPos, vector newWorldPos)
	{
		vector vDirOld = vector.Direction(myWorldPos, oldWorldPos);
		vector vDirNew = vector.Direction(myWorldPos, newWorldPos);
		float cosAngle = vector.Dot(vDirOld, vDirNew);
		
		return cosAngle < 0.707; // cos 45 deg
	}
};

class SCR_AIGetObserveUnknownFireBehaviorParameters: SCR_AIGetActionParameters
{
	// Copied strings to avoid compilation warning about obsolete class
	static ref TStringArray s_aVarsOut = {"Position", "Duration", "Radius", "UseBinoculars", "Delay", "UseMovement", "LookedOnce"};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AISetObserveUnknownFireBehaviorParameters : SCR_AISetActionParameters
{
	static ref TStringArray s_aVarsIn = {"Position", "Duration", "Radius", "UseBinoculars", "Delay", "UseMovement", "LookedOnce"};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	static override bool VisibleInPalette() { return true; }
}