class SCR_AIAttackClusterActivity : SCR_AIFireteamsClusterActivity
{
	protected ref TFireteamLockArray m_aFireteamsAttack = {};
	protected ref TFireteamLockArray m_aFireteamsCover = {};
	
	bool m_bOrdersSent = false;
	
	//------------------------------------------------------------------------------------
	void SCR_AIAttackClusterActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint,
		notnull SCR_AITargetClusterState clusterState,
		notnull TFireteamLockRefArray ftAttack, notnull TFireteamLockRefArray ftCover)
	{
		SetPriority(PRIORITY_ACTIVITY_ATTACK_CLUSTER);
		SetIsUniqueInActionQueue(false);
		
		m_ClusterState = clusterState;
		
		foreach (auto ft : ftAttack)
		{
			RegisterFireteam(ft);
			m_aFireteamsAttack.Insert(ft);
		}
		
		foreach (auto ft : ftCover)
		{
			RegisterFireteam(ft);
			m_aFireteamsCover.Insert(ft);
		}
	}
	
	//------------------------------------------------------------------------------------
	void GetSpecificFireteams(notnull TFireteamLockRefArray fireteamsAttack, notnull TFireteamLockRefArray fireteamsCover)
	{
		fireteamsAttack.Clear();
		fireteamsCover.Clear();
		
		foreach (auto ft : m_aFireteamsAttack)
			fireteamsAttack.Insert(ft);
		
		foreach (auto ft : m_aFireteamsCover)
			fireteamsCover.Insert(ft);
	}
	
	//------------------------------------------------------------------------------------
	override void OnFireteamRemovedFromGroup(SCR_AIGroupFireteam ft)
	{
		if (SCR_AIGroupFireteamLock.RemoveFireteamLock(m_aFireteamsAttack, ft))
		{
			// Was an attacking fireteam
			if (m_aFireteamsAttack.IsEmpty())
			{
				#ifdef AI_DEBUG
				AddDebugMessage("All attacking fireteams are destroyed, activity is failed");
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
			SCR_ChimeraAIAgent leader = SCR_ChimeraAIAgent.Cast(m_Utility.m_Owner.GetLeaderAgent());
			if (leader)
			{
				SCR_AICommsHandler commsHandler = SCR_AISoundHandling.FindCommsHandler(leader);
				if (!commsHandler.CanBypass())
				{
					foreach (SCR_AIGroupFireteamLock ftLock : m_aFireteamsAttack)
					{
						IEntity talkRequestEntity = ftLock.GetFireteam().GetFirstMemberEntity();		
						
						int flankValue = Math.RandomIntInclusive(0, 1);
						SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_FLANK, talkRequestEntity, vector.Zero, flankValue, false, false, SCR_EAITalkRequestPreset.MANDATORY);
						commsHandler.AddRequest(rq);
					}
				}
			}
			
			SendAttackMessages();
			m_bOrdersSent = true;
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
		m_aFireteamsAttack.Clear();
		m_aFireteamsCover.Clear();
	}
	
	
	//------------------------------------------------------------------------------------
	void SendAttackMessages()
	{
		AICommunicationComponent comms = m_Utility.m_Owner.GetCommunicationComponent();
		if (!comms)
			return;
		
		array<AIAgent> agents = {};
		foreach (SCR_AIGroupFireteamLock ft : m_aFireteamsCover)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				if (SCR_AICompartmentHandling.IsInCompartment(agent))
					continue;
				
				SCR_AIMessage_AttackCluster msg = SCR_AIMessage_AttackCluster.Create(m_ClusterState, false);
				msg.m_RelatedGroupActivity = this;
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);
			}
		}
		
		foreach (SCR_AIGroupFireteamLock ft : m_aFireteamsAttack)
		{	
			ft.GetFireteam().GetMembers(agents);
			foreach (AIAgent agent : agents)
			{
				if (!agent)
					continue;
				
				if (SCR_AICompartmentHandling.IsInCompartment(agent))
					continue;
				
				SCR_AIMessage_AttackCluster msg = SCR_AIMessage_AttackCluster.Create(m_ClusterState, true); // Attacking fireteam is allowed to investigate
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
				
				SCR_AIMessage_AttackClusterDone msg = SCR_AIMessage_AttackClusterDone.Create();
				msg.m_RelatedGroupActivity = this;
				msg.SetReceiver(agent);
				comms.RequestBroadcast(msg, agent);

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
		foreach (SCR_AIGroupFireteamLock ftLock : m_aFireteamsAttack)
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		str = str + "| ";
		foreach (SCR_AIGroupFireteamLock ftLock : m_aFireteamsCover)
			str = str + string.Format("%1, ", m_Utility.m_FireteamMgr.GetFireteamId(ftLock.GetFireteam()));
		
		str = str + string.Format(") => C: %1", m_Utility.m_Perception.GetTargetClusterStateId(m_ClusterState));
		
		return str;
	}
}