//------------------------------------------------------------------------------------------------
class SCR_AIDecoIsAboveThreatLevel : DecoratorScripted
{
	static private string THRESHOLD_PORT = "ThresholdIn";
	
	SCR_AIInfoComponent m_InfoComponent;
	
	[Attribute("1", UIWidgets.ComboBox, "Threat threshold", "", ParamEnumArray.FromEnum(EAIThreatState) )]
	private EAIThreatState m_threatThreshold;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(owner);
		if (!chimeraAgent)
			SCR_AgentMustChimera(this, owner);
		m_InfoComponent = chimeraAgent.m_InfoComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool TestFunction(AIAgent owner)
	{		
		if (!m_InfoComponent)
		{
			return false;
		};
		
		EAIThreatState threshold;
		if (!GetVariableIn(THRESHOLD_PORT,threshold))
			threshold = m_threatThreshold;
		
		return threshold < m_InfoComponent.GetThreatState();	
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns true if current threat state is higher than given threshold.";
	}
	
	//------------------------------------------------------------------------------------------------
	override protected string GetNodeMiddleText()
	{
		return "Threshold: " + typename.EnumToString(EAIThreatState,m_threatThreshold);
	}	

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		THRESHOLD_PORT
	};
	protected override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
};
