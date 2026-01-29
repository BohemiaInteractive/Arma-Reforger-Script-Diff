class SCR_AIChangeUnitState : AITaskScripted
{
	static const string PORT_STATE = "UnitState";
	
	[Attribute("0", UIWidgets.ComboBox, "State of unit", "", ParamEnumArray.FromEnum(EUnitState) )]
	protected EUnitState m_stateToChange;
	
	[Attribute("1", UIWidgets.CheckBox, "Add true, remove false")]
	protected bool m_stateAdd;
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		int unitState;
		if(GetVariableIn(PORT_STATE,unitState))
		{
			m_stateToChange = Math.AbsInt(unitState);
			m_stateAdd = unitState > 0;
		}	
		
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!chimeraAgent)
			return SCR_AgentMustChimera(this, owner);
		
		SCR_AIInfoComponent infoComp = 	chimeraAgent.m_InfoComponent;
		if (!infoComp)
			ENodeResult.FAIL;
		
		if (m_stateAdd)
			infoComp.AddUnitState(m_stateToChange);
		else
			infoComp.RemoveUnitState(m_stateToChange);
		return ENodeResult.SUCCESS;
	}
	
	override string GetNodeMiddleText() 
	{ 
		string result = "Removing";
		if (m_stateAdd)
			result = "Adding";
		return result + " " + typename.EnumToString(EUnitState,m_stateToChange);
	};
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_STATE
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	static override string GetOnHoverDescription() 
	{ 
		return "Changes Unit State either adding flag or removing depending of sign of variable in in-port";
	};
};
