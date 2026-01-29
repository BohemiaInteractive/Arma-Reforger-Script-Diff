class SCR_AIInvestigateClusterActivity : SCR_AIFireteamsClusterActivity
{
	protected ref TFireteamLockRefArray m_aFireteamsInvestigate = {};
	protected ref TFireteamLockRefArray m_aFireteamsCover = {};
	
	protected bool m_bOrdersSent = false;
	
	protected static const float INVESTIGATION_RADIUS_MIN = 20.0;
	
	//------------------------------------------------------------------------------------
	//! ftInvestigate - fireteams which will investigate
	//! ftCover - fireteams which will cover others
	void SCR_AIInvestigateClusterActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint,
		notnull SCR_AITargetClusterState clusterState,
		notnull TFireteamLockRefArray ftInvestigate, notnull TFireteamLockRefArray ftCover)
	{
		SetPriority(PRIORITY_ACTIVITY_INVESTIGATE_CLUSTER);
		SetIsUniqueInActionQueue(false);
		
		foreach (auto ft : ftInvestigate)
		{
			RegisterFireteam(ft);
			m_aFireteamsInvestigate.Insert(ft);
		}
		
		foreach (auto ft : ftCover)
		{
			RegisterFireteam(ft);
			m_aFireteamsCover.Insert(ft);
		}
	}
	
	//------------------------------------------------------------------------------------
	void GetSpecificFireteams(notnull TFireteamLockRefArray fireteamsInvestigate, notnull TFireteamLockRefArray fireteamsCover)
	{
		fireteamsInvestigate.Clear();
		fireteamsCover.Clear();
		
		foreach (auto ft : m_aFireteamsInvestigate)
			fireteamsInvestigate.Insert(ft);
		
		foreach (auto ft : m_aFireteamsCover)
			fireteamsCover.Insert(ft);
	}
	
	//------------------------------------------------------------------------------------
	override void OnFireteamRemovedFromGroup(SCR_AIGroupFireteam ft)
	{
		// Was it an investigating fireteam?
		if (SCR_AIGroupFireteamLock.RemoveFireteamLock(m_aFireteamsInvestigate, ft))
		{
			if (m_aFireteamsInvestigate.IsEmpty())
			{
				#ifdef AI_DEBUG
				AddDebugMessage("All investigating fireteams are destroyed, activity is failed");
				#endif
				
				Fail();
			}
		}
		else
		{
			SCR_AIGroupFireteamLock.RemoveFireteamLock(m_aFireteamsCover, ft);
		}
		
		UnregisterFireteam(ft);
	}
	
	//------------------------------------------------------------------------------------	
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		if (!m_bOrdersSent)
		{
			vector investigatePos;
			float investigateRadius;
			CalculateInvestigationArea(m_ClusterState, investigatePos, investigateRadius);
			
			AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
			if (!comms)
				return;
			
			SendInvestigateMessages(comms, investigatePos, investigateRadius);
		}
		
		// !! It runs once and then is suspended, we don't need to run the behavior tree for it.
		//SetActionIsSuspended(true);
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		m_bOrdersSent = false;
		SendCancelMessages();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessages();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessages();
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionRemoved()
	{
		// Release fireteams instantly, if action is still referenced by something else
		super.OnActionRemoved();
		m_aFireteamsInvestigate.Clear();
		m_aFireteamsCover.Clear();
	}
	
	//------------------------------------------------------------------------------------
	protected void SendCancelMessages()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		// Send to all agents in all assigned fireteams
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteamLock ft : m_aAssignedFireteams)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Create(this);
				
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	protected void SendInvestigateMessages(AICommunicationComponent comms, vector pos, float radius)
	{
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteamLock ft : m_aFireteamsInvestigate)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				if (SCR_AICompartmentHandling.IsInCompartment(agent))
					continue;
				
				// Duration is large, since soldiers' investigation is tied to this activity
				SCR_AIMessage_Investigate msg = SCR_AIMessage_Investigate.Create(this,pos, radius, true, duration: 10000);
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);
			}
		}
		
		foreach (SCR_AIGroupFireteamLock ft : m_aFireteamsCover)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				if (SCR_AICompartmentHandling.IsInCompartment(agent))
					continue;
				
				SCR_AIMessage_CoverCluster msg = new SCR_AIMessage_CoverCluster();
				msg.m_RelatedGroupActivity = this;
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	//! Calculates position and radius of investigation area
	static void CalculateInvestigationArea(notnull SCR_AITargetClusterState s, out vector outCenterPos, out float outRadius)
	{
		const float minRadius = INVESTIGATION_RADIUS_MIN;
		
		vector center = 0.5*(s.m_vBBMin + s.m_vBBMax);
		float radius = Math.Max(minRadius, vector.DistanceXZ(center, s.m_vBBMax));
		
		outCenterPos = center;
		outRadius = radius;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetDebugPanelText()
	{
		// List all assigned fireteams
		string str = "FTs: (";
		foreach (SCR_AIGroupFireteamLock ftLock : m_aFireteamsInvestigate)
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		str = str + "| ";
		foreach (SCR_AIGroupFireteamLock ftLock : m_aFireteamsCover)
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		
		str = str + string.Format(") => C: %1", m_Utility.m_Perception.GetTargetClusterStateId(m_ClusterState));
		
		return str;
	}
}