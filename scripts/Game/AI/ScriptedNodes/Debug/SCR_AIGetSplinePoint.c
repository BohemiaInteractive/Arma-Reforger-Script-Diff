class SCR_AIGetSplinePoint: AITaskScripted
{
	static const string PORT_ENTITY = "EntityWithSplineIn";
	static const string PORT_POSITION = "PositionOut";
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_ENTITY
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_POSITION
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	private int m_iLastIndex;
	private ref array <vector> m_aOffsetPath;
	private vector m_vOrigin;
	private bool m_bShouldFail;
	
	
	protected override void OnInit(AIAgent owner)
	{
		IEntity ent;		
		if(!GetVariableIn(PORT_ENTITY,ent))
			ent = owner.GetControlledEntity();
		
		m_aOffsetPath = new array <vector>;
			
		if (ent.GetChildren())
		{
			IEntity entPoly = ent.GetChildren();
			PolylineShapeEntity polyline = PolylineShapeEntity.Cast(entPoly);
						
			while (!polyline && entPoly)
			{
				entPoly = entPoly.GetSibling();
				polyline = PolylineShapeEntity.Cast(entPoly);			
			};
			if (!polyline)
			{
				m_bShouldFail = true;
				return;
			}	

			m_vOrigin = polyline.GetOrigin();
			
			polyline.GetPointsPositions(m_aOffsetPath);			
		}	
	}
	
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (m_bShouldFail)
			return ENodeResult.FAIL;
		
		m_iLastIndex = (m_iLastIndex + 1) % m_aOffsetPath.Count();
		vector positionOut = m_aOffsetPath[m_iLastIndex] + m_vOrigin; 
						
		SetVariableOut(PORT_POSITION,positionOut);										
		
		return ENodeResult.SUCCESS;	
	}
	
	protected static override bool VisibleInPalette()
	{
		return true;
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "BT node for returning position on spline that is hierarchical child of provided entity. Each tick next point on spline is taken.";
	}	
	
};