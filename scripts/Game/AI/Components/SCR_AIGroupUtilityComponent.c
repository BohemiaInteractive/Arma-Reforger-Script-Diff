void ScriptInvoker_GroupMoveFailed_Callback(int moveResult, IEntity vehicleUsed, bool isWaypointRelated, vector moveLocation);
typedef func ScriptInvoker_GroupMoveFailed_Callback;
typedef ScriptInvokerBase<ScriptInvoker_GroupMoveFailed_Callback> ScriptInvoker_GroupMoveFailed;

enum EAIGroupCombatMode
{
	FIRE_AT_WILL = 0,
	HOLD_FIRE,
	RETURN_FIRE
}

[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system for groups")]
class SCR_AIGroupUtilityComponentClass : SCR_AIBaseUtilityComponentClass
{
}

class SCR_AIGroupUtilityComponent : SCR_AIBaseUtilityComponent
{
	static const float AUTONOMOUS_DISTANCE_MAX_DEFAULT = 9999;
	
	SCR_AIGroup m_Owner;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIGroupInfoComponent m_GroupInfo;
	SCR_AIGroupMovementComponent m_GroupMovementComponent;
	SCR_MailboxComponent m_Mailbox;
	SCR_AIGroupSettingsComponent m_SettingsComponent;
	ref array<SCR_AIInfoComponent> m_aInfoComponents = {};
	
	ref ScriptInvoker_GroupMoveFailed m_OnMoveFailed;
	ref ScriptInvokerBase<SCR_AIOnAgentLifeStateChanged> m_OnAgentLifeStateChanged = new ScriptInvokerBase<SCR_AIOnAgentLifeStateChanged>();
	
	// Settings for cluster suppression
	const float SUPPRESS_MAX_CLUSTER_INFO_AGE_S = 90; // For how long since last info we'll be suppressing cluster
	const float SUPPRESS_OLD_CLUSTER_INFO_AGE_S = 15; // Time since last cluster info that we'll consider as old (starts scaling of fire rate to save ammo)
	const float SUPPRESS_MAX_DESTROYED_CLUSTER_INFO_AGE_S = 10; // How long to suppress a target cluster which has only destroyed targets
	const float SUPPRESS_MIN_DIST_TO_CLUSTER_M = 30; // What is the minimal distance of units to suppression bbox to stop firing
	const float SUPPRESS_MAX_DIST_TO_CLUSTER_M = 800; // Max distance ...
	
	protected float m_fLastUpdateTime = -1.0;
	protected float m_fPerceptionUpdateTimer_ms;
	
	// See SetMaxAutonomousDistance()
	protected float m_fMaxAutonomousDistance = AUTONOMOUS_DISTANCE_MAX_DEFAULT;
	
	// Update interval of group perception and target clusters and their processing
	protected const float PERCEPTION_UPDATE_TIMER_MS = 2000.0;
	
	protected bool m_bNewGroupMemberAdded;
	protected ref SCR_AIActionBase m_CurrentActivity;
	
	// Waypoint state
	protected ref SCR_AIWaypointState m_WaypointState;
	
	// Group perception and clusters
	ref SCR_AIGroupPerception m_Perception;
	
	protected ref SCR_AIGroupTargetClusterProcessor m_TargetClusterProcessor;
	
	// Fireteams
	ref SCR_AIGroupFireteamManager m_FireteamMgr;
	
	// Vehicles
	ref SCR_AIGroupVehicleManager m_VehicleMgr;
	
	// Used by SCR_AIGetMemberByGoal nodes
	int m_iGetMemberByGoalNextIndex = 0;
	
	// Current group's threat measure
	protected float m_fThreatMeasure;
	
	// Current group's fire rate (external)
	protected float m_fFireRateCoef = 1;	
	
	// Currently suppressed cluster state
	protected ref SCR_AITargetClusterState m_CurrentSuppressClusterState;
	
	// Combat modes
	protected EAIGroupCombatMode m_eCombatModeExternal = EAIGroupCombatMode.FIRE_AT_WILL;	// Combat mode dictated by external settings (scenario, commanding, game master)
	protected EAIGroupCombatMode m_eCombatModeActual = EAIGroupCombatMode.FIRE_AT_WILL;	// Actual combat mode at given moment, if automatic combat mode is used
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateThreatMeasure()
	{
		// Exit if no agents
		float count = m_aInfoComponents.Count();
		if (count == 0)
			return;
		
		float threat = 0;
		foreach (SCR_AIInfoComponent infoComp : m_aInfoComponents)
			threat += infoComp.GetThreatSystem().GetThreatMeasure();
		
		threat /= count;
		m_fThreatMeasure = threat;
	}
	
	//------------------------------------------------------------------------------------------------
	// Sets group's fire rate coef
	void SetFireRateCoef(float coef = 1, bool overridePersistent = false)
	{
		m_fFireRateCoef = coef;
		
		foreach (SCR_AIInfoComponent infoComp : m_aInfoComponents)
		{
			SCR_AICombatComponent comp = infoComp.GetCombatComponent();
			if (comp)
				comp.SetGroupFireRateCoef(coef, overridePersistent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Get group's current fire rate coef
	float GetFireRateCoef()
	{
		return m_fFireRateCoef;
	}
	
	//------------------------------------------------------------------------------------------------
	// Get group's current threat measure
	float GetThreatMeasure()
	{
		return m_fThreatMeasure;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets max distance for various autonomous behaviors.
	void SetMaxAutonomousDistance(float dist)
	{
		m_fMaxAutonomousDistance = Math.Max(0, dist);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxAutonomousDistance()
	{
		return m_fMaxAutonomousDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets combat mode as desired by external means, such as scenario, or commanding, or game master.
	void SetCombatMode(EAIGroupCombatMode combatMode)
	{
		m_eCombatModeExternal = combatMode;
	}
	
	//------------------------------------------------------------------------------------------------
	//! See SetCombatMode
	EAIGroupCombatMode GetCombatModeExternal()
	{
		return m_eCombatModeExternal;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns actual combat mode. See comment to EvaluateCombatMode method.
	EAIGroupCombatMode GetCombatModeActual()
	{
		return m_eCombatModeActual;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] restartActivity
	//! \return
	SCR_AIActionBase EvaluateActivity(out bool restartActivity)
	{
		SCR_AIActionBase activity;
		restartActivity = false;
		
		if (!m_ConfigComponent)
			return null;
		
		float currentTime = GetGame().GetWorld().GetWorldTime();
		float deltaTime_ms = 0;
		if (m_fLastUpdateTime != -1.0)
			deltaTime_ms = currentTime - m_fLastUpdateTime;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateActivity breakpoint triggered");
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
		// Read messages
		AIMessage msgBase = m_Mailbox.ReadMessage(true);
		if (msgBase)
		{
			SCR_AIMessageGoal msgGoal = SCR_AIMessageGoal.Cast(msgBase);
			if (msgGoal)
			{
				// Process goal message
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformGoalReaction: %1, from BT: %2", msgGoal, msgGoal.m_sSentFromBt));
				#endif
				m_ConfigComponent.PerformGoalReaction(this, msgGoal);
			}
			else
			{
				SCR_AIMessageInfo msgInfo = SCR_AIMessageInfo.Cast(msgBase);
				if (msgInfo)
				{
					// Process info message
					
					bool overrideReaction = CallActionsOnMessage(msgInfo);
			
					if (!overrideReaction)
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("PerformInfoReaction: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
						
						m_ConfigComponent.PerformInfoReaction(this, msgInfo);
					}
					#ifdef AI_DEBUG
					else
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("InfoMessage consumed by action: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
					}
					#endif
				}
			}
		}
			
		RemoveObsoleteActions();
		
		activity = SCR_AIActionBase.Cast(EvaluateActions());
		#ifdef AI_DEBUG
		DiagIncreaseCounter();
		DebugLogActionsPriority();
		#endif
		
		if (activity && (!m_CurrentActivity || (m_CurrentActivity != activity && m_CurrentActivity.IsActionInterruptable())))
			restartActivity = true;
		else if (m_bNewGroupMemberAdded && activity)
			restartActivity = true;
			
		if (restartActivity)
		{
			SetCurrentAction(activity);
			UpdateGroupControlMode(activity);
			m_CurrentActivity = activity;
			
#ifdef WORKBENCH
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_PRINT_ACTIVITY))
				PrintFormat("Agent %1 activity %2",m_Owner,m_CurrentActivity.GetActionDebugInfo());
#endif
		}
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateActivity END\n");
		#endif
		
		// Update group perception, update fire teams
		bool isMilitary = IsMilitary();
		bool isSlave = m_Owner.IsSlave();
		if (isMilitary)
		{
			if (!isSlave && m_FireteamMgr.m_bRebalanceFireteams)
			{
				if (CanRebalanceFireteams()) // In some cases we can't rebalance fireteams yet
				{
					m_FireteamMgr.RebalanceAllFireteams();
				}
			}
		
			// Perception and clusters
			m_fPerceptionUpdateTimer_ms += deltaTime_ms;
			if (m_fPerceptionUpdateTimer_ms > PERCEPTION_UPDATE_TIMER_MS)
			{
				m_Perception.Update();
				
				if (!isSlave)
				{
					UpdateSuppressCluster();
					UpdateThreatMeasure();
					EvaluateFlareUsage();
					
					if (!m_Perception.m_aTargetClusters.IsEmpty())
						UpdateClustersState(m_fPerceptionUpdateTimer_ms);
				}
				
				m_fPerceptionUpdateTimer_ms -= PERCEPTION_UPDATE_TIMER_MS;
			}
		}
		
		// Combat modes
		if (isMilitary)
			EvaluateCombatMode();
		
		m_fLastUpdateTime = currentTime;
		m_bNewGroupMemberAdded = false; // resetting reaction on group member added
		
		return m_CurrentActivity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resolves which actual combat mode the group has now, based on external combat mode.
	//! Actual combat mode can be only either HOLD_FIRE or FIRE_AT_WILL,
	//! Depending on external combat mode value, and if gorup considers itself under attack or not.
	void EvaluateCombatMode()
	{
		if (m_eCombatModeExternal != EAIGroupCombatMode.RETURN_FIRE)
		{
			// Simple case - actual combat mode is same as external
			m_eCombatModeActual = m_eCombatModeExternal;
		}
		else
		{
			// Automatic - actual combat mode depends if we're under attack or not
			foreach (SCR_AIGroupTargetCluster c : m_Perception.m_aTargetClusters)
			{
				if (c.m_State && c.m_State.m_iCountEndangering != 0 && c.m_State.m_iCountAlive != 0)
				{
					// Any target cluster is endangering us
					m_eCombatModeActual = EAIGroupCombatMode.FIRE_AT_WILL;
					return;
				}
			}
			
			// Nothing is endangering us
			m_eCombatModeActual = EAIGroupCombatMode.HOLD_FIRE;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void EvaluateFlareUsage()
	{
		if (!SCR_AIWorldHandling.IsLowLightEnvironment() || !m_GroupInfo || !m_GroupInfo.IsIllumFlareAllowed())
			return;
		
		if (!m_CurrentSuppressClusterState)
			return;

		vector targetPosition = m_CurrentSuppressClusterState.GetCenterPosition();
		
		SCR_AIActivityIllumFlareFeature illumFeature = new SCR_AIActivityIllumFlareFeature();	
		if (illumFeature.Execute(this, targetPosition, null))
			m_GroupInfo.OnIllumFlareUsed();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AITargetClusterState GetCurrentSuppressClusterState()
	{
		return m_CurrentSuppressClusterState;
	}
	
	//------------------------------------------------------------------------------------------------
	bool AnyMembersInsideBBox(vector bbMin, vector bbMax)
	{
		// Check if any friends are inside bbox
		foreach (SCR_AIInfoComponent infoComp : m_aInfoComponents)
		{
			AIAgent agent = AIAgent.Cast(infoComp.GetOwner());
			if (!agent)
				continue;
				
			IEntity character = agent.GetControlledEntity();
			if (!character)
				continue;
				
			vector unitPos = character.GetOrigin();

			if (Math.IsInRange(unitPos[0], bbMin[0], bbMax[0]) && Math.IsInRange(unitPos[1], bbMin[1], bbMax[1]) && Math.IsInRange(unitPos[2], bbMin[2], bbMax[2]))
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool ShouldSuppressCluster(SCR_AITargetClusterState clusterState)
	{
		// Disregard if combat mode doesn't allow it
		if (m_eCombatModeActual == EAIGroupCombatMode.HOLD_FIRE)
			return false;
		
		// Ignore clusters without endangering & identified targets or members too close
		if (clusterState.m_iCountIdentified == 0 && clusterState.m_iCountEndangering == 0)
			return false;
		
		// Ignore clusters too close or too far
		if (clusterState.m_fDistMin < SUPPRESS_MIN_DIST_TO_CLUSTER_M || clusterState.m_fDistMin > SUPPRESS_MAX_DIST_TO_CLUSTER_M)
			return false;
		
		// Max suppression time
		float maxSuppressionTime = SUPPRESS_MAX_CLUSTER_INFO_AGE_S;
		if (clusterState.m_iCountAlive == 0)
			maxSuppressionTime = SUPPRESS_MAX_DESTROYED_CLUSTER_INFO_AGE_S;
		
		// Ignore too old clusters
		if (clusterState.GetTimeSinceLastNewInformation() > maxSuppressionTime)
			return false;
		
		vector bbMin = clusterState.m_vBBMin;
		vector bbMax = clusterState.m_vBBMax;
		
		// Scale BBox
		SCR_AISuppressionObjectVolumeBox.ScaleTargetBBox(bbMin, bbMax, clusterState.m_fDistMin, clusterState.m_iCountIdentified > 0);
			
		// Ignore cluster if some group members are inside it
		if (AnyMembersInsideBBox(bbMin, bbMax))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSuppressCluster()
	{
		SCR_AIGroupTargetCluster cluster = m_Perception.m_MostDangerousCluster;		
		SCR_AITargetClusterState state;
		
		if (cluster)
		{
			state = cluster.m_State;
			
			if (!ShouldSuppressCluster(state))
				state = null;
		}
		
		if (state != m_CurrentSuppressClusterState)
		{
			m_CurrentSuppressClusterState = state;
			
			// Alert agents about change in cluster
			foreach (SCR_AIInfoComponent infoComp : m_aInfoComponents)
			{
				SCR_AICombatComponent comp = infoComp.GetCombatComponent();
				if (comp)
					comp.SetGroupSuppressClusterState(state);
			}		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates info of group members to planner - should be called when adding or removing group member
	//! \param[in] agent
	protected void OnAgentAdded(AIAgent agent)
	{
		// Add to array of AIInfo
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimeraAgent)
			return;

		SCR_AIInfoComponent info = chimeraAgent.m_InfoComponent;
		
		if (!info)
			return;
		
		m_aInfoComponents.Insert(info);	
		
		m_FireteamMgr.OnAgentAdded(agent);
		
		info.m_OnCompartmentEntered.Insert(OnAgentCompartmentEntered);
		info.m_OnCompartmentLeft.Insert(OnAgentCompartmentLeft);
		info.m_OnAgentLifeStateChanged.Insert(OnAgentLifeStateChanged);
		m_bNewGroupMemberAdded = true;
		
		if (info.HasUnitState(EUnitState.IN_VEHICLE))
		{
			OnJoinGroupFromVehicle(agent, info.HasUnitState(EUnitState.PILOT));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] agent
	protected void OnAgentRemoved(SCR_AIGroup group, AIAgent agent)
	{	
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
		if(!utility)
			return Debug.Error("Null AI utility");
			
		if(agent)
			utility.CancelAllGroupActivityBehaviors(this);
		
		// Remove from array of AIInfo
		for (int i = m_aInfoComponents.Count() - 1; i >= 0; i--)
		{
			if (!m_aInfoComponents[i])
			{
				Debug.Error("Null AI info occured"); // investigate when this happens!
				m_aInfoComponents.RemoveOrdered(i);
			}
			else if (m_aInfoComponents[i].IsOwnerAgent(agent))
			{
				// Unsubscribe from compartment event
				SCR_AIInfoComponent infoComp = m_aInfoComponents[i];
				infoComp.m_OnCompartmentEntered.Remove(OnAgentCompartmentEntered);
				infoComp.m_OnCompartmentLeft.Remove(OnAgentCompartmentLeft);
				
				m_aInfoComponents.RemoveOrdered(i);
				break;
			}
		}
		
		m_FireteamMgr.OnAgentRemoved(agent);
		m_VehicleMgr.OnAgentRemoved(agent, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] waypoint
	protected void OnWaypointCompleted(AIWaypoint waypoint)
	{
		if (m_WaypointState && waypoint && m_WaypointState.GetWaypoint() == waypoint)
		{
			m_WaypointState.OnDeselected();
			m_WaypointState = null;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] waypoint
	//! \param[in] isCurrentWaypoint
	protected void OnWaypointToRemove(AIWaypoint waypoint, bool isCurrentWaypoint)
	{
		// Remove old wp state, if it existed
		if (isCurrentWaypoint)
		{
			if (waypoint && m_WaypointState && m_WaypointState.GetWaypoint() == waypoint)
				m_WaypointState.OnDeselected();
			
			m_WaypointState = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAgentCompartmentEntered(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!agent || !targetEntity)
			return;
		// ignoring attachments such as turrets, cargo slots, etc.
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(targetEntity, targetEntity);
		if (!vehicleUsageComp)
		{
			SCR_AIVehicleUsageComponent.ErrorNoComponent(targetEntity);
			return;
		}
		
		if (!vehicleUsageComp.IsVehicleTypeValid())
		{
			SCR_AIVehicleUsageComponent.ErrorIncorrectType(targetEntity);
			return;
		}
		
		AddUsableVehicle(vehicleUsageComp);
		
		// Not from our group? Should not happen.
		if (agent.GetParentGroup() != m_Owner)
			return;
		
		BaseCompartmentSlot compSlot =  manager.FindCompartment(slotID, mgrID);
		
		if (m_GroupMovementComponent && compSlot)
		{
			bool isDriving = compSlot.GetType() == ECompartmentType.PILOT;
			SCR_AIGroupVehicle groupVehicle = m_VehicleMgr.FindVehicle(vehicleUsageComp);
			int vehicleId = groupVehicle.GetSubgroupHandleId();
			if (vehicleId > -1) // agent entered vehicle with subgroup
				JoinSubgroup(agent, vehicleId, isDriving);
			else	// agent entered vehicle, that does not have subgroup, yet
				CreateNewSubgroup(agent, isDriving, groupVehicle);
		}
		
		m_FireteamMgr.OnAgentAssignedToVehicle(agent, vehicleUsageComp, compSlot.GetType());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void JoinSubgroup(AIAgent agent, int handleId, bool isSubgroupLeader)
	{
		
		int oldHandleId = m_GroupMovementComponent.GetAgentMoveHandlerId(agent);
		if (isSubgroupLeader)
			m_GroupMovementComponent.SetMoveHandlerLeader(agent, oldHandleId, handleId);
		else
			m_GroupMovementComponent.MoveAgentToHandler(agent, oldHandleId, handleId);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnJoinGroupFromVehicle(AIAgent agent, bool isDriving)
	{
		ChimeraCharacter charact = ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!charact)
			return false;
		CompartmentAccessComponent comp = charact.GetCompartmentAccessComponent();
		if (!comp)
			return false;
		IEntity vehicleEntity = comp.GetVehicleIn(charact);
		if (!vehicleEntity)
			return false;
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicleEntity, vehicleEntity);
		if (!vehicleUsageComp)
			return false;
			
		BaseCompartmentSlot compartment = comp.GetCompartment();
		if (!compartment)
			return false;
		
		OnAgentCompartmentEntered(agent, vehicleEntity, compartment.GetManager(), compartment.GetCompartmentMgrID(), compartment.GetCompartmentSlotID(), false); 
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int CreateNewSubgroup(AIAgent agent, bool isSubgroupLeader, SCR_AIGroupVehicle groupVehicle)
	{
		int oldHandleId = m_GroupMovementComponent.GetAgentMoveHandlerId(agent);
		AIFormationDefinition formationDef = m_GroupMovementComponent.GetFormationDefinition(oldHandleId);
		if (!formationDef)
			return -1;
		string formationType = formationDef.GetName();
		int newHandleId = m_GroupMovementComponent.CreateGroupMoveHandler(formationType);
		if (isSubgroupLeader)
			m_GroupMovementComponent.SetMoveHandlerLeader(agent, oldHandleId, newHandleId);
		else
			m_GroupMovementComponent.MoveAgentToHandler(agent, oldHandleId, newHandleId);
		groupVehicle.SetSubgroupHandleId(newHandleId);
		return newHandleId;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LeaveSubgroup(AIAgent agent, SCR_AIGroupVehicle groupVehicle)
	{
		int oldHandleId = m_GroupMovementComponent.GetAgentMoveHandlerId(agent);
		m_GroupMovementComponent.MoveAgentToHandler(agent, oldHandleId, AIGroupMovementComponent.DEFAULT_HANDLER_ID);		
		bool isHandleEmpty = m_GroupMovementComponent.GetMoveHandlerAgentCount(oldHandleId) == 0;
		if (isHandleEmpty)
		{
			m_GroupMovementComponent.RemoveGroupMoveHandler(oldHandleId);
			if (groupVehicle)
				groupVehicle.SetSubgroupHandleId();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAgentCompartmentLeft(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!agent || !targetEntity)
			return;
		
		// ignoring attachments such as turrets, cargo slots, etc.
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(targetEntity, targetEntity);
		
		// Not from our group? Should not happen.
		if (agent.GetParentGroup() != m_Owner)
			return;
		
		SCR_AIGroupVehicle groupVehicle = m_VehicleMgr.FindVehicle(targetEntity);
		LeaveSubgroup(agent,groupVehicle);
		
		if (!vehicleUsageComp)
		{
			SCR_AIVehicleUsageComponent.ErrorNoComponent(targetEntity);
			return;
		}
		
		m_FireteamMgr.OnAgentUnassignedFromVehicle(agent, vehicleUsageComp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAgentLifeStateChanged(AIAgent incapacitatedAgent, SCR_AIInfoComponent infoIncap, IEntity vehicle, ECharacterLifeState lifeState)
	{
		m_OnAgentLifeStateChanged.Invoke(incapacitatedAgent, infoIncap, vehicle, lifeState);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] currentWp
	//! \param[in] prevWp
	protected void OnCurrentWaypointChanged(AIWaypoint currentWp, AIWaypoint prevWp)
	{
		// Remove old wp state, if it existed
		if (m_WaypointState && prevWp && m_WaypointState.GetWaypoint() == prevWp)
			m_WaypointState.OnDeselected();
		
		m_WaypointState = null;
		
		// Create new wp state
		if (currentWp)
		{
			SCR_AIWaypoint scrCurrentWp = SCR_AIWaypoint.Cast(currentWp);
			if (scrCurrentWp)
			{
				SCR_AIWaypointState wpState = scrCurrentWp.CreateWaypointState(this);
				if (wpState)
				{
					m_WaypointState = wpState;
					m_WaypointState.OnSelected();
				}
			}
		}
	}
		
	//------------------------------------------------------------------------------------------------
	void OnExecuteWaypointTree()
	{
		if (m_WaypointState)
			m_WaypointState.OnExecuteWaypointTree();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called from m_Perception
	void OnEnemyDetectedFiltered(SCR_AIGroup group, SCR_AITargetInfo target, AIAgent reporter)
	{
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(reporter);
		if (!agent)
			return;
		
		SCR_AICommsHandler commsHandler = agent.m_UtilityComponent.m_CommsHandler;
		
		// Ignore if the talk request can be optimized out
		if (commsHandler.CanBypass())
			return;
		
		SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_CONTACT, target.m_Entity, target.m_vWorldPos,
			enumSignal: 0, transmitIfNoReceivers: true, transmitIfPassenger: true, preset: SCR_EAITalkRequestPreset.MEDIUM);
		commsHandler.AddRequest(rq);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	//! \param[in] prevState
	//! \param[in] newState
	void OnTargetClusterStateChanged(SCR_AITargetClusterState state, EAITargetClusterState prevState, EAITargetClusterState newState)
	{
		// If we've lost enemies at some place, make the assigned fireteams report that
		if (newState == EAITargetClusterState.LOST &&
			(prevState == EAITargetClusterState.INVESTIGATING || prevState == EAITargetClusterState.ATTACKING) &&
			state.m_Activity)
		{
			SCR_AIFireteamsActivity ftActivity = SCR_AIFireteamsActivity.Cast(state.m_Activity);
			if (ftActivity)
			{
				TFireteamLockRefArray fireteamLocks = {};
				ftActivity.GetAssignedFireteams(fireteamLocks);
				foreach (SCR_AIGroupFireteamLock ftLock : fireteamLocks)
				{
					AIAgent reporterAgent = ftLock.GetFireteam().GetMember(0);
					if (!reporterAgent)
						continue;
					SCR_AICommsHandler commsHandler = SCR_AISoundHandling.FindCommsHandler(reporterAgent);
					if (!commsHandler)
						continue;
					if (commsHandler.CanBypass())
						continue;
					
					SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_CLEAR, null, vector.Zero, 0, false, false, SCR_EAITalkRequestPreset.MEDIUM);
					commsHandler.AddRequest(rq);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] waypoint
	//! \param[in] activityType
	//! \param[in] doNotCompleteWaypoint
	void CancelActivitiesRelatedToWaypoint(notnull AIWaypoint waypoint, typename activityType = typename.Empty, bool doNotCompleteWaypoint = false)
	{
		array<ref AIActionBase> actions = {};
		GetActions(actions);
		bool checkType = activityType != typename.Empty;
		foreach (AIActionBase action : actions)
		{
			SCR_AIActivityBase activity = SCR_AIActivityBase.Cast(action);
			
			if (!activity || (checkType && !activity.IsInherited(activityType)))
				continue;
			
			EAIActionState state = activity.GetActionState();
			
			if (state == EAIActionState.COMPLETED || state == EAIActionState.FAILED)
				continue;
			
			if (activity.m_RelatedWaypoint == waypoint)
				activity.Fail(doNotCompleteWaypoint);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Determines when we can rebalance fireteams. We don't want to do that when fighting for example.
	protected bool CanRebalanceFireteams()
	{
		// Can't rebalance fireteams if there is any fireteams-related activity
		array<ref AIActionBase> allActions = {};
		GetActions(allActions);
		
		array<AIActionBase> subactions = {};
		foreach (AIActionBase action : allActions)
		{
			if (SCR_AIFireteamsActivity.Cast(action))
				return false;
			SCR_AICompositeActionParallel parallel = SCR_AICompositeActionParallel.Cast(action);
			if (parallel)
			{
				parallel.GetSubactions(subactions);
				foreach (AIActionBase subaction : subactions)
				{
					if (SCR_AIFireteamsActivity.Cast(subaction))
						return false;
				}
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMoveFailed(int moveResult, IEntity vehicleUsed, bool isWaypointRelated, vector moveLocation)
	{
		if (m_OnMoveFailed)
			m_OnMoveFailed.Invoke(moveResult, vehicleUsed, isWaypointRelated, moveLocation);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker_GroupMoveFailed GetOnMoveFailed()
	{
		if (!m_OnMoveFailed)
			m_OnMoveFailed = new ScriptInvoker_GroupMoveFailed();
		
		return m_OnMoveFailed;
	}

	//---------------------------------------------------------------------------------------------------	
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_Owner = SCR_AIGroup.Cast(owner);
		if (!m_Owner)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(m_Owner.FindComponent(SCR_AIConfigComponent));
		
		m_ConfigComponent.AddDefaultActivities(this);
		//AddAction(new SCR_AIIdleActivity(this))
		
		m_Owner.GetOnAgentAdded().Insert(OnAgentAdded);
		m_Owner.GetOnAgentRemoved().Insert(OnAgentRemoved);
		m_Owner.GetOnWaypointCompleted().Insert(OnWaypointCompleted);
		m_Owner.GetOnWaypointToRemove().Insert(OnWaypointToRemove);
		m_Owner.GetOnCurrentWaypointChanged().Insert(OnCurrentWaypointChanged);
		
		m_GroupInfo = SCR_AIGroupInfoComponent.Cast(m_Owner.FindComponent(SCR_AIGroupInfoComponent));
		
		m_TargetClusterProcessor = new SCR_AIGroupTargetClusterProcessor(this);
		m_TargetClusterProcessor.m_OnClusterStateChanged.Insert(OnTargetClusterStateChanged);
		
		m_FireteamMgr = new SCR_AIGroupFireteamManager(m_Owner);
		
		m_Perception = new SCR_AIGroupPerception(this, m_Owner);
		m_Perception.GetOnEnemyDetectedFiltered().Insert(OnEnemyDetectedFiltered);
		
		m_VehicleMgr = new SCR_AIGroupVehicleManager();
		
		m_Mailbox = SCR_MailboxComponent.Cast(m_Owner.FindComponent(SCR_MailboxComponent));
		
		m_SettingsComponent = SCR_AIGroupSettingsComponent.Cast(m_Owner.FindComponent(SCR_AIGroupSettingsComponent));
		
		m_fPerceptionUpdateTimer_ms = Math.RandomFloat(0, PERCEPTION_UPDATE_TIMER_MS);
		
		m_GroupMovementComponent = SCR_AIGroupMovementComponent.Cast(owner.FindComponent(SCR_AIGroupMovementComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_TARGET_CLUSTERS))
			m_Perception.DiagDrawClusters();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_FIRETEAMS))
			m_FireteamMgr.DiagDrawFireteams();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SUBGROUPS))
			m_GroupMovementComponent.DiagDrawSubgroups(m_Owner);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] currentAction
	void UpdateGroupControlMode(SCR_AIActionBase currentAction)
	{
		#ifdef AI_DEBUG
		AddDebugMessage("UpdateGroupControlMode");
		#endif
		
		if (m_GroupInfo)
		{
			SCR_AIActivityBase currentActivity = SCR_AIActivityBase.Cast(currentAction);
			
			if (currentActivity && currentActivity.m_bIsWaypointRelated.m_Value)
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.FOLLOWING_WAYPOINT);
			else if (SCR_AIIdleActivity.Cast(currentActivity))
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.IDLE);
			else
				m_GroupInfo.SetGroupControlMode(EGroupControlMode.AUTONOMOUS);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Iterates all clusters and updates their state
	//! \param[in] deltaTime_ms
	void UpdateClustersState(float deltaTime_ms)
	{
		foreach (SCR_AIGroupTargetCluster cluster : m_Perception.m_aTargetClusters)
		{
			m_TargetClusterProcessor.UpdateCluster(cluster, cluster.m_State, deltaTime_ms);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Decides whether we are allowed to go to this position
	//! \param[in] pos
	//! \return
	bool IsPositionAllowed(vector pos)
	{		
		AIWaypoint wp = m_Owner.GetCurrentWaypoint();
		
		SCR_DefendWaypoint defendWp = SCR_DefendWaypoint.Cast(wp);
		if (defendWp)
		{
			// If we have a defend wp, we can go only inside of it
			return vector.Distance(defendWp.GetOrigin(), pos) < defendWp.GetCompletionRadius();
		}
		else
		{
			// Either no waypoint, or not a defend waypoint
			// We can go anywhere, but it depends on range
			
			float distance = vector.Distance(m_Owner.GetCenterOfMass(), pos);
			
			return distance < m_fMaxAutonomousDistance;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsUsableVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		if (!vehicleUsageComp)
			return false;
		
		return m_VehicleMgr.FindVehicle(vehicleUsageComp) != null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsUsableVehicle(notnull IEntity vehicleEntity)
	{
		if (!vehicleEntity)
			return false;
		
		return m_VehicleMgr.FindVehicle(vehicleEntity) != null;
	}	
	
	//------------------------------------------------------------------------------------------------
	void AddUsableVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		m_VehicleMgr.TryAddVehicle(vehicleUsageComp);
		
		// Events
		vehicleUsageComp.GetOnDeleted().Remove(OnVehicleDeleted); // not to add it twice
		vehicleUsageComp.GetOnDeleted().Insert(OnVehicleDeleted);
		vehicleUsageComp.GetOnDamageStateChanged().Remove(OnVehicleDamageStateChanged); // not to add it twice
		vehicleUsageComp.GetOnDamageStateChanged().Insert(OnVehicleDamageStateChanged);
		
		// Create a vehicle combat activity as soon as first vehicle is added
		if (!HasActionOfType(SCR_AIVehicleCombatActivity))
		{
			auto activity = new SCR_AIVehicleCombatActivity(this, null);
			AddAction(activity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveUsableVehicle(notnull SCR_AIVehicleUsageComponent vehicleUsageComp)
	{
		// Remove from vehicle mgr
		m_VehicleMgr.RemoveVehicle(vehicleUsageComp);
		
		// Remove from fireteam mgr
		m_FireteamMgr.OnVehicleRemoved(vehicleUsageComp);
		
		// Events
		vehicleUsageComp.GetOnDeleted().Remove(OnVehicleDeleted);
		vehicleUsageComp.GetOnDamageStateChanged().Remove(OnVehicleDamageStateChanged);
		
		// Remove vehicle combat activity when we no longer have any vehicles
		if (m_VehicleMgr.GetVehiclesCount() == 0)
		{
			auto vehicleCombatActivity = FindActionOfType(SCR_AIVehicleCombatActivity);
			if (vehicleCombatActivity)
				vehicleCombatActivity.Fail();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDeleted(SCR_AIVehicleUsageComponent comp)
	{
		RemoveUsableVehicle(comp);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDamageStateChanged(SCR_AIVehicleUsageComponent comp, EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		RemoveUsableVehicle(comp);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Are we a military AI or not?
	bool IsMilitary()
	{
		SCR_Faction f = SCR_Faction.Cast(m_Owner.GetFaction());
		if (!f)
			return false;
		return f.IsMilitary();
	}
	
	// Diagnostics and debugging below
}
