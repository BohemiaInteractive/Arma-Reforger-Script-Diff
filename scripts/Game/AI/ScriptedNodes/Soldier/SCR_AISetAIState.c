class SCR_AISetAIState : AITaskScripted
{
	static const string PORT_AI_STATE = "AIState";
	
	[Attribute("0", UIWidgets.ComboBox, "State of AI", "", ParamEnumArray.FromEnum(EUnitAIState) )]
	protected EUnitAIState m_stateToSet;
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		EUnitAIState state;
		if(!GetVariableIn(PORT_AI_STATE,state))
			state = m_stateToSet;
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!chimeraAgent)
			return SCR_AgentMustChimera(this, owner);
		
		SCR_AIInfoComponent infoComp = 	chimeraAgent.m_InfoComponent;
		if (!infoComp)
			ENodeResult.FAIL;
		
		infoComp.SetAIState(state);
		return ENodeResult.SUCCESS;
	}
	
	static override string GetOnHoverDescription() 
	{ 
		return "Sets AI state either on self";	
	};
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_AI_STATE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
};
