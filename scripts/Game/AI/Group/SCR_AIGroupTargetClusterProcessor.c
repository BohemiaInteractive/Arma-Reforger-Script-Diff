// Processes group's target clusters, performs decisions about what to do with them

void SCR_AITargetClusterStateChanged(SCR_AITargetClusterState state, EAITargetClusterState prevState, EAITargetClusterState newState);
typedef func SCR_AITargetClusterStateChanged;

class SCR_AIGroupTargetClusterProcessor : Managed
{
	protected const float INITIAL_DECISION_DELAY_MS = 5000.0;
	
	// When we don't receive new information from cluster for more than this amount of seconds,
	// We switch to 'LOST' state.
	protected const float MAX_CLUSTER_AGE_S = 120.0;
	
	protected SCR_AIGroupUtilityComponent m_Utility; // Owner utility component of this
	
	ref ScriptInvokerBase<SCR_AITargetClusterStateChanged> m_OnClusterStateChanged = new ScriptInvokerBase<SCR_AITargetClusterStateChanged>();
	
	//---------------------------------------------------------------------------
	void UpdateCluster(SCR_AIGroupTargetCluster c, SCR_AITargetClusterState s, float deltaTime_ms)
	{
		s.m_fTimer_ms += deltaTime_ms;
		
		if (s.m_eState != s.m_ePrevState)
		{
			OnLeaveState(s, s.m_ePrevState);
			OnEnterState(s, s.m_eState);
		}
		
		EAITargetClusterState newDesiredState = EvaluateNewDesiredState(s);
		
		switch (s.m_eState)
		{
			case EAITargetClusterState.NONE:
			{
				if (newDesiredState == s.m_eState)
				{
					// Do nothing
					if (s.m_Activity)
					{
						UnassignActivity(s);
					}
				}
				else
				{
					if (s.m_fTimer_ms > INITIAL_DECISION_DELAY_MS)
					{
						SwitchToState(s, newDesiredState);
					}
				}				
				break;
			}
			
			
			case EAITargetClusterState.LOST:
			{
				if (newDesiredState == s.m_eState)
				{
					// Do nothing
					if (s.m_Activity)
					{
						UnassignActivity(s);
					}
				}
				else
				{
					SwitchToState(s, newDesiredState);
				}
				break;
			}
			
			case EAITargetClusterState.ATTACKING:		
			case EAITargetClusterState.INVESTIGATING:
			case EAITargetClusterState.DEFENDING:
			{
				if (newDesiredState == s.m_eState)
				{
					// We want to remain in this state
					
					if (!HasActivityForState(s, newDesiredState))
					{
						TFireteamLockRefArray mainFireteams = {};
						TFireteamLockRefArray auxFireteams = {};
						
						if (s.m_Activity)
						{
							// There is already activity from previous state
							// Complete it, but try to reallocate existing fireteams
							
							GetAssignedFireteams(SCR_AIFireteamsClusterActivity.Cast(s.m_Activity), mainFireteams, auxFireteams);
							UnassignActivity(s);
						}
						
						// Try to create a new activity
						SCR_AIActivityBase activity = TryCreateActivityForState(s, newDesiredState, mainFireteams, auxFireteams);
						if (activity)
						{
							AssignActivity(s, activity);
						}
					}
					else
					{
						// There is an activity for this state already,
						// If it is complete or failed, unassign it
						if (s.m_Activity)
						{
							EAIActionState activityState = s.m_Activity.GetActionState();
							if (activityState == EAIActionState.COMPLETED || activityState == EAIActionState.FAILED)
								UnassignActivity(s);
						}
					}
				}
				else
				{
					// We want to switch to a different state
					SwitchToState(s, newDesiredState);
				}
				
				break;
			}
			
			case EAITargetClusterState.UNINITIALIZED:
			{
				if (newDesiredState != s.m_eState)
				{
					SwitchToState(s, newDesiredState);
				}
				break;
			}
		}
	}
	
