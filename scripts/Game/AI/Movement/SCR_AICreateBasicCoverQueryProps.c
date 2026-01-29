class SCR_AICreateBasicCoverQueryProps : AITaskScripted
{
	// Outputs
	protected const static string PORT_COVER_QUERY_PROPERTIES = "CoverQueryProps";
	
	// Inputs
	protected const static string PORT_POSITION = "Position";
	protected const static string PORT_RADIUS = "Radius";
	
	protected ref CoverQueryProperties m_CoverQueryProps = new CoverQueryProperties();
	
	[Attribute("0", UIWidgets.EditBox)]
	protected float m_fRadius;
	
	[Attribute("0", UIWidgets.CheckBox)]
	protected bool m_bSelectHighestScoreCover;
	
	//---------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ownerEntity = owner.GetControlledEntity();
		if (!ownerEntity)
			return ENodeResult.FAIL;
		
		float radius = 0;
		if (!GetVariableIn(PORT_RADIUS, radius))
			radius = m_fRadius;
		
		vector queryPos = vector.Zero;
		if (!GetVariableIn(PORT_POSITION, queryPos))
			queryPos = ownerEntity.GetOrigin();
		
		m_CoverQueryProps.m_vNearestPolyHalfExtend = SCR_AIFindCover.NEAREST_POLY_HALF_EXTEND;
		m_CoverQueryProps.m_fNmAreaCostScale = SCR_AIFindCover.NAVMESH_AREA_COST_SCALE;
		m_CoverQueryProps.m_vSectorPos = queryPos;
		m_CoverQueryProps.m_vSectorDir = vector.Zero;
		m_CoverQueryProps.m_vThreatPos = queryPos; // Threat pos is not provided here, it's a basic query in radius
		m_CoverQueryProps.m_fQuerySectorAngleCosMin = -1.0;
		m_CoverQueryProps.m_fSectorDistMin = 0;
		m_CoverQueryProps.m_fSectorDistMax = radius;
		//m_CoverQueryProps.m_fCoverHeightMin;
		//m_CoverQueryProps.m_fCoverHeightMax = 10.0;
		m_CoverQueryProps.m_fCoverToThreatAngleCosMin = -1.0;
		m_CoverQueryProps.m_fScoreWeightDirection = 0;
		m_CoverQueryProps.m_fScoreWeightDistance = 1.0;
		m_CoverQueryProps.m_bCheckVisibility = false;
		m_CoverQueryProps.m_bSelectHighestScore = m_bSelectHighestScoreCover;
		m_CoverQueryProps.m_iMaxCoversToCheck = SCR_AIFindCover.MAX_COVERS_LOW_PRIORITY;
		
		SetVariableOut(PORT_COVER_QUERY_PROPERTIES, m_CoverQueryProps);
		
		return ENodeResult.SUCCESS;
	}
	
	
	
	//---------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_POSITION,
		PORT_RADIUS
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_COVER_QUERY_PROPERTIES
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override string GetOnHoverDescription() { return "Creates CoverQueryProperties for basic circular query without threat visibility checking"; }
}