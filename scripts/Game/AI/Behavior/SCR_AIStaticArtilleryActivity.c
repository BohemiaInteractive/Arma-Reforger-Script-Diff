class SCR_AIStaticArtilleryActivity : SCR_AIActivityBase
{
	// Requested parameters
	protected vector m_vTargetPos;
	protected SCR_EAIArtilleryAmmoType m_eAmmoType;
	protected int m_iTargetShotCount;
	
	// Runtime data
	protected ref array<AIAgent> m_aAssignedAgents = {};// Agents to which we sent messages
	protected ref array<AIAgent> m_aFailedAgents = {};	// Agents which reported their failing of behavior
	protected int m_iActualShotCount;
	
	//------------------------------------------------------------------------------------------------------------------------
	//! targetPos - position where artillery shells must land.
	//! targetShotCount - total amount of shots which must be fired. If it's -1, artillery will be fired infinitely. 
	void SCR_AIStaticArtilleryActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector targetPos, SCR_EAIArtilleryAmmoType ammoType, int targetShotCount, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_iTargetShotCount = targetShotCount;
		m_eAmmoType = ammoType;
		m_vTargetPos = targetPos;
		SetPriority(PRIORITY_ACTIVITY_ARTILLERY_SUPPORT);
		m_fPriorityLevel.m_Value = priorityLevel;
		SetIsUniqueInActionQueue(false); // Otherwise it will fail if a second one is added, which causes waypoint to be completed, which we don't want
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		// Assign agents to turrets, send them messages
		
		array<ref SCR_AIGroupVehicle> allVehicles = {};		// All vehicles from vehicle mgr
		array<ref SCR_AIGroupVehicle> usableTurrets = {}; 	// All artillery turrets which we will consider
		array<AIAgent> freeAgents = {};						// Agents free for assignment to artillery
		array<AIAgent> assignedAgents = {}; 				// Agent for that turret in usableTurrets array
		
		m_Utility.m_VehicleMgr.GetAllVehicles(allVehicles);
		m_Utility.m_Owner.GetAgents(freeAgents);
		
		foreach (SCR_AIGroupVehicle v : allVehicles)
		{
			SCR_AIVehicleUsageComponent vehicleUsageComp = v.GetVehicleUsageComponent();
			
			if (!vehicleUsageComp)
				continue;
			
			if (vehicleUsageComp.GetVehicleType() != EAIVehicleType.STATIC_ARTILLERY)
				continue;
			
			// Check if it's occupied by someone not in our group
			TurretCompartmentSlot slot = vehicleUsageComp.GetTurretCompartmentSlot();
			if (!slot)
				continue;
			
			IEntity occupant = slot.GetOccupant();
			if (occupant)
			{
				AIAgent occupantAgent = SCR_AIUtils.GetAIAgent(occupant);
				if (!occupantAgent)
					continue; // No agent, ignore
				else if (occupantAgent.GetParentGroup() != m_Utility.m_Owner)
					continue; // Agent not in this group, ignore
				
				// Agent is in this group, assign to this turret
				usableTurrets.Insert(v);
				assignedAgents.Insert(occupantAgent);
				freeAgents.RemoveItem(occupantAgent);
			}
			else
			{
				// Noone inside, put it in usableTurrets array, assign operator later
				usableTurrets.Insert(v);
				assignedAgents.Insert(null);
			}
		}
		
		// Assign remaining agents and turrets
		for (int i = 0; i < usableTurrets.Count(); i++)
		{
			// Continue if already assigned
			if (assignedAgents[i])
				continue;
			
			// Break if no more free agents
			if (freeAgents.IsEmpty())
				break;
			
			int bestAgentId = -1;
			AIAgent bestAgent = FindBestAgentForArtillery(freeAgents, usableTurrets[i].GetEntity(), bestAgentId);
			
			if (!bestAgent)
				break;
			
			assignedAgents[i] = bestAgent;
			freeAgents.Remove(bestAgentId);
		}
		
		// Send goal messages
		SendMessages(usableTurrets, assignedAgents);
		
		// Store the list of involved agents
		m_aAssignedAgents.Clear();
		foreach (auto a : assignedAgents)
		{
			if (a)
				m_aAssignedAgents.Insert(a);
		}
		m_aFailedAgents.Clear();
	}
	
	//------------------------------------------------------------------------------------
	//! Selects best agent to operate artillery entity. Now selects nearest agent.
	protected AIAgent FindBestAgentForArtillery(notnull array<AIAgent> agents, notnull IEntity artilleryEntity, out int outIndex)
	{
		float minDistSq = float.MAX;
		AIAgent bestAgent = null;
		int bestAgentIndex = -1;
		vector artilleryPos = artilleryEntity.GetOrigin();
		
		foreach (int i, AIAgent agent : agents)
		{
			IEntity controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity)
				continue;
			vector controlledEntityPos = controlledEntity.GetOrigin();
			float distSq = vector.DistanceSq(controlledEntityPos, artilleryPos);
			if (distSq < minDistSq)
			{
				bestAgent = agent;
				bestAgentIndex = i;
				minDistSq = distSq;
			}
		}
		
		outIndex = bestAgentIndex;
		return bestAgent;
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
		
		// If this is failed, also fail the waypoint
		if (m_RelatedWaypoint)
			m_Utility.m_Owner.CompleteWaypoint(m_RelatedWaypoint);
	}	
	
	//------------------------------------------------------------------------------------------------------------------------
	protected void SendMessages(notnull array<ref SCR_AIGroupVehicle> turrets, notnull array<AIAgent> assignedAgents)
	{
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!myComms)
			return;
		
		for (int i = 0; i < turrets.Count(); i++)
		{
			SCR_AIGroupVehicle turret = turrets[i];
			AIAgent agent = assignedAgents[i];
			
			if (!agent) // It's possible that noone was assigned to this turret
				continue;
			
			SCR_AIMessage_ArtillerySupport msg = new SCR_AIMessage_ArtillerySupport();
			msg.m_RelatedGroupActivity = this;
			msg.m_fPriorityLevel = m_fPriorityLevel.m_Value;
			msg.m_ArtilleryEntity = turret.GetEntity();
			msg.m_vTargetPos = m_vTargetPos;
			msg.m_eAmmoType = m_eAmmoType;
			msg.SetReceiver(agent);
			
			myComms.RequestBroadcast(msg, agent);
		}
	} 
	
	//------------------------------------------------------------------------------------------------------------------------
	override void OnChildBehaviorCreated(SCR_AIBehaviorBase childBehavior)
	{
		// Subscribe to events of the child behavior
		
		SCR_AIStaticArtilleryBehavior artilleryBehavior = SCR_AIStaticArtilleryBehavior.Cast(childBehavior);
		
		if (!artilleryBehavior)
			return;
		
		artilleryBehavior.GetOnArtilleryFired().Insert(OnChildBehaviorArtilleryFired);
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	//! Called by agent when he failed his behavior related to this activity
	override void OnChildBehaviorFinished(SCR_AIBehaviorBase childBehavior)
	{
		// Ignore if this activity is already over
		if (GetActionState() != EAIActionState.RUNNING)
			return;
		
		// Ignore if the child behavior is not failed
		if (childBehavior.GetActionState() != EAIActionState.FAILED)
			return;
		
		// Ignore if it was cancelled by this group, there is no reason to fail this activity in this case.
		// For instance the behavior could be cancelled because the group temporary selected another activity.
		int failReason = childBehavior.GetFailReason();
		if (failReason == EAIActionFailReason.CANCELLED)
			return;
		
		AIAgent agent = childBehavior.m_Utility.GetAIAgent();
		m_aFailedAgents.Insert(agent);
		
		// If m_aFailedAgents and m_aAssignedAgents are same, fail
		int nFailed = 0;
		int nValidAssigned = 0;
		foreach (AIAgent a : m_aAssignedAgents)
		{
			if (!a)
				continue;
			
			nValidAssigned++;
			
			if (m_aFailedAgents.Contains(a))
				nFailed++;
		}
		
		if (nValidAssigned == nFailed)
		{
			// All assigned agents have failed their behaviors
			Fail();
			Print(string.Format("SCR_AIStaticArtilleryActivity: FAILED, noone can shoot at the target position: %1, or there is no ammo", m_vTargetPos), LogLevel.DEBUG);
		}
	}
	
	//------------------------------------------------------------------------------------------------------------------------
	protected void OnChildBehaviorArtilleryFired(SCR_AIStaticArtilleryBehavior behavior, AIAgent agent)
	{
		m_iActualShotCount++;
		
		if (m_iTargetShotCount > 0 && m_iActualShotCount >= m_iTargetShotCount)
		{
			// We're done with fire mission, complete the activity, and complete the waypoint
			Complete();
			if (m_RelatedWaypoint)
				m_Utility.m_Owner.CompleteWaypoint(m_RelatedWaypoint);
		}
	}
}