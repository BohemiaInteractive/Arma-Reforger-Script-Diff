class SCR_AreaGeneratorBaseEntityClass : SCR_GeneratorBaseEntityClass
{
}

//! SCR_AreaGeneratorBaseEntity responsibilities:
//! - carry obstacle detection interface
class SCR_AreaGeneratorBaseEntity : SCR_GeneratorBaseEntity
{
	/*
		Obstacles
	*/

	[Attribute(defvalue: "0", desc: "[COSTY] Avoid objects - the trace check is a 10cm cylinder (for trees mostly)", category: "Obstacles")]
	protected bool m_bAvoidObjects;

	[Attribute(defvalue: "0", desc: "Avoid roads, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidRoads;

	[Attribute(defvalue: "0", desc: "Avoid rivers, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidRivers;

	[Attribute(defvalue: "0", desc: "Avoid power lines, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidPowerLines;

	[Attribute(defvalue: "0", desc: "Avoid tracks, respecting their clearance setting", category: "Obstacles")]
	protected bool m_bAvoidTracks;

	[Attribute(defvalue: "0", desc: "Avoid lakes", category: "Obstacles")]
	protected bool m_bAvoidLakes;

	[Attribute(defvalue: "0", desc: "Avoid above or below ocean level", category: "Obstacles", uiwidget: UIWidgets.ComboBox, enums: SCR_ParamEnumArray.FromString("None;Avoid land;Avoid ocean"))]
	protected int m_iAvoidLandOrOcean;

//	[Attribute(defvalue: "0", desc: "Land/Ocean separation offset", category: "Obstacles")]
//	protected float m_fAvoidLandOceanOffset;

	[Attribute(defvalue: "0", category: "Obstacles", desc: "Entirely regenerates if a shape obstacle (listed and checked above) is added/(re)moved in the area")]
	protected bool m_bRegenerateByObstacleChanges;

#ifdef WORKBENCH

	protected static ref SCR_ObstacleDetector s_ObstacleDetector;

	protected static const float BBOX_CHECK_HEIGHT = 100.0;
	protected static const float AVOID_OBJECTS_CHECK_RADIUS = 0.1;

	//------------------------------------------------------------------------------------------------
	protected void RefreshObstacles()
	{
		if (!m_ParentShapeSource)
			return;

		array<vector> vectorPoints = GetAnchorPoints(m_ParentShapeSource);
		SCR_AABB bbox = new SCR_AABB(vectorPoints);
		bbox.m_vMin[1] = BBOX_CHECK_HEIGHT * -0.5;
		bbox.m_vMax[1] = BBOX_CHECK_HEIGHT * 0.5;

		SetAvoidOptions();
		s_ObstacleDetector.RefreshObstaclesByAABB(CoordToParent(bbox.m_vMin), CoordToParent(bbox.m_vMax));
	}

	//------------------------------------------------------------------------------------------------
	// overridable for each generator to have their own options (e.g Forest Generator to not have an "Avoid Forests" option)
	protected void SetAvoidOptions()
	{
		s_ObstacleDetector.SetAvoidObjects(m_bAvoidObjects);
		s_ObstacleDetector.SetAvoidObjectsDetectionRadius(AVOID_OBJECTS_CHECK_RADIUS);
		s_ObstacleDetector.SetAvoidObjectsDetectionHeight(BBOX_CHECK_HEIGHT);
		s_ObstacleDetector.SetAvoidRoads(m_bAvoidRoads);
		s_ObstacleDetector.SetAvoidRivers(m_bAvoidRivers);
		s_ObstacleDetector.SetAvoidPowerLines(m_bAvoidPowerLines);
		s_ObstacleDetector.SetAvoidTracks(m_bAvoidTracks);
		s_ObstacleDetector.SetAvoidLakes(m_bAvoidLakes);
		s_ObstacleDetector.SetAvoidLand(m_iAvoidLandOrOcean == 1);
		s_ObstacleDetector.SetAvoidOcean(m_iAvoidLandOrOcean == 2);
//		s_ObstacleDetector.SetAvoidLandOceanOffset(m_fAvoidLandOceanOffset);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasObstacle(vector worldPos, array<IEntity> exclusionList = null)
	{
		if (!s_ObstacleDetector)
		{
			Print("HasObstacle() method requires obstacles info through RefreshObstacle() method first", LogLevel.ERROR);
			return true; // prevent placement by default
		}

		return s_ObstacleDetector.HasObstacle(worldPos, exclusionList);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasObstaclesList()
	{
		if (!s_ObstacleDetector)
		{
			Print("HasObstacle() method requires obstacles info through RefreshObstacle() method first", LogLevel.ERROR);
			return true; // prevent placement by default
		}

		return s_ObstacleDetector.HasObstaclesList();
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearObstacles()
	{
		if (s_ObstacleDetector)
			s_ObstacleDetector.ClearObstacles();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		super.OnIntersectingShapeChangedXZInternal(shapeEntitySrc, other, mins, maxes);

		if (!m_bRegenerateByObstacleChanges || !shapeEntitySrc || !other)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		IEntitySource childEntitySource;
		GeneratorBaseEntity generator;
		for (int i, childrenCount = other.GetNumChildren(); i < childrenCount; i++)
		{
			childEntitySource = other.GetChild(i);
			generator = GeneratorBaseEntity.Cast(worldEditorAPI.SourceToEntity(childEntitySource));
			if (!generator)
				continue;

			if ((m_bAvoidRoads && generator.IsInherited(RoadGeneratorEntity)) ||
				(m_bAvoidRivers && generator.IsInherited(RiverEntity)) ||
				(m_bAvoidPowerLines && generator.IsInherited(SCR_PowerlineGeneratorEntity)) ||
				(m_bAvoidTracks && generator.IsInherited(SCR_TrackGeneratorEntity)) ||
				// (m_bAvoidPrefabGenerators && generator.IsInherited(PrefabGeneratorEntity)) || // no clearance... yet
				(m_bAvoidLakes && generator.IsInherited(LakeGeneratorEntity)))
			{
				OnRegenerate();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// to be overridden for now
	protected void OnRegenerate();

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_AreaGeneratorBaseEntity(IEntitySource src, IEntity parent)
	{
		if (!_WB_GetEditorAPI()) // thumbnail generation
			return;

		if (!s_ObstacleDetector)
			s_ObstacleDetector = new SCR_ObstacleDetector();
	}
#endif
}
