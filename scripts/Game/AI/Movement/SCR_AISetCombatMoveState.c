class SCR_AISetCombatMoveState : AITaskScripted
{
	// Inputs
	protected static const string PORT_IN_COVER = "InCover";
	protected static const string PORT_EXPOSED_IN_COVER = "ExposedInCover";
	protected static const string PORT_AIM_AT_TARGET = "AimAtTarget";
	
	protected SCR_AICombatMoveState m_State;
	
	// Return value
	[Attribute(ENodeResult.SUCCESS.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ENodeResult))]
	protected ENodeResult m_eReturnValue;
	
	// Request state
	[Attribute()]
	protected bool m_bSetRequestState;
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(SCR_EAICombatMoveRequestState))]
	protected SCR_EAICombatMoveRequestState m_eRequestStateValue;
	
	// Fail reason
	[Attribute()]
	protected bool m_bSetRequestFailReason;
	[Attribute("0", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EAICombatMoveRequestFailReason))]
	protected SCR_EAICombatMoveRequestFailReason m_eRequestFailReason;
	
	// In cover
	[Attribute()]
	protected bool m_bSetInCover;
	[Attribute()]
	protected bool m_bInCoverValue;
	
	// Exposed in cover
	[Attribute()]
	protected bool m_bSetExposedInCover;
	[Attribute()]
	protected bool m_bExposedInCoverValue;
	
	// Fire when moving
	[Attribute()]
	protected bool m_bSetAimAtTarget;
	
	// Timers
	[Attribute()]
	protected bool m_bResetTimer;
	
	// Cover lock
	[Attribute()]
	protected bool m_bReleaseCoverLock;
	
	//--------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utilityComp = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		if (utilityComp)
			m_State = utilityComp.m_CombatMoveState;
	}
	
	//--------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_State)
			return ENodeResult.FAIL;
			
		if (m_bSetInCover)
		{
			if (!GetVariableIn(PORT_IN_COVER, m_State.m_bInCover))
				m_State.m_bInCover = m_bInCoverValue;
		}
		
		if (m_bSetExposedInCover)
		{
			if (!GetVariableIn(PORT_EXPOSED_IN_COVER, m_State.m_bExposedInCover))
				m_State.m_bExposedInCover = m_bExposedInCoverValue;
		}
		
		if (m_bSetAimAtTarget)
		{
			GetVariableIn(PORT_AIM_AT_TARGET, m_State.m_bAimAtTarget);
		}
		
		if (m_bReleaseCoverLock)
		{
			m_State.ReleaseCover();
		}
		
		return m_eReturnValue;
	}
	
	
	
	//--------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	override string GetNodeMiddleText()
	{
		array<string> strings = {
			"m_bSetRequestState",
			"m_bSetRequestFailReason",
			"m_bSetInCover",
			"m_bSetExposedInCover",
			"m_bResetTimer",
			"m_bReleaseCoverLock"
		};
		array<bool> values = {
			m_bSetRequestState,
			m_bSetRequestFailReason,
			m_bSetInCover,
			m_bSetExposedInCover,
			m_bResetTimer,
			m_bReleaseCoverLock
		};
		
		string str;
		int nLinesAdded = 1;
		str = str + string.Format("Node Result: %1", typename.EnumToString(ENodeResult, m_eReturnValue));
		for (int i = 0; i < strings.Count(); i++)
		{
			if (values[i])
			{
				if (nLinesAdded > 0)
					str = str + "\n";
				str = str + string.Format("%1: %2", strings[i], values[i]);
				
				string varName, varValue;
				if (i == 0)	// m_bSetRequestState
				{
					varName = "m_eRequestStateValue";
					varValue = typename.EnumToString(SCR_EAICombatMoveRequestState, m_eRequestStateValue);
				}
				else if (i == 1) // m_bSetRequestFailReason
				{
					varName = "m_eRequestFailReason";
					varValue = typename.EnumToString(SCR_EAICombatMoveRequestFailReason, m_eRequestFailReason);
				}
				else if (i == 2) // m_bSetInCover
				{
					varName = "m_bInCoverValue";
					varValue = m_bInCoverValue.ToString();
				}
				else if (i == 3) // m_bSetExposedInCover
				{
					varName = "m_bExposedInCoverValue";
					varValue = m_bExposedInCoverValue.ToString();
				}
				
				if (!varName.IsEmpty() && !varValue.IsEmpty())
					str = str + string.Format("\n%1: %2", varName, varValue);
				
				nLinesAdded++;
			}
		}
		return str;
	}
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_IN_COVER,
		PORT_EXPOSED_IN_COVER,
		PORT_AIM_AT_TARGET
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}