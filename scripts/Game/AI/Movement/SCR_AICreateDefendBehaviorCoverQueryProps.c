class SCR_AICreateDefendBehaviorCoverQueryProps : AITaskScripted
{
	// Outputs
	protected const static string PORT_COVER_QUERY_PROPERTIES = "CoverQueryProps";
	
	// Inputs
	protected const static string PORT_SECTOR_CENTER_POSITION = "SectorCenterPos";
	protected const static string PORT_SECTOR_AXIS_POINT_POSITION = "SectorAxisPointPos";
	protected const static string PORT_SECTOR_HALF_SIZE_DEG = "SectorHalfSizeDeg";
	protected const static string PORT_SECTOR_RADIUS = "SectorRadius";
	
	protected ref CoverQueryProperties m_CoverQueryProps = new CoverQueryProperties();
	
	protected const float QUERY_MIN_RADIUS = 8.0;
	protected const float QUERY_MAX_RADIUS = 18.0;
	protected const float QUERY_RADIUS_TO_WAYPOINT_RADIUS_RATIO = 0.3;
	
	//---------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity ownerEntity = owner.GetControlledEntity();
		if (!ownerEntity)
			return ENodeResult.FAIL;
		
		vector sectorCenterPos;
		if (!GetVariableIn(PORT_SECTOR_CENTER_POSITION, sectorCenterPos))
			return ENodeResult.FAIL;
		
		vector sectorAxisPointPos;
		if (!GetVariableIn(PORT_SECTOR_AXIS_POINT_POSITION, sectorAxisPointPos))
			return ENodeResult.FAIL;
		
		float sectorHalfSizeDeg;
		if (!GetVariableIn(PORT_SECTOR_HALF_SIZE_DEG, sectorHalfSizeDeg))
			return ENodeResult.FAIL;
		
		float sectorRadius;
		if (!GetVariableIn(PORT_SECTOR_RADIUS, sectorRadius))
			return ENodeResult.FAIL;
				
		vector sectorDir = (sectorAxisPointPos - sectorCenterPos).Normalized();
		
		// Threat position is not exactly known, but expected far away outside sector
		vector threatPos = sectorCenterPos + (4.0 * sectorRadius) * sectorDir;
		
		vector myPos = ownerEntity.GetOrigin();
		m_CoverQueryProps.m_vNearestPolyHalfExtend = SCR_AIFindCover.NEAREST_POLY_HALF_EXTEND;
		m_CoverQueryProps.m_fNmAreaCostScale = SCR_AIFindCover.NAVMESH_AREA_COST_SCALE;
		m_CoverQueryProps.m_vSectorPos = myPos;
		m_CoverQueryProps.m_vSectorDir = sectorDir;
		m_CoverQueryProps.m_vThreatPos = threatPos;
		m_CoverQueryProps.m_fQuerySectorAngleCosMin = -1; // Full circle
		m_CoverQueryProps.m_fCoverToThreatAngleCosMin = 0.707; // Cos(45 deg) we don't need cover direction to be so perfect
		m_CoverQueryProps.m_fScoreWeightDirection = 2.0;
		m_CoverQueryProps.m_fScoreWeightDistance = 1.0;
		m_CoverQueryProps.m_bCheckVisibility = false;
		m_CoverQueryProps.m_bSelectHighestScore = false; // Lowest score, nearest cover
		
		// Min and max distance
		// Clamp max distance at reasonable value
		float desiredQueryRadius = QUERY_RADIUS_TO_WAYPOINT_RADIUS_RATIO * sectorRadius;
		m_CoverQueryProps.m_fSectorDistMax = Math.Clamp(desiredQueryRadius, QUERY_MIN_RADIUS, QUERY_MAX_RADIUS);
		m_CoverQueryProps.m_fSectorDistMin = 0;
		
		// Defend waypoint usage is very numerous, that's why we should reduce its performance impact
		m_CoverQueryProps.m_iMaxCoversToCheck = SCR_AIFindCover.MAX_COVERS_LOW_PRIORITY;
		
		SetVariableOut(PORT_COVER_QUERY_PROPERTIES, m_CoverQueryProps);
		
		return ENodeResult.SUCCESS;
	}
	
	
	
	//---------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_SECTOR_CENTER_POSITION,
		PORT_SECTOR_AXIS_POINT_POSITION,
		PORT_SECTOR_HALF_SIZE_DEG,
		PORT_SECTOR_RADIUS
	};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_COVER_QUERY_PROPERTIES
	};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override string GetOnHoverDescription() { return "Creates CoverQueryProperties for defend behavior"; }
}