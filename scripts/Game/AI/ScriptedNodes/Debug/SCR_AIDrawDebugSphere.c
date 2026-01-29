class SCR_AIDrawDebugSphere: AITaskScripted
{
	static const string PORT_ORIGIN	= "OriginIn";
	static const string PORT_RADIUS	= "RadiusIn";
		
	[Attribute("1", UIWidgets.EditBox, desc: "Radius of sphere" )]
	private float m_fRadius;
	
	[Attribute("0.18 0.80 0.44 1", UIWidgets.ColorPicker, desc: "Color of sphere" )]
	private ref Color m_Color;
		
	ref Shape sphere;
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_ORIGIN,
		PORT_RADIUS
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
			vector position;
			float radius;
			int color;
			
			GetVariableIn(PORT_ORIGIN, position);
			if(!GetVariableIn(PORT_RADIUS, radius))
				radius = m_fRadius;
			if (m_Color)
				color =m_Color.PackToInt();
			else
				color = COLOR_GREEN_A;
			sphere = Shape.CreateSphere(color, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, position, radius);
		}
#endif
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Draws a sphere of given radius";
	}
};