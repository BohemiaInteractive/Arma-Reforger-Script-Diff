class SCR_AIDecideBehavior: AITaskScripted
{
	// Update intervals for different LOD values
	static const int LOD_MAX = 10;
	static const int LOD_COUNT = LOD_MAX + 1;
	static float s_aUpdateIntervals[LOD_COUNT] = {
		0.55,	// LOD 0
		1.3,	// 1
		2.0,	
		2.0,	// 3
		2.0,
		2.0,	// 5
		2.0,
		2.0,	// 7
		2.0,
		2.0,	// 9
		2.0
	};
	
	// Inputs
	protected static string PORT_UNKNOWN_TARGET = "UnknownTarget";
	
	// Outputs
	protected static string PORT_BEHAVIOR_TREE = "BehaviorTree";
	protected static string PORT_UPDATE_BEHAVIOR = "UpdateBehavior";
	protected static string PORT_USE_COMBAT_MOVE = "UseCombatMove";
	protected static string PORT_UPDATE_INTERVAL = "UpdateInterval";
	
	protected SCR_AIBehaviorBase		m_PreviousBehavior;
	protected SCR_AIBehaviorBase		m_CurrentBehavior;	
	protected SCR_AIUtilityComponent	m_UtilityComponent;	
	
	protected float m_fRandomDelay_s; // Random delay added to update interval to distribute updated over time more evenly
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (!m_UtilityComponent)
		{
			NodeError(this, owner, "Can't find utility component.");
		}
		
		m_fRandomDelay_s = Math.RandomFloat(0.0, s_aUpdateIntervals[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_UtilityComponent)
			return ENodeResult.FAIL;
		
		BaseTarget unknownTarget;		
		GetVariableIn(PORT_UNKNOWN_TARGET, unknownTarget);

		m_CurrentBehavior = m_UtilityComponent.EvaluateBehavior(unknownTarget);
		if (!m_CurrentBehavior || m_CurrentBehavior.m_sBehaviorTree == ResourceName.Empty)
		{
			Print("AI: Missing behavior tree in " + m_CurrentBehavior.ToString(), LogLevel.WARNING);
			return ENodeResult.FAIL;
		}

		if (m_PreviousBehavior != m_CurrentBehavior)
		{
			SetVariableOut(PORT_BEHAVIOR_TREE, m_CurrentBehavior.m_sBehaviorTree);
			SetVariableOut(PORT_UPDATE_BEHAVIOR, true);
		}
		
		// m_bUseCombatMove can change at behavior run time
		SetVariableOut(PORT_USE_COMBAT_MOVE, m_CurrentBehavior.m_bUseCombatMove);
		
		// Resolve desired update interval
		int lod = Math.ClampInt(owner.GetLOD(), 0, LOD_MAX);
		float updateInterval = s_aUpdateIntervals[lod] + m_fRandomDelay_s;
		SetVariableOut(PORT_UPDATE_INTERVAL, updateInterval);
		m_fRandomDelay_s = 0;
				
		m_PreviousBehavior = m_CurrentBehavior;
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_BEHAVIOR_TREE,
		PORT_UPDATE_BEHAVIOR,
		PORT_USE_COMBAT_MOVE,
		PORT_UPDATE_INTERVAL
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_UNKNOWN_TARGET
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
};