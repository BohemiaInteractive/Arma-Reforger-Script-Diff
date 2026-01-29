//! Activity which handles combat of a group if it has vehicles
class SCR_AIVehicleCombatActivity : SCR_AIActivityBase
{
	static const string ACTIVITY_NAME = "SCR_AIVehicleCombatActivity";
	
	//------------------------------------------------------------------------------------
	void SCR_AIVehicleCombatActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint)
	{
		SetPriority(PRIORITY_ACTIVITY_COMBAT_WITH_VEHICLES);
	}
	
	//------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		// Do we have vehicles?
		if (m_Utility.m_VehicleMgr.GetVehiclesCount() == 0)
			return 0;
		
		// Does combat mode forbit combat?
		if (m_Utility.GetCombatModeActual() == EAIGroupCombatMode.HOLD_FIRE)
			return 0;
		
		// Does something threaten us too much?
		SCR_AIGroupTargetCluster c = m_Utility.m_Perception.m_MostDangerousCluster;
		if (!c)
			return 0;
		
		// Are we not a slave group?
		if (m_Utility.m_Owner.IsSlave())
			return 0;
		
		// Do we have at least some non-static vehicles?
		int countNonStatic = 0;
		array<ref SCR_AIGroupVehicle> allVehicles = {};
		m_Utility.m_VehicleMgr.GetAllVehicles(allVehicles);
		foreach (SCR_AIGroupVehicle v : allVehicles)
		{
			if (!v.IsStatic())
				countNonStatic++;
		}
		
		if (countNonStatic == 0)
			return 0;
		
		// Is the target cluster dangerous enough?
		if (!IsClusterDangerousEnough(c))
			return 0;
		
		return GetPriority();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		array<ref SCR_AIGroupVehicle> vehicles = {};
		m_Utility.m_VehicleMgr.GetAllVehicles(vehicles);
		m_Utility.m_OnAgentLifeStateChanged.Insert(OnAgentLifeStateChanged);
		
		foreach (SCR_AIGroupVehicle v : vehicles)
		{
			VehicleUsageLogic(v);
		}
	}
	
	//------------------------------------------------------------------------------------
	// Checks if vehicle gunner should take over the driver seat and retreat
	void GunnerRetreatLogic(AIAgent agent, IEntity vehicle)
	{	
		// !!!
		// Temporarly disabled this until retreat mechanic is available
		// !!!
		return;
		
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
		if (!chimeraAgent || !vehicle)
			return;
		
		if (ShouldGunnerRetreatInVehicle(chimeraAgent, vehicle))
		{
			AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
			if (!myComms)
				return;
			
			// Send message to take over driver seat
			SCR_AIMessageHandling.SendGetInMessage(agent, vehicle, EAICompartmentType.Pilot, this, myComms, ACTIVITY_NAME);
		}
	}
	
	//------------------------------------------------------------------------------------
	// Checks if vehicle gunner should take over the driver seat and retreat
	bool ShouldGunnerRetreatInVehicle(SCR_ChimeraAIAgent chimeraAgent, IEntity vehicleEntity)
	{			
		vector dangerPos;
		float dangerDistance;
		
		BaseTarget target = chimeraAgent.m_UtilityComponent.m_CombatComponent.GetCurrentTarget();
		if (!target)
			target = chimeraAgent.m_UtilityComponent.m_CombatComponent.GetLastSeenEnemy();
		
		SCR_AIGroupTargetCluster cluster = m_Utility.m_Perception.m_MostDangerousCluster;
		
		/*
			TODO: Consider firepower difference and threat/suppression
			cluster.m_State.m_iCountIdentified + cluster.m_State.m_iCountDetected
		*/
		
		if (target)
		{
			dangerPos = target.GetLastSeenPosition();
			dangerDistance = target.GetDistance();
		}
		else
		{
			// No targets & clusters, can safely retreat
			if (!cluster)
				return true;
				
			dangerPos = cluster.m_State.GetCenterPosition();
			dangerDistance = cluster.GetMinDistance();
		}
		
		// If enemy is far, don't sweat, can retreat
		if (dangerDistance > 300)
			return true;
		
		// If enemy is upclose, don't retreat (agent will either shoot target or dismount and engage)
		if (dangerDistance < 40)
			return false;		
		
		/*
		To consider: vehicle movement and gunner rotation capability
		
		vector mat[3];
		vehicleEntity.GetTransform(mat);
		vector vehicleDir = mat[2];
		
		vector dirToTarget = vector.Direction(vehicleEntity.GetOrigin(), dangerPos).Normalized();
		float dangerDot = Math.AbsFloat(vector.Dot(vehicleDir, dirToTarget));	
		
		if (dangerDot < 0.6)
			return true;
		*/
		
		return false;	
	}
	
	//------------------------------------------------------------------------------------
	void VehicleUsageLogic(SCR_AIGroupVehicle vehicle)
	{
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
		SCR_AIVehicleUsageComponent vehicleComp = vehicle.GetVehicleUsageComponent();
		IEntity vehicleEntity = vehicle.GetEntity();
		
		if (!vehicleEntity || !vehicleComp || !myComms)
			return;
		
		array<ref SCR_AIGroupFireteam> crewFireteams = {};
		array<ref SCR_AIGroupFireteam> cargoFireteams = {};
		m_Utility.m_FireteamMgr.FindFireteamsOfVehicle(crewFireteams, vehicleComp, SCR_AIGroupFireteamVehicleCrew);
		m_Utility.m_FireteamMgr.FindFireteamsOfVehicle(cargoFireteams, vehicleComp, SCR_AIGroupFireteamVehicleCargo);
		
		int soldierCounter = 0;
		
		array<AIAgent> ftAgents = {};
		if (!vehicle.HasWeapon())
		{
			foreach (SCR_AIGroupFireteam ft : crewFireteams)
			{
				ft.GetMembers(ftAgents);
				foreach (AIAgent agent : ftAgents)
				{
					SCR_AIMessageHandling.SendDismountMessage(agent, vehicleEntity, soldierCounter, this, myComms, ACTIVITY_NAME);
					ft.RemoveMember(agent);
					soldierCounter++;
				}
			}
		}
		else // since we have a vehicle with weapon, we listen on driver's lifeStateChange and react on it
		{
			foreach (SCR_AIGroupFireteam ft : crewFireteams)
			{
				ft.GetMembers(ftAgents);
				foreach (AIAgent agent : ftAgents)
				{
					if (!vehicleComp) 
						break;
					if  (!vehicleComp.CanBePiloted())
						break;
					if (!agent)
						continue;
					SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
					if (!chimeraAgent)
						continue;
					SCR_AIInfoComponent agentInfo = chimeraAgent.m_InfoComponent;
					if (!agentInfo)
						continue;
					// actual logic
					if (agentInfo.HasUnitState(EUnitState.PILOT) && (agentInfo.HasUnitState(EUnitState.WOUNDED) || agentInfo.HasUnitState(EUnitState.UNCONSCIOUS)))
					{
						// driver is incap when attack starts - find someone available and use him as new driver
						AIAgent newDriver;
						SCR_AIGroupFireteam newDriversFireTeam;
						if (GetAvailableAgent(cargoFireteams, newDriver, newDriversFireTeam))
						{
							newDriversFireTeam.RemoveMember(newDriver);
							SCR_AIMessageHandling.SendGetInMessage(newDriver, vehicleEntity, EAICompartmentType.Pilot, this, myComms, ACTIVITY_NAME);
						};
					}
					else if (agentInfo.HasUnitState(EUnitState.IN_TURRET) && ftAgents.Count() == 1)
					{
						// gunner is the only one in the crew fireteam
						AIAgent newDriver;
						SCR_AIGroupFireteam newDriversFireTeam;
						if (GetAvailableAgent(cargoFireteams, newDriver, newDriversFireTeam))
						{
							newDriversFireTeam.RemoveMember(newDriver);
							SCR_AIMessageHandling.SendGetInMessage(newDriver, vehicleEntity, EAICompartmentType.Pilot, this, myComms, ACTIVITY_NAME);
						}
						else
							GunnerRetreatLogic(agent, vehicleEntity);
					} 
				}
			}
		}
		
		foreach (SCR_AIGroupFireteam ft : cargoFireteams)
		{
			ft.GetMembers(ftAgents);
			foreach (AIAgent agent : ftAgents)
			{
				SCR_AIMessageHandling.SendDismountMessage(agent, vehicleEntity, soldierCounter, this, myComms, ACTIVITY_NAME);
				ft.RemoveMember(agent);
				soldierCounter++;
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	//! returns first non bleeding conscious agent we have inside given array of fireteams
	//! \param[out] availableAgent - the agent that is not wounded or unconscious
	//! \param[out] agentsFireTeam - the fireteam of the agent
	bool GetAvailableAgent(array<ref SCR_AIGroupFireteam> fireteams, out AIAgent availableAgent, out SCR_AIGroupFireteam agentsFireTeam)
	{
		array<AIAgent> ftAgents = {};
		SCR_AIInfoComponent agentInfo;
		foreach (SCR_AIGroupFireteam ft : fireteams)
		{
			ft.GetMembers(ftAgents);
			foreach (AIAgent agent : ftAgents)
			{
				agentInfo = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
				if (!agentInfo.HasUnitState(EUnitState.WOUNDED) && !agentInfo.HasUnitState(EUnitState.UNCONSCIOUS))
				{
					availableAgent = agent;
					agentsFireTeam = ft;
					return true;
				}	
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------
	protected bool IsClusterDangerousEnough(notnull SCR_AIGroupTargetCluster c)
	{
		// Has it not been seen for too long?
		if (c.m_State.GetTimeSinceLastNewInformation() > 20.0)
			return false;
		
		// We could also check if we have a waypoint or not
		
		// Is it too far?
		float distThreshold;
		if (c.m_State.m_iCountEndangering > 0)
			distThreshold = 550.0;
		else
			distThreshold = 200.0;
		
		if (c.m_State.m_fDistMin > distThreshold)
			return false;
		
		return true;
	}
	
	//-----------------------------------------------------------------------------------------
	//! Reacts on incapacitated (or dead) driver agent
	//! it sends messages to other crew members to get in the driver seat,
	//! if that driver is gunner, finds new gunner outside the vehicle and moves towards him
	void OnAgentLifeStateChanged(AIAgent incapacitatedAgent, SCR_AIInfoComponent infoIncap, IEntity vehicle, ECharacterLifeState lifeState)
	{
		if (!incapacitatedAgent || !vehicle || lifeState == ECharacterLifeState.ALIVE || !infoIncap.HasUnitState(EUnitState.PILOT))
			return;
		SCR_AIGroupFireteam fireTeam = m_Utility.m_FireteamMgr.FindFireteam(incapacitatedAgent);
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!fireTeam || !myComms)
			return;
		SCR_AIInfoComponent info;
		array<AIAgent> agentsOfGroup = {};
		m_Utility.m_Owner.GetAgents(agentsOfGroup);
		int agentsNum = agentsOfGroup.Count();
		if (agentsNum > 1)
		{
			int memberCount = fireTeam.GetMemberCount();
			if (memberCount > 1) // we have some agent left in the same fireteam -> we use him
			{
				// replacing driver with either gunner or somebody else
				array<AIAgent> agentsOfFireteam = {};
				fireTeam.GetMembers(agentsOfFireteam);
				AIAgent newDriverAgent, newGunnerAgent;
				bool newDriverIsOldGunner = false;
				foreach (AIAgent agent : agentsOfFireteam)
				{
					if (agent != incapacitatedAgent && memberCount == 2) // there is only incap driver and gunner
					{
						newDriverAgent = agent;
						newDriverIsOldGunner = true;
						break;
					}
					else if (agent != incapacitatedAgent) // there are more ft members -> we skip the gunner
					{
						info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
						if (!info || info.HasUnitState(EUnitState.WOUNDED) || info.HasUnitState(EUnitState.UNCONSCIOUS) || info.HasUnitState(EUnitState.IN_TURRET))
							continue;
						newDriverAgent = agent;
						break;
					}
				}
				// possibly replacing gunner if he is the new driver
				if (newDriverIsOldGunner && agentsNum > 2) // we have available gunner outside of our fireteam!
				{
					foreach (AIAgent agent : agentsOfGroup)
					{
						if (!agent || agent == incapacitatedAgent || agent == newDriverAgent)
							continue;
						info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
						if (!info || info.HasUnitState(EUnitState.WOUNDED) || info.HasUnitState(EUnitState.UNCONSCIOUS))
							continue;
						newGunnerAgent = agent;
						break;
					}
				}
				
				if (newDriverAgent)
					GunnerRetreatLogic(newDriverAgent, vehicle);
				
				if (newGunnerAgent)
				{
					SCR_AIMessageHandling.SendGetInMessage(newGunnerAgent, vehicle, EAICompartmentType.Turret, this, myComms, ACTIVITY_NAME);
					SCR_AIMessageHandling.SendMoveMessage(newDriverAgent, newGunnerAgent.GetControlledEntity(), this, myComms, ACTIVITY_NAME);
				}
			}
		}
	}
	
	
	//-----------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		//cleanup
		m_Utility.m_OnAgentLifeStateChanged.Remove(OnAgentLifeStateChanged);
	}
}