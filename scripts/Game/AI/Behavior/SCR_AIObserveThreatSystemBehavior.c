//! Behavior for looking at sectors from the threat system
class SCR_AIObserveThreatSystemBehavior : SCR_AIBehaviorBase
{
	protected bool m_bBehaviorActive;
	
	protected int m_iCurrentSector;
	protected float m_fCurrentSectorDanger;
	protected SCR_EAIThreatSectorFlags m_eCurrentSectorFlags;
	protected int m_iCurrentSectorObserveCounter; // How many times we observed current sector
	
	protected WorldTimestamp m_TimestampStartHighPriorityState;
	protected float m_fHighPriorityDuration_s;
	
	protected ref SCR_AICombatMoveLogic_HideFromThreatSystem m_CombatMoveLogic;
		
	protected static const float HIGH_PRIORITY_INITIAL_DURATION_S = 6.0;
	
	//--------------------------------------------------------------------------------------------------------------------------
	void SCR_AIObserveThreatSystemBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{	
		if (!utility || !utility.GetAIAgent())
			return;
				
		m_sBehaviorTree = "{496CEB065448925C}AI/BehaviorTrees/Chimera/Soldier/ObserveThreatSystemBehavior.bt";
		m_bAllowLook = false; // Disable standard looking
		m_bUseCombatMove = true;
		
		m_CombatMoveLogic = new SCR_AICombatMoveLogic_HideFromThreatSystem(utility, this);
		
		utility.m_SectorThreatFilter.GetOnEscalationInvoker().Insert(OnThreatSectorEscalation);
		utility.m_SectorThreatFilter.GetOnMajorSectorChangedInvoker().Insert(OnMajorSectorChanged);
		utility.m_SectorThreatFilter.GetOnDamageTaken().Insert(OnDamageTaken);
			
		Reset();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void ~SCR_AIObserveThreatSystemBehavior()
	{
		if (m_Utility && m_Utility.m_SectorThreatFilter)
		{
			m_Utility.m_SectorThreatFilter.GetOnEscalationInvoker().Remove(OnThreatSectorEscalation);
			m_Utility.m_SectorThreatFilter.GetOnMajorSectorChangedInvoker().Remove(OnMajorSectorChanged);
		}
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	//! Resets whole behavior to default state
	protected void Reset()
	{
		m_bBehaviorActive = false;
		m_iCurrentSector = -1;
		m_fCurrentSectorDanger = 0;
		m_eCurrentSectorFlags = 0;
		m_fHighPriorityDuration_s = 0;
		m_iCurrentSectorObserveCounter = 0;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		// Randomly report under fire
		if (Math.RandomFloat01() < 0.2)
		{
			if (!m_Utility.m_CommsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_UNDER_FIRE, null, vector.Zero, 0, false, true, SCR_EAITalkRequestPreset.IRRELEVANT);
				m_Utility.m_CommsHandler.AddRequest(rq);
			}
		}
		
		m_CombatMoveLogic.ReactToSector(m_iCurrentSector);
		m_Utility.m_CombatMoveState.EnableAiming(true);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		m_CombatMoveLogic.Reset();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		if (!m_bBehaviorActive)
			return 0;
		
		// Makes no sense for driver
		if (m_Utility.m_AIInfo.HasUnitState(EUnitState.PILOT))
			return 0;
		
		if (m_fHighPriorityDuration_s != 0)
		{
			WorldTimestamp timestamp = GetGame().GetWorld().GetTimestamp();
			float highPriorityTimePassed_s = timestamp.DiffSeconds(m_TimestampStartHighPriorityState);
			if (highPriorityTimePassed_s > m_fHighPriorityDuration_s)
			{
				m_iCurrentSectorObserveCounter++;
				StopHighPriorityState();
			}
		}

		if (m_fHighPriorityDuration_s != 0)
			return PRIORITY_BEHAVIOR_OBSERVE_THREATS_HIGH_PRIORITY;
		
		// Not in high priority state
		if (m_Utility.ShouldKeepFormation())
		{
			if (!m_Utility.GetSubformationLeaderMoving() && m_Utility.GetNearSubformationLeader())
				return PRIORITY_BEHAVIOR_OBSERVE_THREATS_LOW_PRIORITY;
			else
				return 0; // Subformation leader wants to move, and we must follow him, so disregard looking at this threat further
		}
		else
			return PRIORITY_BEHAVIOR_OBSERVE_THREATS_LOW_PRIORITY;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	override void OnActionExecuted()
	{
		if (!m_Utility.m_AIInfo.HasUnitState(EUnitState.IN_VEHICLE))
			m_CombatMoveLogic.Update();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void OnThreatSectorEscalation(SCR_AISectorThreatFilter ts, int sectorId, float dangerValue)
	{
		// Don't care if it's not about the current sector
		if (sectorId != m_iCurrentSector)
			return;
		
		m_fCurrentSectorDanger = dangerValue;
		m_CombatMoveLogic.ReactToSector(sectorId);
		SwitchToHighPriorityState(HIGH_PRIORITY_INITIAL_DURATION_S);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void OnMajorSectorChanged(SCR_AISectorThreatFilter ts, int newSectorId, int oldSectorId, float dangerValue)
	{
		if (newSectorId != -1)
		{
			if (dangerValue > m_fCurrentSectorDanger || m_iCurrentSector == -1)
			{
				SwitchToHighPriorityState(HIGH_PRIORITY_INITIAL_DURATION_S);
				m_CombatMoveLogic.ReactToSector(newSectorId);
			}
			
			m_bBehaviorActive = true;
			m_iCurrentSector = newSectorId;
			m_fCurrentSectorDanger = dangerValue;
			m_eCurrentSectorFlags = ts.GetSectorFlags(newSectorId);
		}
		else
		{
			// Major sector id switched to -1, this means there is no threat in threat system any more
			m_CombatMoveLogic.Reset();
			Reset();
		}
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void OnDamageTaken(SCR_AISectorThreatFilter ts, int sectorId)
	{
		if (sectorId != m_iCurrentSector)
			return;
		
		// Every time we take damage, interrupt current behavior and try to find a new position 		
		SwitchToHighPriorityState(HIGH_PRIORITY_INITIAL_DURATION_S);
		m_CombatMoveLogic.ReactToSector(sectorId);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	//! Called by combat movement logic
	void OnMovementCompleted(bool inCover)
	{
		if (GetActionState() != EAIActionState.RUNNING || !m_bBehaviorActive || m_iCurrentSector == -1 || !m_Utility.m_SectorThreatFilter.IsSectorActive(m_iCurrentSector))
			return;
		
		// After movement is done, figure out how long we need to look at this, and activate high priority state again for this duration
		float observeDuration = CalculateObserveDuration(m_iCurrentSector);
		SwitchToHighPriorityState(observeDuration);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	//! How much should we look at this?
	//! More range -> bigger duration, more danger -> bigger duration
	protected float CalculateObserveDuration(int sectorId)
	{
		vector threatPos = m_Utility.m_SectorThreatFilter.GetSectorPos(sectorId);
		float threatDist = vector.Distance(m_Utility.m_OwnerEntity.GetOrigin(), threatPos);
		float baseDuration = 0.1 * threatDist;
		
		float danger = m_Utility.m_SectorThreatFilter.GetSectorDanger(sectorId);
		float dangerCoef = Math.Map(danger, 0, 10, 1, 3);
		
		float duration_s = (0.4 * baseDuration * dangerCoef) / (1.0 + m_iCurrentSectorObserveCounter);
		
		// In this combat mode we care even less
		if (m_Utility.m_CombatComponent.GetCombatMode() == EAIGroupCombatMode.HOLD_FIRE)
			duration_s *= 0.5;
		
		if (duration_s < 1.0)
			duration_s = 1.0;
		
		return duration_s * Math.RandomFloat(0.75, 1.25);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected void SwitchToHighPriorityState(float duration_s)
	{
		WorldTimestamp timestamp = GetGame().GetWorld().GetTimestamp();
		m_fHighPriorityDuration_s = duration_s;
		m_TimestampStartHighPriorityState = timestamp;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected void StopHighPriorityState()
	{
		m_fHighPriorityDuration_s = 0;
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	protected bool IsInHighPriorityState()
	{
		return m_fHighPriorityDuration_s != 0;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		if (m_eCurrentSectorFlags & SCR_EAIThreatSectorFlags.CAUSED_DAMAGE)
			return SCR_EAIBehaviorCause.DANGER_HIGH; // If we were hit once then we will get hit second time soon
		if (m_eCurrentSectorFlags & SCR_EAIThreatSectorFlags.DIRECTED_AT_ME)
			return SCR_EAIBehaviorCause.COMBAT; // If bullets are flying by then it's clearly a combat situation
		else
			return SCR_EAIBehaviorCause.DANGER_LOW;
	}
};
