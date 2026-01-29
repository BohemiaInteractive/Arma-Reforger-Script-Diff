class SCR_AIAnimateActivity : SCR_AIActivityBase
{
	protected ref array<AIAgent> m_aAgentsAnimating = {};
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIAnimateActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, float priority = PRIORITY_ACTIVITY_ANIMATE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		SetPriority(priority);
		m_fPriorityLevel.Init(this, priorityLevel);	
	}
	
	//------------------------------------------------------------------------------------------------
	bool AddAgentToAnimatedAgents(AIAgent agent)
	{
		if (m_aAgentsAnimating.Find(agent) > -1)
		{
			Print("Agent added twice to list of animated agents!", LogLevel.WARNING);
			return false;
		}	
		m_aAgentsAnimating.Insert(agent);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	bool RemoveAgentFromAnimatedAgents(AIAgent agent, bool isSuccess)
	{
		int index = m_aAgentsAnimating.Find(agent);
		if (index < 0)
		{
			Print("Agent is not animated!", LogLevel.WARNING);
			return false;
		}
		m_aAgentsAnimating.Remove(index);
		if (m_aAgentsAnimating.IsEmpty())
		{
			if (isSuccess)
				Complete();
			else 
				Fail();
			if (m_RelatedWaypoint)
				m_Utility.m_Owner.CompleteWaypoint(m_RelatedWaypoint);
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		//we distribute now all animation scripts to all our agents, in future one might use beh tree if the distribution becomes context-related
		array<AIAgent> agents = {};
		m_Utility.m_Owner.GetAgents(agents);
		AICommunicationComponent myComms = m_Utility.m_Owner.GetCommunicationComponent();
		SCR_AIAnimationWaypoint waypoint = SCR_AIAnimationWaypoint.Cast(m_RelatedWaypoint);
		if (!waypoint)
			return;
		foreach (int index, AIAgent agent: agents)
		{
			if (!waypoint.m_aAnimationScripts.IsIndexValid(index))
				break;
			SCR_AIMessageHandling.SendAnimateMessage(agent, waypoint,  waypoint.m_aAnimationScripts[index], this, waypoint.GetOnAnimationBehaviorAction(), myComms, "Animate activity director");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " animating on " + m_RelatedWaypoint.ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}
};