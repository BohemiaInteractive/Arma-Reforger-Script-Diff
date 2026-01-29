/*!
Action Cause value is not used for group settings now, unlike character settings.
This enum is here for potential future compatibility.
*/
enum SCR_EAIActivityCause
{
	SAFE = 10,
	ALWAYS = 1000 		// Special value for settings to run in all activities
}

class SCR_AIActivityBase : SCR_AIActionBase
{
	SCR_AIGroupUtilityComponent m_Utility;
	ref SCR_BTParam<bool> m_bIsWaypointRelated = new SCR_BTParam<bool>(SCR_AIActionTask.WAYPOINT_RELATED_PORT);
	AIWaypoint m_RelatedWaypoint = null;
	bool m_bAllowFireteamRebalance = true;	// Allows rebalancing of fireteams while this activity exists

	array<ref SCR_AIActivityFeatureBase> GetActivityFeatures()
	{
		return null;
	}
	
	SCR_AIActivityFeatureBase FindActivityFeature(typename featureType)
	{
		array<ref SCR_AIActivityFeatureBase> features = GetActivityFeatures();
		if (!features) return null;
		
		foreach (SCR_AIActivityFeatureBase feature : features)
			if (feature && feature.IsInherited(featureType))
				return feature;
		
		return null;
	}
		
	//---------------------------------------------------------------------------------------------------------------------------------
	void InitParameters(AIWaypoint relatedWaypoint)
	{
		bool isWaypointRelated = relatedWaypoint != null;
		m_bIsWaypointRelated.Init(this, isWaypointRelated);
		m_RelatedWaypoint = relatedWaypoint;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------	
	void SCR_AIActivityBase(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint)
	{
		m_Utility = utility;
		InitParameters(relatedWaypoint);
	}
	
	//------------------------------------------------------------------------------------
	protected void SendCancelMessagesToAllAgents()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		// Send to all agents
		array<AIAgent> agents = {};
		m_Utility.m_Owner.GetAgents(agents);
		foreach (AIAgent agent : agents)
		{
			if (!agent)
				continue;
			
			SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Create(this);
			
			msg.SetReceiver(agent);
			comms.RequestBroadcast(msg, agent);
		}
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	//! This is an overload of basic Fail() method, since we need to pass extra information regarding related waypoint
	//! \param[in] doNotCompleteWaypoint
	void Fail(bool doNotCompleteWaypoint)
	{
		// If doNotCompleteWaypoint == true, the waypoint completed first and should not be completed for the second time
		// we break the reference here deliberately and activity wont touch the waypoint from there on
		if (doNotCompleteWaypoint)
			m_RelatedWaypoint = null;
		
		Fail();
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------	
	// Called from OnActionFailed and OnActionCompleted of the child behavior
	void OnChildBehaviorFinished(SCR_AIBehaviorBase childBehavior);
	
	//---------------------------------------------------------------------------------------------------------------------------------	
	// Called from constructor of the child behavior. Here we can register child behaviors.
	void OnChildBehaviorCreated(SCR_AIBehaviorBase childBehavior);
	
	//---------------------------------------------------------------------------------------------------------------------------------
	#ifdef AI_DEBUG
	override void AddDebugMessage(string str)
	{
		if (!m_Utility || !m_Utility.m_Owner)
			return;
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(m_Utility.m_Owner.FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.ACTION);
	}
	#endif
};

class SCR_AIIdleActivity : SCR_AIActivityBase
{
	//---------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIIdleActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityIdle.bt";
		SetPriority(1.0);
	}
};
