class CoverQueryProperties
{
	// Maximum amount of covers which will be checked. If we discover more than this amount of covers in sector,
	// then covers are decimated.
	// This is an optimization, because cover scoring pathfinding is performance heavy.
	// If this value is 0, decimation is not used and all covers are checked.
	int m_iMaxCoversToCheck;
	
	vector m_vNearestPolyHalfExtend;		// Half extend of nearest polygon search
	vector m_vSectorPos;					// Center of cover query sector
	vector m_vSectorDir;					// Direction of cover query sector
	vector m_vThreatPos;					// Threat (target) position
	float m_fQuerySectorAngleCosMin = -1.0;	// Min cos of cover query sector angle
	float m_fSectorDistMin;					// Min distance to cover from cover query sector center
	float m_fSectorDistMax;					// Max distance to cover from cover query sector center
	float m_fCoverHeightMin;				// Min height of cover
	float m_fCoverHeightMax = 10.0;			// Max height of cover
	float m_fCoverToThreatAngleCosMin = -1.0;	// Min cos of angle between cover's own direction and direction towards enemy from this cover

	float m_fScoreWeightDirection;			// Weight of directivity. Increases score of covers with bigger angle between m_vSectorDir and cover pos.
	float m_fScoreWeightDistance = 1.0;		// Weight of distance between sector center and cover

	float m_fNmAreaCostScale = 1.0;			// Scale of navmesh area costs taken from navmesh project
	
	bool m_bCheckVisibility;				// When true, only covers which have direct visibility of m_vThreatPos will be returned
	float m_fVisibilityTraceFraction = 0.5;	// When visibility check is enabled, vis. is considered good if the fraction of trace is above this value
	
	bool m_bSelectHighestScore;				// When true, cover with highest score is selected, otherwise lowest score cover is selected.
}