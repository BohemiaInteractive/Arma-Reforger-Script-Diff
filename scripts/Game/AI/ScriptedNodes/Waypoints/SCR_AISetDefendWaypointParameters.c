class SCR_AISetDefendWaypointParameters : AITaskScripted
{
	static const string PRESET_INDEX_PORT = "PresetIndexIn";
	static const string FAST_INIT_PORT = "FastInitIn";
	static const string WAYPOINT_PORT = "WaypointIn";
	
	[Attribute("-1", UIWidgets.EditBox, "Set current preset index of defend waypoint")];
	private int m_iNewPresetIndex;
	
	[Attribute("-1", UIWidgets.EditBox, "Set fast init for of defend waypoint")];
	private int m_iNewFastInit;
	
	[Attribute("1", UIWidgets.CheckBox, "Should call invoker OnWaypointPropertiesChange?")];
	private bool m_bCallInvoker;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_PORT,
		PRESET_INDEX_PORT,
		FAST_INIT_PORT
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIWaypoint waypoint;
		SCR_DefendWaypoint wp;
		int newIndex;
		
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}
		
		if (!GetVariableIn(WAYPOINT_PORT, waypoint))
		{
			waypoint = group.GetCurrentWaypoint();
		}
		
		wp = SCR_DefendWaypoint.Cast(waypoint);
		if (!wp)
		{
			return NodeError(this, owner, "Did not provide defend waypoint to set!");
		}
		
		if (!GetVariableIn(PRESET_INDEX_PORT, newIndex))
			newIndex = m_iNewPresetIndex;
		
		if (newIndex > -1 && !wp.SetCurrentDefendPreset(newIndex))	// I want to change preset
		{
			return NodeError(this, owner, "Wrong index of preset provided");
		}
		
		if (!GetVariableIn(FAST_INIT_PORT, newIndex))
			newIndex = m_iNewFastInit;
		
		if (newIndex > -1) // I want to change fast init state
		{
			wp.SetFastInit(newIndex);
		}
		
		if (m_bCallInvoker)
		{	
			wp.GetOnWaypointPropertiesChanged().Invoke();
		}	
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Changes preset of defend waypoint, defined in m_DefendPresets array of defend waypoint entity.";
	}
};