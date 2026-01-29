class SCR_AILeaveStaticVehicles : AITaskScripted
{
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Update time of checks [ms]")]
	protected float m_fUpdateInterval_ms;
	
	static const string NODE_NAME = "SCR_AILeaveStaticVehicles";
	
	protected float m_fNextUpdate_ms;
	protected ref array<AIAgent> m_aAgents = {};
	protected ref array<AIAgent> m_aInformedAgents = {};
	protected ref array<ref SCR_AIGroupVehicle> m_aUsedVehicles = {};
	protected ref array<ref SCR_AIGroupVehicle> m_aVehiclesToLeave = {};
	protected SCR_AIGroup m_Group;
	protected SCR_AIGroupUtilityComponent m_Utility;
	protected int m_iStateOfExecution;	
		
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
		m_Utility.m_OnAgentLifeStateChanged.Remove(OnAgentLifeStateChanged);
		m_Utility.m_OnAgentLifeStateChanged.Insert(OnAgentLifeStateChanged);
		m_iStateOfExecution = STATE_TESTING_STATE;	
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
		}	
		
		m_aVehiclesToLeave.Clear();
		m_Utility.m_VehicleMgr.GetAllVehicles(m_aUsedVehicles);
		
		foreach (SCR_AIGroupVehicle groupVehicle : m_aUsedVehicles)
		{
			if (!groupVehicle)
				continue;
			// we skip group helicopters and static turrets
			SCR_AIVehicleUsageComponent vehicleUsage = groupVehicle.GetVehicleUsageComponent();
			if (groupVehicle.IsStatic() || !groupVehicle.CanMove() || (vehicleUsage && !vehicleUsage.CanBePiloted()))
			{
				m_aVehiclesToLeave.Insert(groupVehicle);
				continue;
			}			
		}
		bool willTryToLeave = m_aVehiclesToLeave.Count() > 0;
		if (willTryToLeave)
			m_iStateOfExecution = STATE_SENDING_SIGNALS;
		else
			m_iStateOfExecution = STATE_FINISHED;
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected ENodeResult SendingMessages_State()
	{
		if (m_aVehiclesToLeave.Count() == 0)
		{ 
			m_iStateOfExecution = STATE_FINISHED;
			return ENodeResult.RUNNING;
		};
		m_aInformedAgents.Clear();
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();	
		foreach (SCR_AIGroupVehicle groupVehicle : m_aVehiclesToLeave)
		{
			int vehicleHandleId = groupVehicle.GetSubgroupHandleId();
			array<AIAgent> agentsOfHandler = {};
			m_Utility.m_GroupMovementComponent.GetAgentsInHandler(agentsOfHandler, vehicleHandleId);
			
			foreach (int index, AIAgent agent : agentsOfHandler)
			{
				SCR_AIMessageHandling.SendDismountMessage(agent, groupVehicle.GetEntity(), index, null, myComms, NODE_NAME);					
				m_aInformedAgents.Insert(agent);	
			}			
		}
		if (m_aInformedAgents.IsEmpty())
			m_iStateOfExecution = STATE_FINISHED;
		m_iStateOfExecution = STATE_WAITING;
		return ENodeResult.RUNNING;
	
	}
		
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected ENodeResult Waiting_State()
	{
		foreach (AIAgent agent : m_aInformedAgents)
		{
			SCR_ChimeraAIAgent chimAg = SCR_ChimeraAIAgent.Cast(agent);
			if (!chimAg)
				continue;
			SCR_AIInfoComponent info = chimAg.m_InfoComponent;
			if (!info)
				continue;
			if (info.HasUnitState(EUnitState.IN_VEHICLE))
				return ENodeResult.RUNNING;					
		}
		m_iStateOfExecution = STATE_FINISHED;
		return ENodeResult.RUNNING;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------	
	protected ENodeResult Finished_State()
	{
		return ENodeResult.SUCCESS;		
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
		if (vehicle && infoIncap.HasUnitState(EUnitState.IN_VEHICLE) && m_iStateOfExecution != STATE_FINISHED)
			Testing_State(false);
		else if (m_aInformedAgents.Find(incapacitatedAgent) > -1)
			m_iStateOfExecution = STATE_SENDING_SIGNALS;
	}
		
	//----------------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
	{
		return true;
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "LeaveStaticVehicles: goes over the array of known vehicles and group members leave all static or broken vehicles. Is running while change is ongoing.";
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	};
};