	//--------------------------------------------------------------------------------
	// Evaluates new desired state - what to do with this target cluster
	EAITargetClusterState EvaluateNewDesiredState(SCR_AITargetClusterState s)
	{
		vector centerPos = 0.5*(s.m_vBBMin + s.m_vBBMax);
		
		bool playerControlled = m_Utility.m_Owner.IsSlave();

		// Disregard if combat mode doesn't allow it	
		if (m_Utility.GetCombatModeActual() == EAIGroupCombatMode.HOLD_FIRE)
			return EAITargetClusterState.NONE;
		
		if (s.GetTimeSinceLastNewInformation() > s.m_fMaxAge_s)
		{
			return EAITargetClusterState.LOST;
		}
		else if (!m_Utility.IsPositionAllowed(centerPos))
		{
			// We are not allowed to go there
			
			if (s.m_iCountIdentified > 0)
			{
				// For now we don't want to use DefendFromClusterActivity, as it only corresponds to defend waypoint and collides with its functionality.
				return EAITargetClusterState.NONE;
				//return EAITargetClusterState.DEFENDING;	// We can only defend against this
			}
		}
		else
		{
			// We are allowed to go there
			
			// Don't investigate if controlled by player
			
			if (s.m_iCountIdentified > 0)
				return EAITargetClusterState.ATTACKING;
			else if ( ((s.m_iCountLost + s.m_iCountDetected + s.m_iCountDestroyed) > 0) && 
						!playerControlled)
				return EAITargetClusterState.INVESTIGATING;
		}
		
		return EAITargetClusterState.NONE;
	}
	
	void OnLeaveState(SCR_AITargetClusterState s, EAITargetClusterState oldState)
	{
	}
	
	void OnEnterState(SCR_AITargetClusterState s, EAITargetClusterState newState)
	{
		// Calculate max cluster age threshold
		s.m_fMaxAge_s = CalculateMaxAgeThreshold_s(s, newState);
	}
	
