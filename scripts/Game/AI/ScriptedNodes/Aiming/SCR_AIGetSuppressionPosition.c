class SCR_AIGetSuppressionPosition: AITaskScripted
{
  	static const string PORT_DURATION = "DurationIn";
  	static const string PORT_SHOOT_START = "ShootStartIn";
	static const string PORT_SHOOT_END = "ShootEndIn";
	
	static const string PORT_AIM_POSITION = "AimPositionOut";
	static const string PORT_PROGRESS = "ProgressOut";
	static const string PORT_IDLE_TIME = "IdleTimeSOut";
	
	protected float m_fFireDuration;
	protected vector m_vStartPos, m_vEndPos;
	
	protected SCR_AICombatComponent m_CombatComponent;
		
#ifdef WORKBENCH
	//Diagnostic visualization
	ref array<ref Shape> m_aDbgShapes = {};
#endif
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.FindComponent(SCR_AICombatComponent));
	}

	//------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		float duration;
		vector startPos, endPos;
		
		if (!GetVariableIn(PORT_DURATION, duration) || !GetVariableIn(PORT_SHOOT_START, startPos) || !GetVariableIn(PORT_SHOOT_END, endPos))
			return ENodeResult.FAIL;
						
		// Start of new line
		if (m_vStartPos != startPos || m_vEndPos != endPos)
		{
			m_vStartPos = startPos;
			m_vEndPos = endPos;
			m_fFireDuration = 0;
		}
		// Continuation of same line
		else
			m_fFireDuration += dt;
		
		float progress = Math.Clamp(m_fFireDuration / duration, 0, 1);
		vector aimPosition = startPos + ((endPos - startPos) * progress);
		
		SetVariableOut(PORT_AIM_POSITION, aimPosition);
		SetVariableOut(PORT_PROGRESS, progress);
		
#ifdef WORKBENCH
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SUPPRESS_DEBUG))
		{
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.DARK_GREEN, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, aimPosition, 0.1));
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.DARK_RED, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, m_vStartPos, 0.1));
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.DARK_YELLOW, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, m_vEndPos, 0.1));
			m_aDbgShapes.Insert(Shape.CreateArrow(m_vStartPos, m_vEndPos, 1, Color.DARK_CYAN, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE));
		}
#endif
		
		return ENodeResult.SUCCESS;
    }
	
	//------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_DURATION,
		PORT_SHOOT_START,
		PORT_SHOOT_END
	};
	
	//------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_AIM_POSITION,
		PORT_PROGRESS,
		PORT_IDLE_TIME
	};
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------------------------------------------
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Calculate position to shoot at during suppressive fire";
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() 
	{
		return true;
	}
};

