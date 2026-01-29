class SCR_AIGetLookParameters : AITaskScripted
{
	static const string PORT_LOOK = "bLook";
	static const string PORT_CANCEL = "bCancel";
	static const string PORT_RESTART = "bRestart";
	static const string PORT_POSITION = "vPosition";
	static const string PORT_DURATION = "vDuration";
	
	SCR_AILookAction m_LookAction;
	SCR_AISectorThreatFilter m_ThreatFilter;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_LookAction)
			return ENodeResult.FAIL;

		bool look, cancelLook, restartLook;
		vector lookPos;
		float lookDuration;
		m_LookAction.MoveLookParametersToNode(look, lookPos, lookDuration, cancelLook, restartLook);
		
		SetVariableOut(PORT_LOOK, look);
		
		if (look)
		{
			SetVariableOut(PORT_CANCEL, cancelLook);
			SetVariableOut(PORT_RESTART, restartLook);
			
			SetVariableOut(PORT_POSITION, lookPos);
			SetVariableOut(PORT_DURATION, lookDuration);
		}
		
		return ENodeResult.SUCCESS;
	}
	
	protected static override bool VisibleInPalette() {return true;}
	protected static override string GetOnHoverDescription() {return "Gets parameters of a lookAction";}
	
	override void OnInit(AIAgent owner)
	{
		SCR_AIUtilityComponent utility = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		
		if (utility)
		{
			m_LookAction = utility.m_LookAction;
			m_ThreatFilter = utility.m_SectorThreatFilter;
		}
	}

	protected static ref TStringArray s_aVarsOut = {
		PORT_LOOK,
		PORT_CANCEL,
		PORT_RESTART,
		PORT_POSITION,
		PORT_DURATION
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
};