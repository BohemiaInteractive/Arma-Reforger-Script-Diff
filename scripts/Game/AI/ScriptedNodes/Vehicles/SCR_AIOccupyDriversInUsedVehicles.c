class SCR_AIOccupyDriversInUsedVehicles : AITaskScripted
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Test vehicles that are not registered inside the group?")]
	protected bool m_bTestNewVehicles;
	
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.EditBox, desc: "Limit search for replacement by this distance [m]?")]
	protected float m_fMaxDistanceOfSearch_m;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Update time of checks [ms]")]
	protected float m_fUpdateInterval_ms;
	
	static const string PORT_CAN_USE_VEHICLE = "CanUseVehicle";
	static const string NODE_NAME = "SCR_AIOccupyDriversInUsedVehicles";
	
	protected float m_fNextUpdate_ms;
	protected ref array<AIAgent> m_aAgents = {};
	protected ref array<AIAgent> m_aInformedAgents = {};
	protected ref array<ref SCR_AIGroupVehicle> m_aUsedVehicles = {};
	protected ref array<ref SCR_AIGroupVehicle> m_aVehiclesToOccupy = {};
	protected SCR_AIGroup m_Group;
	protected SCR_AIGroupUtilityComponent m_Utility;
	protected int m_iStateOfExecution;
	protected int m_iNumberOfUnpilotableVehicles;
	
	protected static int STATE_TESTING_STATE 	= 0; // testing initial setup
	protected static int STATE_SENDING_SIGNALS 	= 1; // sending signals to agents
	protected static int STATE_WAITING 			= 2; // waiting for agents to obey
	protected static int STATE_FINISHED 		= 3; // done everything
		
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_Group = SCR_AIGroup.Cast(owner);
		if (!m_Group) 
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return;
		}
		m_Utility = SCR_AIGroupUtilityComponent.Cast(m_Group.FindComponent(SCR_AIGroupUtilityComponent));
		if (!m_Utility)
			return;
		m_aAgents.Clear();
		m_aInformedAgents.Clear();
		m_aUsedVehicles.Clear();
		m_aVehiclesToOccupy.Clear();
		m_Utility.m_OnAgentLifeStateChanged.Remove(OnAgentLifeStateChanged);
		m_Utility.m_OnAgentLifeStateChanged.Insert(OnAgentLifeStateChanged);
		m_iStateOfExecution = STATE_TESTING_STATE;
		m_iNumberOfUnpilotableVehicles = 0;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_Group || !m_Utility)
			return ENodeResult.FAIL;
		
		float currentTime_ms = GetGame().GetWorld().GetWorldTime();
		if (currentTime_ms < m_fNextUpdate_ms)
			return ENodeResult.RUNNING;
		m_fNextUpdate_ms = currentTime_ms + m_fUpdateInterval_ms;
		
		switch (m_iStateOfExecution)
		{
			case STATE_TESTING_STATE:
			{
				return Testing_State(true);
			}
			case STATE_SENDING_SIGNALS:
			{
				return SendingMessages_State();
			}
			case STATE_WAITING:
			{
				return Waiting_State();
			}
			case STATE_FINISHED:
			{
				return Finished_State();
			}	
		}
		return ENodeResult.RUNNING;	
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected ENodeResult Testing_State(bool isScheduled)
	{
		m_Group.GetAgents(m_aAgents);
		if (isScheduled)
		{
			// check that AIAgents arent just moving in/out of compartments
			foreach (AIAgent agent: m_aAgents)
			{
				ChimeraCharacter chChar = ChimeraCharacter.Cast(agent.GetControlledEntity());
				if (!chChar)
					continue;
				CompartmentAccessComponent compAcc = chChar.GetCompartmentAccessComponent();
				if (!compAcc)
					continue;
				bool inTransition = compAcc.IsGettingIn() || compAcc.IsGettingOut();
				if (inTransition)
					return ENodeResult.RUNNING;
			}
			// check all vehicles are registered as usable
			if (m_bTestNewVehicles)
			{
				foreach(AIAgent ag: m_aAgents)
				{
					ChimeraCharacter char = ChimeraCharacter.Cast(ag.GetControlledEntity());
					if (!char || !char.IsInVehicle())
						continue;
					CompartmentAccessComponent acc = char.GetCompartmentAccessComponent();
					if (!acc)
						continue;
					IEntity vehicleEntity = acc.GetVehicleIn(char);
					SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicleEntity, vehicleEntity); 
					m_Utility.AddUsableVehicle(vehicleUsageComp);
				}
			}
		}
		m_aVehiclesToOccupy.Clear();
		m_Utility.m_VehicleMgr.GetAllVehicles(m_aUsedVehicles);
		
		foreach (SCR_AIGroupVehicle groupVehicle : m_aUsedVehicles)
		{
			if (!groupVehicle)
				continue;
			// we skip group helicopters and static turrets
			SCR_AIVehicleUsageComponent vehicleUsage = groupVehicle.GetVehicleUsageComponent();
			if (groupVehicle.IsStatic() || (vehicleUsage && !vehicleUsage.CanBePiloted()))
			{
				m_iNumberOfUnpilotableVehicles++;
				continue;
			}
			// add vehicles that dont have operational driver
			if (!groupVehicle.DriverIsConscious())
			{
				m_aVehiclesToOccupy.Insert(groupVehicle);
			}
		}
		bool willTryToReoccupy = m_aVehiclesToOccupy.Count() > 0;
		if (willTryToReoccupy)
			m_iStateOfExecution = STATE_SENDING_SIGNALS;
		else
			m_iStateOfExecution = STATE_FINISHED;
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected ENodeResult SendingMessages_State()
	{
		if (m_aVehiclesToOccupy.Count() == 0)
		{ 
			m_iStateOfExecution = STATE_FINISHED;
			return ENodeResult.RUNNING;
		};
		m_aInformedAgents.Clear();
		foreach (SCR_AIGroupVehicle groupVehicle : m_aVehiclesToOccupy)
		{
			AIAgent agentToUseAsDriver;
			int vehicleHandleId = groupVehicle.GetSubgroupHandleId();
			array<AIAgent> agentsOfHandler = {};
			// try to occupy with driver in the same subgroup
			m_Utility.m_GroupMovementComponent.GetAgentsInHandler(agentsOfHandler, vehicleHandleId);
			agentToUseAsDriver = GetAgentThatIsConscious(agentsOfHandler, m_aInformedAgents, excludeDistance: m_fMaxDistanceOfSearch_m, locationOfAccident: groupVehicle.GetEntity().GetOrigin());
			// try to occupy with driver subgroup on foot
			if (!agentToUseAsDriver) 
			{
				m_Utility.m_GroupMovementComponent.GetAgentsInHandler(agentsOfHandler, AIGroupMovementComponent.DEFAULT_HANDLER_ID);				
				agentToUseAsDriver = GetAgentThatIsConscious(agentsOfHandler, m_aInformedAgents, excludeDistance: m_fMaxDistanceOfSearch_m, locationOfAccident: groupVehicle.GetEntity().GetOrigin());
			}
			// try to occupy with any alive agent
			if (!agentToUseAsDriver)
			{
				agentToUseAsDriver = GetAgentThatIsConscious(m_aAgents, m_aInformedAgents, true, excludeDistance: m_fMaxDistanceOfSearch_m, locationOfAccident: groupVehicle.GetEntity().GetOrigin());
			}
			if (agentToUseAsDriver)
			{
				AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
				SCR_AIMessageHandling.SendGetInMessage(agentToUseAsDriver, groupVehicle.GetEntity(), EAICompartmentType.Pilot, null, myComms, NODE_NAME);
				m_aInformedAgents.Insert(agentToUseAsDriver);
			}
			else 
				continue;
		}
		if (m_aInformedAgents.IsEmpty())
			m_iStateOfExecution = STATE_FINISHED;
		m_iStateOfExecution = STATE_WAITING;
		return ENodeResult.RUNNING;
	
	}
		
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected ENodeResult Waiting_State()
	{
		foreach (AIAgent agent : 	m_aInformedAgents)
		{
			SCR_AIInfoComponent info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
			if (info &&  !info.HasUnitState(EUnitState.PILOT))
				return ENodeResult.RUNNING;
		}
		m_iStateOfExecution = STATE_FINISHED;
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected ENodeResult Finished_State()
	{
		bool haveUsableVehicles = m_aUsedVehicles.Count() - m_iNumberOfUnpilotableVehicles > 0;
		int vehiclesToOccupy = m_aVehiclesToOccupy.Count();
		bool canOccupyVehicles = vehiclesToOccupy == 0 || m_aInformedAgents.Count() == vehiclesToOccupy;
		bool canUseVehicles = haveUsableVehicles && canOccupyVehicles;
		SetVariableOut(PORT_CAN_USE_VEHICLE, canUseVehicles);
		if (canUseVehicles)
			return ENodeResult.SUCCESS;
		return ENodeResult.FAIL;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected void CancelOrders()
	{
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
		for (int index = m_aInformedAgents.Count() - 1; index >=0; index--)
		{
			if (m_aInformedAgents[index])
				SCR_AIMessageHandling.SendCancelMessage(m_aInformedAgents[index], null, myComms, NODE_NAME);
			m_aInformedAgents.Remove(index);
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (!m_Utility)
			return;
		m_Utility.m_OnAgentLifeStateChanged.Remove(OnAgentLifeStateChanged);
		switch (m_iStateOfExecution)
		{
			case STATE_SENDING_SIGNALS:
			{
				CancelOrders();
				break;
			}
			case STATE_WAITING:
			{
				CancelOrders();
				break;
			}
			case STATE_FINISHED:
			{
				return;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	void OnAgentLifeStateChanged(AIAgent incapacitatedAgent, SCR_AIInfoComponent infoIncap, IEntity vehicle, ECharacterLifeState lifeState)
	{
		if (vehicle && infoIncap.HasUnitState(EUnitState.PILOT) && m_iStateOfExecution != STATE_FINISHED)
			Testing_State(false);
		else if (m_aInformedAgents.Find(incapacitatedAgent) > -1)
			m_iStateOfExecution = STATE_SENDING_SIGNALS;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected AIAgent GetAgentThatIsConscious(notnull array<AIAgent> agents, array<AIAgent> restrictedAgents, bool excludeDrivers = false, float excludeDistance = -1, vector locationOfAccident = vector.Zero)
	{
		foreach (AIAgent agent : agents)
		{
			if (excludeDrivers)
			{
				SCR_AIInfoComponent info = SCR_AIInfoComponent.Cast(agent.FindComponent(SCR_AIInfoComponent));
				if (!info || info.HasUnitState(EUnitState.PILOT))
					continue;
			}
			IEntity agentEntity = agent.GetControlledEntity();
			if (SCR_AIDamageHandling.IsConscious(agentEntity) && restrictedAgents.Find(agent) == -1)
			{
				if (excludeDistance > 0)
				{
					vector agentPos = agent.GetControlledEntity().GetOrigin();
					if (vector.Distance(agentPos,locationOfAccident) > excludeDistance)
						continue;
				}
				return agent;
			}
		}
		return null;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_CAN_USE_VEHICLE };
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "OccupyDriversInUsedVehicles: goes over the array of known vehicles and tries to occupy the drivers with alive group members. Is running while seats are not occupied.\nReturns bool if we can occupy (at least one) driver.";
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	};
};