class SCR_AIClearActionsForDefendActivity: AITaskScripted
{
	static const string PORT_WAYPOINT				= "WaypointIn";
	
	[Attribute("false", UIWidgets.CheckBox, "Keep turrets occupied?")];
	bool m_bKeepTurrets;
	
	[Attribute("false", UIWidgets.CheckBox, "Keep smart actions occupied?")];
	bool m_bKeepSA;
	
	[Attribute("true", UIWidgets.Auto, "Keep sector covers")];
	bool m_bKeepSectorCovers;
		
	[Attribute("500", UIWidgets.EditBox, "Update interval")]
	protected float m_fUpdateInterval_ms;
	
	protected float m_fNextUpdate_ms;
	
	protected SCR_AIGroup m_groupOwner;
	protected SCR_DefendWaypoint m_RelatedWaypoint;
	protected bool m_bSendingCancel;
	protected SCR_MailboxComponent m_Mailbox;
	protected SCR_AIDefendActivity m_RelatedActivity;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
			{
				SCR_AgentMustBeAIGroup(this, owner);
				return;
			}	
		}
		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(m_groupOwner.FindComponent(SCR_AIGroupUtilityComponent));
		if (!utility)
			return;
		m_Mailbox = utility.m_Mailbox;
		m_RelatedActivity = SCR_AIDefendActivity.Cast(utility.GetCurrentAction());		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEnter(AIAgent owner)
	{
		m_bSendingCancel = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_groupOwner)
			return ENodeResult.FAIL;
		IEntity waypointEnt;
		
		if (!m_Mailbox || !m_RelatedActivity)
			return ENodeResult.FAIL;
		
		GetVariableIn(PORT_WAYPOINT,waypointEnt);		
		
		bool waitingForCancel = false;
		bool cancelSent;
		array<AIAgent> agents = {};
		m_groupOwner.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			SCR_ChimeraAIAgent chimeraAIAgent = SCR_ChimeraAIAgent.Cast(agent);
			if (!chimeraAIAgent)
				continue;
			SCR_AIUtilityComponent utility = chimeraAIAgent.m_UtilityComponent;
			if (!utility)
				continue;
			SCR_AIBehaviorBase currBehavior = utility.GetCurrentBehavior();
			if (SCR_AIDefendBehavior.Cast(currBehavior) && !m_bKeepSectorCovers)
			{
				waitingForCancel = true;
				if (m_bSendingCancel)
				{
					SCR_AIMessage_Cancel cancelMessage = SCR_AIMessage_Cancel.Create(m_RelatedActivity);
					m_Mailbox.RequestBroadcast(cancelMessage, agent);
					cancelSent = true;					
					continue;
				}	
				break;
			}
			if (SCR_AIPerformActionBehavior.Cast(currBehavior) && !m_bKeepSA)
			{
				waitingForCancel = true;
				if (m_bSendingCancel)
				{
					SCR_AIMessage_Cancel cancelMessage = SCR_AIMessage_Cancel.Create(m_RelatedActivity);
					m_Mailbox.RequestBroadcast(cancelMessage, agent);
					cancelSent = true;
					continue;				
				}
				break;						
			}
			if (utility.m_AIInfo.HasUnitState(EUnitState.IN_TURRET) && !m_bKeepTurrets)
			{
				waitingForCancel = true;
				if (m_bSendingCancel)
				{
					IEntity agentEntity = agent.GetControlledEntity();
					IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(agentEntity);
					SCR_AIBoardingParameters params = new SCR_AIBoardingParameters;
					SCR_AIMessage_GetOut getOutMessage = SCR_AIMessage_GetOut.Create(vehicle, params, m_RelatedActivity);
					m_Mailbox.RequestBroadcast(getOutMessage, agent);
					cancelSent = true;
					continue;				
				}	
				break;
			}			
		}
		if (cancelSent)
			m_bSendingCancel = false;
		if (waitingForCancel)
		{
			return ENodeResult.RUNNING;
		}		
		return ENodeResult.SUCCESS;
	}	
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_WAYPOINT		
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "ClearActionsForDefendActivity: Goes over all group members and makes them leave the actions they were assigned to. Returns RUNNING until all canceled behaviors are done.";
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() {	return true;	}
}