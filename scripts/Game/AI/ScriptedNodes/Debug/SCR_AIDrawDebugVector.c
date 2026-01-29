class SCR_AIDrawDebugVector: AITaskScripted
{
	static const string PORT_POS_START	= "StartPosIn";
	static const string PORT_POS_END 	= "EndPosIn";
	
	ref Shape sphere;
	ref Shape arrow;

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_POS_START,
		PORT_POS_END
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
    {
        return true;
    }

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
		{
			vector posFrom,posTo;
			
			GetVariableIn(PORT_POS_START, posFrom);
			GetVariableIn(PORT_POS_END, posTo);
			
			arrow = Shape.CreateArrow(posFrom, posTo, 0.2, ARGB(100,255,0,0), 0);
			sphere = Shape.CreateSphere(ARGB(100,255,0,0),ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, posFrom, 0.05);
		}		
#endif
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Draws a vector from Start to End";
	}	
};