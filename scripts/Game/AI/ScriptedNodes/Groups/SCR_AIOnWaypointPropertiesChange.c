class SCR_AIOnWaypointPropertiesChange: AITaskScripted
{
	
	[Attribute("-1", UIWidgets.EditBox, "Timeout (ms) after it succeeds")];
	float m_fTimeout_Base_ms;
	
	[Attribute("0", UIWidgets.EditBox, "Random timeout delta")];
	float m_fTimeout_Random_ms;
	
	protected ScriptInvoker m_OnWaypointChanged, m_OnWaypointPropertiesChanged;
	protected bool m_bWaypointPropertiesChanged;
	protected float m_fActualTime_ms;
	protected float m_fTimeout_ms;
	
	
	override void OnEnter(AIAgent owner)
	{
		m_fActualTime_ms = GetGame().GetWorld().GetWorldTime();
		if (m_fTimeout_Base_ms >= 0) 
			m_fTimeout_ms = m_fTimeout_Base_ms + Math.RandomFloat(0, m_fTimeout_Random_ms);
	}
	
	//------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (m_fTimeout_Base_ms > 0 && (GetGame().GetWorld().GetWorldTime() - m_fActualTime_ms) > m_fTimeout_ms)
			return ENodeResult.SUCCESS;
				
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		if (!m_OnWaypointChanged) // register on event of changing the waypoint in group
		{
			m_OnWaypointChanged = group.GetOnCurrentWaypointChanged();
			m_OnWaypointChanged.Insert(OnCurrentWaypointChanged);
			OnCurrentWaypointChanged(group.GetCurrentWaypoint(), null); // register next changes to set waypoint
			return ENodeResult.SUCCESS;
		}
		
		if (m_bWaypointPropertiesChanged)
		{
			m_bWaypointPropertiesChanged = false;
			return ENodeResult.SUCCESS;
		}
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------------------
	void OnCurrentWaypointChanged(AIWaypoint currentWp, AIWaypoint prevWp)
	{
		if (m_OnWaypointPropertiesChanged)
			m_OnWaypointPropertiesChanged.Remove(OnWaypointPropertiesChanged);
		SCR_AIWaypoint wp = SCR_AIWaypoint.Cast(currentWp);
		if (!wp)
		{
			m_OnWaypointPropertiesChanged = null;
			return;
		}
		
		m_OnWaypointPropertiesChanged = wp.GetOnWaypointPropertiesChanged();
		m_OnWaypointPropertiesChanged.Insert(OnWaypointPropertiesChanged);
	}	
	
	//------------------------------------------------------------------------------------------------------------
	void OnWaypointPropertiesChanged() 
	{
		m_bWaypointPropertiesChanged = true;
	}
	
	//------------------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		if (nodeCausingAbort && nodeCausingAbort != this)
		{
			if (m_OnWaypointChanged)
				m_OnWaypointChanged.Remove(OnCurrentWaypointChanged);
			if (m_OnWaypointPropertiesChanged)
				m_OnWaypointPropertiesChanged.Remove(OnWaypointPropertiesChanged);
			m_bWaypointPropertiesChanged = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription() 
	{ 
		return "OnWaypointPropertiesChange: keeps running until group has SCR_AIWaypoint - waypoint. \nThen it keeps running until this waypoint has changed its properties or moved to different position.";
	}
		
	//------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true;}
	
	//------------------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning() { return true;}
}