	void SwitchToState(SCR_AITargetClusterState s, EAITargetClusterState newState)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("%1: SwitchToState: %2", s, typename.EnumToString(EAITargetClusterState, newState)));
		#endif
		
		s.m_ePrevState = s.m_eState;
		s.m_eState = newState;
		m_OnClusterStateChanged.Invoke(s, s.m_ePrevState, s.m_eState);
	}
	
	void AssignActivity(notnull SCR_AITargetClusterState s, notnull SCR_AIActivityBase activity)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("%1: AssignActivity: %2", s, activity));
		#endif
		
		// Ensure a composite parallel activity for sub activities
		SCR_AIClustersCompositeAction parallelAction = SCR_AIClustersCompositeAction.Cast(m_Utility.FindActionOfType(SCR_AIClustersCompositeAction));
		if (!parallelAction)
		{
			parallelAction = new SCR_AIClustersCompositeAction();
			m_Utility.AddAction(parallelAction);
		}
		parallelAction.AddAction(activity);
		
		s.m_Activity = activity;
	}
	
	void UnassignActivity(SCR_AITargetClusterState s)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("%1: UnassignActivity: %2", s, s.m_Activity));
		#endif
		
		if (s.m_Activity)
		{
			EAIActionState activityState = s.m_Activity.GetActionState();
			if (activityState != EAIActionState.FAILED && activityState != EAIActionState.COMPLETED)
				s.m_Activity.Complete();
		}
		
		s.m_Activity = null;
	}
	
	bool HasActivityForState(SCR_AITargetClusterState s, EAITargetClusterState eState)
	{
		switch (eState)
		{
			// Return true if there is an activity of proper class
			case EAITargetClusterState.INVESTIGATING:
				return SCR_AIInvestigateClusterActivity.Cast(s.m_Activity) != null;
			case EAITargetClusterState.ATTACKING:
				return SCR_AIAttackClusterActivity.Cast(s.m_Activity) != null;
			case EAITargetClusterState.DEFENDING:
				return SCR_AIDefendFromClusterActivity.Cast(s.m_Activity) != null;
		}
		
		return false;
	}
	
	SCR_AIActivityBase TryCreateActivityForState(SCR_AITargetClusterState s, EAITargetClusterState estate, notnull TFireteamLockRefArray inFtsMain, notnull TFireteamLockRefArray inFtsAux)
	{
		TFireteamLockRefArray ftsMain;
		TFireteamLockRefArray ftsAux;
		
		// ---- Try to allocate new fireteams, or reuse existing
		array<SCR_AIGroupFireteam> newFireteams = {};
		switch (estate)
		{
			case EAITargetClusterState.INVESTIGATING:
			case EAITargetClusterState.ATTACKING:
			{
				//-----------------------------------
				// Main fireteams
				
				// Reuse previous main fireteams if provided
				if (!inFtsMain.IsEmpty())
					ftsMain = SCR_AIGroupFireteamLock.CopyLockArray(inFtsMain); // Reuse old fireteams
				else
					ftsMain = {};
				
				// Reuse previous aux fireteams if provided
				if (!inFtsAux.IsEmpty())
					ftsAux = SCR_AIGroupFireteamLock.CopyLockArray(inFtsAux); // Reuse old fireteams
				else
					ftsAux = {};
				
				// Allocate even more fireteams if needed
				AllocateMoreFireteams(s, ftsMain, ftsAux);
				
				// If by now we have nothing, it's pointless
				if (ftsMain.IsEmpty())
					return null;
				
				
				//-----------------------------------
				// Aux fireteams
				
				
				
				// Try to ensure at least one aux. fireteam
				// If we have many main fireteams, distribute some to aux fireteams
				if (ftsAux.IsEmpty())
				{
					if (ftsMain.Count() > 1)
					{
						SCR_AIGroupFireteamLock ftLock = ftsMain[ftsMain.Count()-1];
						ftsMain.Remove(ftsMain.Count()-1);
						ftsAux.Insert(ftLock);
					}
					else if (m_Utility.m_FireteamMgr.FindFreeFireteams(newFireteams, 1, SCR_AIGroupFireteam))
					{
						SCR_AIGroupFireteamLock.TryLockFireteams(newFireteams, ftsAux, true);
					}
				}
				else if(ftsAux.Count() > 1)
				{
					// So far we don't want more than 1 aux fireteam to keep AI more engaged
					while (ftsAux.Count() > 1)
					{
						SCR_AIGroupFireteamLock ftLock = ftsAux[ftsAux.Count()-1];
						ftsMain.Insert(ftLock);
						ftsAux.Remove(ftsAux.Count()-1);
					}
				}
				
				break;
			}
			
			case EAITargetClusterState.DEFENDING:
			{
				if (!inFtsMain.IsEmpty() || !inFtsAux.IsEmpty())
				{
					// Here we need only one array of fireteams
					// Combine all previous main and aux fireteams into one array
					ftsMain = {};
					foreach (auto ft : inFtsMain)
						ftsMain.Insert(ft);
					foreach (auto ft : inFtsAux)
						ftsAux.Insert(ft);
					
					ftsAux = {};
				}
				else if (m_Utility.m_FireteamMgr.FindFreeFireteams(newFireteams, 1, SCR_AIGroupFireteam))
				{
					ftsMain = {};
					SCR_AIGroupFireteamLock.TryLockFireteams(newFireteams, ftsMain, true);
					ftsAux = {};
				}
				else
				{
					// Failed to find any fireteams
					return null;
				}
				
				break;
			}
			
			default:
			{
				// Should not be possible to call this function for those states
				return null;
			}
		}
		
		
		
		// ---- Create activity
		// At this point we know we have found fireteams
		SCR_AIActivityBase activity = null;
		switch (estate)
		{
			case EAITargetClusterState.INVESTIGATING:
			{
				activity = new SCR_AIInvestigateClusterActivity(m_Utility, null, s, ftsMain, ftsAux);
				//m_Utility.AddAction(activity);
				//return activity;
				break;
			}
			case EAITargetClusterState.ATTACKING:
			{
				activity = new SCR_AIAttackClusterActivity(m_Utility, null, s, ftsMain, ftsAux);
				//m_Utility.AddAction(activity);
				//return activity;
				break;
			}
			case EAITargetClusterState.DEFENDING:
			{
				activity = new SCR_AIDefendFromClusterActivity(m_Utility, null, s, ftsMain);
				//m_Utility.AddAction(activity);
				//return activity;
				break;
			}
		}
		
		return activity;
	}
	
	void AllocateMoreFireteams(SCR_AITargetClusterState s, notnull TFireteamLockRefArray inOutFtLocksMain, notnull TFireteamLockRefArray ftLocksAux)
	{
		// We slightly overestimate amount of enemies to allocate even more people
		float fEnemies = 1.3 * (float)s.m_iCountDetected + 1.3*s.m_iCountIdentified + 1.3*s.m_iCountLost + 0.5*s.m_iCountDestroyed;
		
		int nEnemies = Math.Ceil(fEnemies);
		
		// Count soldiers from what we have so far
		int nSoldiersAllocated = 0;
		foreach (SCR_AIGroupFireteamLock ftLock : inOutFtLocksMain)
			nSoldiersAllocated += ftLock.GetFireteam().GetMemberCount();
		foreach (SCR_AIGroupFireteamLock ftLock : ftLocksAux)
			nSoldiersAllocated += ftLock.GetFireteam().GetMemberCount();
		
		// Allocate fireteams
		array<SCR_AIGroupFireteam> freeFireteams = {};
		m_Utility.m_FireteamMgr.GetFreeFireteams(freeFireteams, SCR_AIGroupFireteam);
		while (nSoldiersAllocated < nEnemies && !freeFireteams.IsEmpty())
		{
			SCR_AIGroupFireteam newFireteam = freeFireteams[0];
			SCR_AIGroupFireteamLock newFtLock = newFireteam.TryLock();
			inOutFtLocksMain.Insert(newFtLock);
			freeFireteams.Remove(0);
			
			nSoldiersAllocated += newFireteam.GetMemberCount();
		}
	}	
	
	//! Returns arrays with fireteams already assigned to this cluster, if there are any. Otherwise returns empty arrays.
	void GetAssignedFireteams(notnull SCR_AIFireteamsActivity fromActivity, notnull TFireteamLockRefArray outMainFireteams, notnull TFireteamLockRefArray outAuxFireteams)
	{
		outMainFireteams.Clear();
		outAuxFireteams.Clear();
		
		if (SCR_AIInvestigateClusterActivity.Cast(fromActivity))
		{
			SCR_AIInvestigateClusterActivity.Cast(fromActivity).GetSpecificFireteams(outMainFireteams, outAuxFireteams);
			return;
		}
		else if (SCR_AIAttackClusterActivity.Cast(fromActivity))
		{
			SCR_AIAttackClusterActivity.Cast(fromActivity).GetSpecificFireteams(outMainFireteams, outAuxFireteams);
			return;
		}
		else if (SCR_AIDefendFromClusterActivity.Cast(fromActivity))
		{
			SCR_AIDefendFromClusterActivity.Cast(fromActivity).GetAssignedFireteams(outMainFireteams);
			return;
		}
		return;
	}
	
	//! Calculates how long we should investigate this target cluster
	float CalculateMaxAgeThreshold_s(SCR_AITargetClusterState s, EAITargetClusterState newState)
	{
		int countAlive = s.m_iCountLost + s.m_iCountDetected + s.m_iCountIdentified;
		
		if (countAlive > 0)
		{
			// If some targets are still alive
			return MAX_CLUSTER_AGE_S;
		}
		else
		{
			// Everything is destroyed, investigation time is lower
			vector ourPos = m_Utility.m_Owner.GetCenterOfMass();
			vector tgtPos = 0.5 * (s.m_vBBMin + s.m_vBBMax);
			float distance = vector.DistanceXZ(ourPos, tgtPos);
			float tgtCount = s.m_iCountDestroyed + s.m_iCountIdentified;
			
			const float movementSpeed = 2.0; // Speed in m/s
			float duration_s = distance / movementSpeed + 15.0 * tgtCount;
			duration_s = Math.Max(20.0, duration_s);
			
			return duration_s;
		}
	}
	
	void SCR_AIGroupTargetClusterProcessor(SCR_AIGroupUtilityComponent utility)
	{
		m_Utility = utility;
	}
	
	#ifdef AI_DEBUG
	void AddDebugMessage(string str)
	{
		SCR_AIInfoBaseComponent infoComp = m_Utility.m_GroupInfo;
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.CLUSTER);
	}
	#endif
}