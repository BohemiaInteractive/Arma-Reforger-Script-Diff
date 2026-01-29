class SCR_AIDefendFromClusterActivity : SCR_AIFireteamsClusterActivity
{
	bool m_bOrdersSent = false;
	
	//------------------------------------------------------------------------------------
	void SCR_AIDefendFromClusterActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint,
		notnull SCR_AITargetClusterState clusterState,
		notnull TFireteamLockRefArray fireteams)
	{
		SetPriority(PRIORITY_ACTIVITY_DEFEND_FROM_CLUSTER);
		SetIsUniqueInActionQueue(false);
		
		foreach (auto ft : fireteams)
		{
			RegisterFireteam(ft);
		}
	}
	
	//------------------------------------------------------------------------------------
	override void OnFireteamRemovedFromGroup(SCR_AIGroupFireteam ft)
	{	
		UnregisterFireteam(ft);
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();
		
		if (!m_bOrdersSent)
		{
			SendDefendMessages();
			m_bOrdersSent = true;
		}
		
		// !! It runs once and then is suspended, we don't need to run the behavior tree for it.
		//SetActionIsSuspended(true);
	}
	
	//------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessages();
	}
	
	//------------------------------------------------------------------------------------
	void SendDefendMessages()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteamLock ft : m_aAssignedFireteams)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				if (SCR_AICompartmentHandling.IsInCompartment(agent))
					continue;
				
				// Now we send attack message, but forbit investigation there
				SCR_AIMessage_AttackCluster msg = SCR_AIMessage_AttackCluster.Create(m_ClusterState, false);
				msg.m_RelatedGroupActivity = this;
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------
	void SendCancelMessages()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteamLock ft : m_aAssignedFireteams) // Send to all fireteams
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				SCR_AIMessage_Cancel msgCancel = SCR_AIMessage_Cancel.Create(this);
				msgCancel.SetReceiver(agent);
				comms.RequestBroadcast(msgCancel, agent);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetDebugPanelText()
	{
		// List all assigned fireteams
		string str = "FTs: (";
		foreach (SCR_AIGroupFireteamLock ftLock : m_aAssignedFireteams)
		{
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		}
		str = str + string.Format(") => C: %1", m_Utility.m_Perception.GetTargetClusterStateId(m_ClusterState));
		
		return str;
	}
}