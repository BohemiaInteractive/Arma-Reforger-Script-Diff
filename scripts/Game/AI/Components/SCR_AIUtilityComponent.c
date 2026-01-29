[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIUtilityComponentClass : SCR_AIBaseUtilityComponentClass
{	
}

class SCR_AIUtilityComponent : SCR_AIBaseUtilityComponent
{
	GenericEntity m_OwnerEntity;
	protected SCR_CharacterControllerComponent m_OwnerController;
	protected FactionAffiliationComponent m_FactionComponent;
	SCR_AIConfigComponent m_ConfigComponent;
	SCR_AIInfoComponent m_AIInfo;
	SCR_AICombatComponent m_CombatComponent;
	PerceptionComponent m_PerceptionComponent;
	SCR_MailboxComponent m_Mailbox;
	SCR_AICharacterSettingsComponent m_SettingsComponent;
	
	ref SCR_AIThreatSystem m_ThreatSystem;
	ref SCR_AISectorThreatFilter m_SectorThreatFilter;
	ref SCR_AILookAction m_LookAction;
	ref SCR_AICommsHandler m_CommsHandler;
	ref SCR_AIBehaviorBase m_CurrentBehavior; //!< Used for avoiding constant casting, outside of this class use GetCurrentBehavior()
	ref SCR_AICombatMoveState m_CombatMoveState;
	ref SCR_AIMovementDetector m_MovementDetector; // Used by GetSubformationLeaderMoving()
	
	// CallQueue of this AI. It gets updated from EvaluateBehavior, so that it's synchronous with other AI logic.
	protected ref ScriptCallQueue m_Callqueue;
	
	protected ref BaseTarget m_UnknownTarget;
	protected float m_fReactionUnknownTargetTime_ms; //!< WorldTime timestamp

	protected float m_fLastUpdateTime;
	
	protected static const float DISTANCE_HYSTERESIS_FACTOR = 0.45; 	//!< how bigger must be old distance to new in IsInvestigationRelevant()
	protected static const float NEARBY_DISTANCE_SQ = 2500; 			//!< what is the minimal distance of new vs old in IsInvestigationRelevant()
	protected static const float REACTION_TO_SAME_UNKNOWN_TARGET_INTERVAL_MS = 2500; //!< how often to react to same unknown target if it didn't change
		
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] unknownTarget
	//! \return
	SCR_AIBehaviorBase EvaluateBehavior(BaseTarget unknownTarget)
	{
		if (!m_OwnerController || !m_ConfigComponent || !m_OwnerEntity)
			return null;
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior START");
		if (m_bEvaluationBreakpoint)
		{
			Print("EvaluateBehavior breakpoint triggered", LogLevel.NORMAL);
			debug;
			m_bEvaluationBreakpoint = false;
		}
		#endif
		
		// Update delta time and players's position
		float time = m_OwnerEntity.GetWorld().GetWorldTime();
		float deltaTime = time - m_fLastUpdateTime;
		m_fLastUpdateTime = time;

		// Update call queue.
		// It must be updated before evaluation of behaviors.
		m_Callqueue.Tick(0.001 * deltaTime);
		
		// Create events from commands, danger events, new targets
		m_ThreatSystem.Update(this, deltaTime);
		m_SectorThreatFilter.Update(0.001 * deltaTime);
		m_CombatComponent.UpdatePerceptionFactor(m_PerceptionComponent, m_ThreatSystem);

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
					
					// Try to notify actions about the message
					bool overrideReaction = CallActionsOnMessage(msgInfo);
		
					#ifdef AI_DEBUG
					if (overrideReaction)
					{
						AddDebugMessage(string.Format("InfoMessage consumed by action: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
					}
					#endif
					
					// If message was not consumed by action, process it
					if (!overrideReaction)
					{
						#ifdef AI_DEBUG
						AddDebugMessage(string.Format("PerformInfoReaction: %1, from BT: %2", msgInfo, msgInfo.m_sSentFromBt));
						#endif
						m_ConfigComponent.PerformInfoReaction(this, msgInfo);
					}
				}
			}
		}
		
		bool reactToUnknownTarget = false;
		if (unknownTarget)
		{
			if (unknownTarget == m_UnknownTarget)
			{	// Same target
				if (GetGame().GetWorld().GetWorldTime() - m_fReactionUnknownTargetTime_ms > REACTION_TO_SAME_UNKNOWN_TARGET_INTERVAL_MS)
					reactToUnknownTarget = true;
			}
			else
			{	// Different target
				reactToUnknownTarget = true;
			}
		}
		if (reactToUnknownTarget && m_ConfigComponent.m_Reaction_UnknownTarget)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformReaction: Unknown Target: %1", unknownTarget));
			#endif
			
			m_ConfigComponent.m_Reaction_UnknownTarget.PerformReaction(this, m_ThreatSystem, unknownTarget, unknownTarget.GetLastSeenPosition());
			m_fReactionUnknownTargetTime_ms = GetGame().GetWorld().GetWorldTime();
		}
		m_UnknownTarget = unknownTarget;
		
		//------------------------------------------------------------------------------------------------
		// Evaluate current weapon and target
		
		bool weaponEvent;
		bool selectedTargetChanged;
		bool retreatTargetChanged;
		bool compartmentChanged;
		BaseTarget prevTarget;
		BaseTarget selectedTarget;
		m_CombatComponent.EvaluateWeaponAndTarget(weaponEvent, selectedTargetChanged,
			prevTarget, selectedTarget, retreatTargetChanged, compartmentChanged);
		
		if (selectedTargetChanged && m_ConfigComponent.m_Reaction_SelectedTargetChanged)
		{
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("PerformReaction: Selected Target Changed: %1", selectedTarget));
			#endif
			
			m_ConfigComponent.m_Reaction_SelectedTargetChanged.PerformReaction(this, prevTarget, selectedTarget);
		}
		
		BaseTarget retreatTarget = m_CombatComponent.GetRetreatTarget();
		if (retreatTarget &&
			(compartmentChanged ||
			(selectedTargetChanged && !selectedTarget && retreatTarget) || // Nothing to attack any more and must retreat from some target
			(!selectedTarget && retreatTargetChanged))) // Not attacking anything and must retreat from a different target
		{
			if (m_ConfigComponent.m_Reaction_RetreatFromTarget)
			{
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformReaction: Retreat From Target: %1", retreatTarget));
				#endif
				
				m_ConfigComponent.m_Reaction_RetreatFromTarget.PerformReaction(this, m_ThreatSystem, retreatTarget, retreatTarget.GetLastSeenPosition());
			}
		}
		
		//------------------------------------------------------------------------------------------------
		// Update combat component
		m_CombatComponent.Update(deltaTime);
		
		// Evaluation: Remove completed behaviors, evaluate, set new behavior
		RemoveObsoleteActions();
		AIActionBase selectedAction = EvaluateActions();
		#ifdef AI_DEBUG
		DiagIncreaseCounter();
		DebugLogActionsPriority();
		#endif
		
		if (selectedAction && selectedAction != m_CurrentBehavior && (!m_CurrentBehavior || m_CurrentBehavior.IsActionInterruptable()))
		{
			SetCurrentAction(selectedAction);
			m_CurrentBehavior = SCR_AIBehaviorBase.Cast(selectedAction);
#ifdef WORKBENCH
			SCR_AIDebugVisualization.VisualizeMessage(m_OwnerEntity, SCR_AIDebug.GetBehaviorName(m_CurrentBehavior), EAIDebugCategory.BEHAVIOR, 5);
#endif
		}
		
		m_CurrentBehavior.OnActionExecuted();
		
		// Update comms handler
		if (m_CommsHandler.m_bNeedUpdate)
			m_CommsHandler.Update(deltaTime);
		
		// Update combat move state
		if (m_CombatMoveState.m_bInCover && m_CombatMoveState.GetAssignedCover())
			m_CombatMoveState.VerifyCurrentCover(m_OwnerEntity.GetOrigin());
		
		#ifdef AI_DEBUG
		AddDebugMessage("EvaluateBehavior END\n");
		#endif
		
		return m_CurrentBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if we should be mindful about our formation overall
	bool ShouldKeepFormation()
	{
		return HasActionOfType(SCR_AIMoveInFormationBehavior);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns state of m_MovementDetector of our subformation leader.
	//! See SCR_AIMovementFilter class.
	bool GetSubformationLeaderMoving()
	{
		SCR_ChimeraAIAgent leaderAgent = SCR_ChimeraAIAgent.Cast(GetSubformationLeaderAgent());
		
		if (!leaderAgent)
			return false;
		
		return leaderAgent.m_UtilityComponent.m_MovementDetector.GetMoving();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetNearSubformationLeader()
	{
		vector myPos = m_OwnerEntity.GetOrigin();
		AIAgent subformationLeaderAgent = GetSubformationLeaderAgent();
		
		if (!subformationLeaderAgent)
			return false;
		
		IEntity subformationLeaderEntity = subformationLeaderAgent.GetControlledEntity();
		
		if (!subformationLeaderEntity)
			return false;
		
		vector leaderPos = subformationLeaderEntity.GetOrigin();
		float distance = vector.DistanceSq(myPos, leaderPos);
		return distance < 20*20;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns leader agent of subformation of this agent
	protected AIAgent GetSubformationLeaderAgent()
	{
		AIAgent myAgent = GetAIAgent();
		SCR_AIGroup myGroup = SCR_AIGroup.Cast(myAgent.GetParentGroup());
		
		// No group - no leader agent
		if (!myGroup)
			return null;
		
		// Check our subformation
		AIGroupMovementComponent movementComp = AIGroupMovementComponent.Cast(myGroup.GetMovementComponent());
		int moveHandlerId = movementComp.GetAgentMoveHandlerId(myAgent);
		bool formationDisplaced = movementComp.IsFormationDisplaced(moveHandlerId);
		
		SCR_AIGroup masterGroup = myGroup.GetMaster();
		
		if (formationDisplaced && masterGroup)
		{
			// This is only possible for player-led formation
			
			int leaderId = masterGroup.GetLeaderID();
			IEntity leaderEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(leaderId);
			if (!leaderEntity)
					return null;
			
			AIControlComponent controlComp = AIControlComponent.Cast(leaderEntity.FindComponent(AIControlComponent));
			
			if (!controlComp)
				return null;
			
			return controlComp.GetAIAgent();
		}
		
		// Formation is not displaced - get our group's leader
		// Either moving as slave group or as master group, but it doesn't matter, either way get our leader
		return myGroup.GetLeaderAgent();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns true if this agent is leader of its subformation
	bool IsSubformationLeader()
	{
		AIAgent myAgent = GetAIAgent();
		SCR_AIGroup myGroup = SCR_AIGroup.Cast(myAgent.GetParentGroup());

		if (!myGroup)
			return false;
		
		AIGroupMovementComponent movementComp = AIGroupMovementComponent.Cast(myGroup.GetMovementComponent());
		
		if (!movementComp)
			return false;
		
		int moveHandlerId = movementComp.GetAgentMoveHandlerId(myAgent);
		
		bool formationDisplaced = movementComp.IsFormationDisplaced(moveHandlerId);
		
		// If formation is displaced, then we can't be leader of it
		if (formationDisplaced)
			return false;
		
		// Formation is not displaced - find our index in formation
		array<AIAgent> agentsInHandler = {};
		movementComp.GetAgentsInHandler(agentsInHandler, moveHandlerId);
		int myIndexInHandler = agentsInHandler.Find(myAgent);
		
		return myIndexInHandler == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Independent attacking with movement is allowed only when group combat move is not present or unit state is EUnitState.IN_TURRET
	//!
	//! \return
	bool CanIndependentlyMove()
	{
		if (m_AIInfo && m_AIInfo.HasUnitState(EUnitState.IN_TURRET))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_AIBehaviorBase GetCurrentBehavior()
	{
		return m_CurrentBehavior;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_CharacterControllerComponent GetCharacterController()
	{
		return m_OwnerController;
	}
	
	//------------------------------------------------------------------------------------------------
	// when agent leaves a group, all former group-induced behaviors should fail
	void CancelAllGroupActivityBehaviors(notnull SCR_AIGroupUtilityComponent groupUtility)
	{
		array<ref AIActionBase> actions = {};
		GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			if (!action)
				continue;
			SCR_AIActivityBase relatedGroupActivity = SCR_AIActivityBase.Cast(action.GetRelatedGroupActivity());
			if (relatedGroupActivity && relatedGroupActivity.m_Utility == groupUtility)
				action.Fail();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] action
	void WrapBehaviorOutsideOfVehicle(SCR_AIActionBase action)
	{
		
	 	CompartmentAccessComponent compartmentAccess = CompartmentAccessComponent.Cast(m_OwnerEntity.FindComponent(CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
			 
		if (!compartmentAccess.IsInCompartment())
			return;	
		
		SCR_AIActivityBase relatedActivity;
		SCR_AIBehaviorBase behavior = SCR_AIBehaviorBase.Cast(action);
		if (behavior)
			relatedActivity = SCR_AIActivityBase.Cast(behavior.GetRelatedGroupActivity());
		
		float priority = action.GetPriority();
		float priorityLevel = action.EvaluatePriorityLevel();
		IEntity vehicle = compartmentAccess.GetCompartment().GetOwner();
		ECompartmentType compartmentType = SCR_AICompartmentHandling.CompartmentClassToType(compartmentAccess.GetCompartment().Type());
		AddAction(new SCR_AIGetOutVehicle(this, relatedActivity, vehicle, priority: priority + 100, priorityLevel: priorityLevel));
		AddAction(new SCR_AIGetInVehicle(this, relatedActivity, vehicle, compartmentAccess.GetCompartment(), roleInVehicle: compartmentType, priority: priority, priorityLevel: priorityLevel));
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		AIAgent agent = GetOwner();
		if (!agent)
			return;	
		
		m_ConfigComponent = SCR_AIConfigComponent.Cast(agent.FindComponent(SCR_AIConfigComponent));

		m_OwnerEntity = GenericEntity.Cast(agent.GetControlledEntity());
		if (!m_OwnerEntity)
			return;

		m_Callqueue = new ScriptCallQueue();
		m_AIInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
		m_CombatComponent = SCR_AICombatComponent.Cast(m_OwnerEntity.FindComponent(SCR_AICombatComponent));
		m_PerceptionComponent = PerceptionComponent.Cast(m_OwnerEntity.FindComponent(PerceptionComponent));
		m_OwnerController = SCR_CharacterControllerComponent.Cast(m_OwnerEntity.FindComponent(SCR_CharacterControllerComponent));
		m_FactionComponent = FactionAffiliationComponent.Cast(m_OwnerEntity.FindComponent(FactionAffiliationComponent));
		m_ThreatSystem = new SCR_AIThreatSystem(this);
		m_SectorThreatFilter = new SCR_AISectorThreatFilter(m_OwnerEntity);
		m_AIInfo.InitThreatSystem(m_ThreatSystem); // let the AIInfo know about the threat system - move along with creating threat system instance!
		m_LookAction = new SCR_AILookAction(this, false); // LookAction is not regular behavior and is evaluated separately
		m_Mailbox = SCR_MailboxComponent.Cast(owner.FindComponent(SCR_MailboxComponent));
		m_SettingsComponent = SCR_AICharacterSettingsComponent.Cast(owner.FindComponent(SCR_AICharacterSettingsComponent));
		m_CommsHandler = new SCR_AICommsHandler(m_OwnerEntity, agent);
		m_CombatMoveState = new SCR_AICombatMoveState();
		m_MovementDetector = new SCR_AIMovementDetector(m_OwnerEntity);
		
		// Subscribe to events
		m_AIInfo.m_OnCompartmentEntered.Insert(OnCompartmentEntered);
		m_AIInfo.m_OnCompartmentLeft.Insert(OnCompartmentLeft);
		
		// Add default behaviors after the rest is initialized
		m_ConfigComponent.AddDefaultBehaviors(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCompartmentEntered (AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot slot = manager.FindCompartment(slotID, mgrID);
		if (PilotCompartmentSlot.Cast(slot))
		{
			IEntity vehicle = slot.GetVehicle();
			if (!vehicle)
				return;
			auto behavior = new SCR_AIIdleBehavior_Driver(this, null, vehicle);
			AddAction(behavior);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCompartmentLeft (AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot slot = manager.FindCompartment(slotID, mgrID);
		if (PilotCompartmentSlot.Cast(slot))
		{
			auto behavior = FindActionOfType(SCR_AIIdleBehavior_Driver);
			behavior.Fail();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Are we a military AI or not?
	bool IsMilitary()
	{
		SCR_Faction f = SCR_Faction.Cast(m_FactionComponent.GetAffiliatedFaction());
		if (!f)
			return false;
		return f.IsMilitary();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		m_CommsHandler.EOnDiag(timeSlice);
		m_SectorThreatFilter.EOnDiag(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetOrigin()
	{
		return m_OwnerEntity.GetOrigin();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns CallQueue of this AI. It gets updated from EvaluateBehavior, so that it's synchronous with other AI logic.
	ScriptCallQueue GetCallqueue()
	{
		return m_Callqueue;
	}
	
	//------------------------------------------------------------------------------------------------
	void LookAt(vector pos,float duration = 2)
	{
		m_LookAction.LookAt(pos, SCR_AILookAction.PRIO_COMMANDER, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AIUtilityComponent()
	{
		if (m_AIInfo)
		{
			m_AIInfo.m_OnCompartmentEntered.Remove(OnCompartmentEntered);
			m_AIInfo.m_OnCompartmentLeft.Remove(OnCompartmentLeft);
		}
	}
}