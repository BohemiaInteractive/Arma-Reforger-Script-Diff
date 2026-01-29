class SCR_AIDefendActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<vector> m_vDefendDirection = new SCR_BTParam<vector>(SCR_AIActionTask.DEFEND_DIRECTION_PORT);	
	
	protected ref array<AIAgent> m_aRadialCoverAgents = {};
	//-------------------------------------------------------------------------------------------------
	void InitParameters(vector defendDirection, float priorityLevel)
	{
		m_vDefendDirection.Init(this, defendDirection);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//-------------------------------------------------------------------------------------------------
	void SCR_AIDefendActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector defendDirection, float priority = PRIORITY_ACTIVITY_DEFEND, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(defendDirection, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityDefend.bt";
		SetPriority(priority);
		if (!relatedWaypoint)
			return;
		
		float waypointRadius = relatedWaypoint.GetCompletionRadius();
		vector waypointPositionWorld = relatedWaypoint.GetOrigin(); 
		if (defendDirection == vector.Zero) // use orientation of the waypoint
		{
			m_vDefendDirection.m_Value = GetDefendDirection(relatedWaypoint, waypointRadius);
		}
		else
		{
			m_vDefendDirection.m_Value =  (defendDirection - waypointPositionWorld).Normalized() * waypointRadius + waypointPositionWorld;
		}
	}
	
	//-------------------------------------------------------------------------------------------------
	void AddAgentToRadialCover (AIAgent agent)
	{
		if (m_aRadialCoverAgents.Find(agent) == -1)
			m_aRadialCoverAgents.Insert(agent);	
	}
	
	//-------------------------------------------------------------------------------------------------
	void ClearRadialCoverAgents ()
	{
		m_aRadialCoverAgents.Clear();
	}
	
	//-------------------------------------------------------------------------------------------------
	int GetRadialCoverAgentsCount ()
	{
		return m_aRadialCoverAgents.Count();
	}
	
	//-------------------------------------------------------------------------------------------------
	void AllocateAgentsToRadialCover ()
	{
		if (!m_RelatedWaypoint)
			return;
		vector originOfLocalSpace = m_RelatedWaypoint.GetOrigin();	
		vector directionToDefend = m_vDefendDirection.m_Value - originOfLocalSpace;
		float angleToDefend = directionToDefend.ToYaw();
		float sector = 360 / m_aRadialCoverAgents.Count();
		float length = directionToDefend.Length();
		vector directionAxis;
		float angleRange = sector/2;
		foreach(int index, AIAgent agent: m_aRadialCoverAgents)
		{
			int directionAngle = Math.Round(angleToDefend + sector * index);
			directionAngle = directionAngle % 360;
			directionAxis[0] = Math.Sin(directionAngle * Math.DEG2RAD) * length;
			directionAxis[2] = Math.Cos(directionAngle * Math.DEG2RAD) * length;
			SendDefendSector(agent, directionAxis + originOfLocalSpace, angleRange);
		}
	}
	
	//-------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " defending area of " + m_RelatedWaypoint.ToString();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
		if (group)
			group.ReleaseCompartments();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Utility.GetAIAgent());
		if (group)
			group.ReleaseCompartments();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}
	
	//-------------------------------------------------------------------------------------------------
	bool SendDefendSector (AIAgent who, vector directionAxis, float angleRange)
	{
		SCR_AIMessage_Defend defendMessage = SCR_AIMessage_Defend.Create(directionAxis, angleRange, m_bIsWaypointRelated.m_Value, m_fPriorityLevel.m_Value, m_RelatedWaypoint, this);
		m_Utility.m_Mailbox.RequestBroadcast(defendMessage, who);
		return true;
	}
	
	//-------------------------------------------------------------------------------------------------
	//! calculates a direction vector from centralEntity towards frontal direction of the centralEntity
	static vector GetDefendDirection(IEntity centralEntity, float directionDistance)
	{
		vector startPositionWorld = centralEntity.GetOrigin(); 
		float rotationAngle = centralEntity.GetAngles()[1] * Math.DEG2RAD;
		vector endPositionWorld;
		endPositionWorld[0] = startPositionWorld[0] + Math.Sin(rotationAngle) * directionDistance;
		endPositionWorld[1] = startPositionWorld[1];
		endPositionWorld[2] = startPositionWorld[2] + Math.Cos(rotationAngle) * directionDistance;
		return endPositionWorld;
	}
};
