#ifdef WORKBENCH

//! Obstacle detector class used for example by SCR_GeneratorBaseEntity and ObjectBrushTool
//! Refreshing Obstacles grabs all the possible obstacles
//! HasObstacle checks depending on Avoid* settings
//! Refreshing Obstacles -then- setting AvoidX is possible as first all obstacles are stored then used or not in the detection
class SCR_ObstacleDetector
{
	protected bool m_bAvoidObjects;
	protected float m_fAvoidObjectsDetectionRadius;
	protected float m_fAvoidObjectsDetectionHeight;
	protected bool m_bAvoidRoads;
	protected bool m_bAvoidRivers;
	protected bool m_bAvoidPowerLines;
	protected bool m_bAvoidTracks;
	protected bool m_bAvoidForests;
	protected bool m_bAvoidLakes;
	protected bool m_bAvoidLand;
	protected bool m_bAvoidOcean;
	protected float m_fAvoidLandOceanOffset;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoRoads;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoRivers;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoPowerLines;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoTracks;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoLakes;
	protected ref array<ref SCR_ObstacleDetectorSplineInfo> m_aAvoidInfoForests;

	protected float m_fOceanLevel;
	protected ref TraceSphere m_AvoidObjectTraceSphere;

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidObjects
	void SetAvoidObjects(bool avoidObjects)
	{
		m_bAvoidObjects = avoidObjects;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] radius accepts 0..inf radius - a negative value will be considered to be zero
	void SetAvoidObjectsDetectionRadius(float radius)
	{
		if (radius < 0)
			radius = 0;

		m_fAvoidObjectsDetectionRadius = radius;
		if (m_AvoidObjectTraceSphere)
			m_AvoidObjectTraceSphere.Radius = m_fAvoidObjectsDetectionRadius;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] height
	void SetAvoidObjectsDetectionHeight(float height)
	{
		m_fAvoidObjectsDetectionHeight = height;
		if (m_AvoidObjectTraceSphere)
			m_AvoidObjectTraceSphere.End[1] = m_AvoidObjectTraceSphere.Start[1] + m_fAvoidObjectsDetectionHeight;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidRoads
	void SetAvoidRoads(bool avoidRoads)
	{
		m_bAvoidRoads = avoidRoads;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidRivers
	void SetAvoidRivers(bool avoidRivers)
	{
		m_bAvoidRivers = avoidRivers;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidPowerLines
	void SetAvoidPowerLines(bool avoidPowerLines)
	{
		m_bAvoidPowerLines = avoidPowerLines;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidTracks
	void SetAvoidTracks(bool avoidTracks)
	{
		m_bAvoidTracks = avoidTracks;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidForests
	void SetAvoidForests(bool avoidForests)
	{
		m_bAvoidForests = avoidForests;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidLakes
	void SetAvoidLakes(bool avoidLakes)
	{
		m_bAvoidLakes = avoidLakes;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidLand
	void SetAvoidLand(bool avoidLand)
	{
		m_bAvoidLand = avoidLand;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] avoidOcean
	void SetAvoidOcean(bool avoidOcean)
	{
		m_bAvoidOcean = avoidOcean;
	}

	//------------------------------------------------------------------------------------------------
	//! positive = above water, negative = below water
	//! \param[in] offset value in metres
	void SetAvoidLandOceanOffset(float offset)
	{
		m_fAvoidLandOceanOffset = offset;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldMin
	//! \param[in] worldMax
	// used by SCR_GeneratorBaseEntity.RefreshObstacles() (and RefreshObstaclesByWorld() which is unused)
	void RefreshObstaclesByAABB(vector worldMin, vector worldMax)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || !worldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(true, true);
		m_fOceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();
		worldEditorAPI.GetWorld().QueryEntitiesByAABB(worldMin, worldMax, AllSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//!
	// unused
	void RefreshObstaclesByWorld()
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || !worldEditorAPI.GetWorld())
			return;

		vector min, max;
		worldEditorAPI.GetWorld().GetBoundBox(min, max);
		RefreshObstaclesByAABB(min, max);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldPos
	//! \param[in] radius
	// used by ObjectBrushTool.CreateObjects()
	void RefreshRoadObstaclesBySphere(vector worldPos, float radius)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || !worldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(true, false);
		m_fOceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();
		worldEditorAPI.GetWorld().QueryEntitiesBySphere(worldPos, radius, RoadSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldPos
	//! \param[in] radius
	// used by ObjectBrushTool.OnMousePressEvent()
	void RefreshAreaObstaclesBySphere(vector worldPos, float radius)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || !worldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(false, true);
		m_fOceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();
		worldEditorAPI.GetWorld().QueryEntitiesBySphere(worldPos, radius, AreaSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//!
	// used by ObjectBrushTool.OnMousePressEvent()
	void RefreshAreaObstaclesByWorld()
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI || !worldEditorAPI.GetWorld())
			return;

		SetupObstacleArrays(false, true);
		m_fOceanLevel = worldEditorAPI.GetWorld().GetOceanBaseHeight();

		vector min, max;
		worldEditorAPI.GetWorld().GetBoundBox(min, max);
		worldEditorAPI.GetWorld().QueryEntitiesByAABB(min, max, AreaSplineQueryFilter);
	}

	//------------------------------------------------------------------------------------------------
	//! Detects obstacles based on current settings - a SCR_ObstacleDetector.RefreshObstaclesBy*() method must have been called earlier
	//! \param[in] worldPos the location to check - only the 2D position will be checked
	//! \param[in] exclusionList list of entities that should not be considered as obstacles
	//! \return true if an obstacle has been detected or on error, false otherwise
	bool HasObstacle(vector worldPos, array<IEntity> exclusionList = null)
	{
		if (!m_aAvoidInfoRoads) // a null array means obstacles were cleared or never initialised
		{
			Print("SCR_ObstacleDetector.HasObstacle() method requires obtaining obstacles info through SCR_ObstacleDetector.RefreshObstaclesBy*() method first", LogLevel.ERROR);
			return true; // prevent placement by default
		}

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
			return true;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return true;

		if (world.IsOcean())
		{
			if (m_bAvoidLand && m_bAvoidOcean)
				return true; // let's save some time

			float terrainY = world.GetSurfaceY(worldPos[0], worldPos[2]);

			if (m_bAvoidOcean && terrainY < m_fOceanLevel + m_fAvoidLandOceanOffset)
				return true;

			if (m_bAvoidLand && terrainY > m_fOceanLevel + m_fAvoidLandOceanOffset)
				return true;
		}

		if (m_bAvoidForests)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoForests)
			{
				if (Math2D.IsPointInPolygon(info.m_a2DPoints, worldPos[0], worldPos[2]))
					return true;
			}
		}

		if (m_bAvoidLakes)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoLakes)
			{
				if (worldPos[1] < info.m_fLakeSurfaceY && Math2D.IsPointInPolygon(info.m_a2DPoints, worldPos[0], worldPos[2]))
					return true;
			}
		}

		if (m_bAvoidRoads)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoRoads)
			{
				if (info.IsNearRoadSplineXZ(worldPos) && SCR_Math3D.IsPointWithinSplineDistanceXZ(info.m_aTesselatedPoints, worldPos, info.m_fClearance))
					return true;
			}
		}

		if (m_bAvoidRivers)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoRivers)
			{
				if (info.IsNearRoadSplineXZ(worldPos) && SCR_Math3D.IsPointWithinSplineDistanceXZ(info.m_aTesselatedPoints, worldPos, info.m_fClearance))
					return true;
			}
		}

		if (m_bAvoidPowerLines)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoPowerLines)
			{
				if (info.IsNearRoadSplineXZ(worldPos) && SCR_Math3D.IsPointWithinSplineDistanceXZ(info.m_aTesselatedPoints, worldPos, info.m_fClearance))
					return true;
			}
		}

		if (m_bAvoidTracks)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : m_aAvoidInfoTracks)
			{
				if (info.IsNearRoadSplineXZ(worldPos) && SCR_Math3D.IsPointWithinSplineDistanceXZ(info.m_aTesselatedPoints, worldPos, info.m_fClearance))
					return true;
			}
		}

		// tracing is the expensive part
		if (m_bAvoidObjects && (m_fAvoidObjectsDetectionHeight != 0 || m_fAvoidObjectsDetectionHeight > 0))
		{
			if (!m_AvoidObjectTraceSphere)
			{
				m_AvoidObjectTraceSphere = new TraceSphere();
				m_AvoidObjectTraceSphere.Radius = m_fAvoidObjectsDetectionRadius;
				m_AvoidObjectTraceSphere.LayerMask = EPhysicsLayerPresets.Main | EPhysicsLayerDefs.Water; // is Water required?
				m_AvoidObjectTraceSphere.Flags = TraceFlags.ENTS;
			}

			m_AvoidObjectTraceSphere.Start = worldPos;
			m_AvoidObjectTraceSphere.End = { worldPos[0], worldPos[1] + m_fAvoidObjectsDetectionHeight, worldPos[2] }; // move up

			if (exclusionList)
				m_AvoidObjectTraceSphere.ExcludeArray = exclusionList;
			else if (m_AvoidObjectTraceSphere.ExcludeArray)
				m_AvoidObjectTraceSphere.ExcludeArray.Clear();

			world.TraceMove(m_AvoidObjectTraceSphere, null);

			if (m_AvoidObjectTraceSphere.TraceEnt) // or TraceMove < 1
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if calling HasObstacle is possible
	//! \return true if a search for obstacles (using Refresh.*Obstacles.* methods) has been called (after init or ClearObstacles call)
	bool HasObstaclesList()
	{
		return m_aAvoidInfoRoads != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets up needed arrays
	//! \param[in] setupRoadSplines instanciates "road" spline arrays
	//! \param[in] setupAreaSplines instanciates area spline arrays
	protected void SetupObstacleArrays(bool setupRoadSplines, bool setupAreaSplines)
	{
		// roads
		if (setupRoadSplines || !m_aAvoidInfoRoads)
			m_aAvoidInfoRoads = {};

		if (setupRoadSplines || !m_aAvoidInfoRivers)
			m_aAvoidInfoRivers = {};

		if (setupRoadSplines || !m_aAvoidInfoPowerLines)
			m_aAvoidInfoPowerLines = {};

		if (setupRoadSplines || !m_aAvoidInfoTracks)
			m_aAvoidInfoTracks = {};

		// areas
		if (setupAreaSplines || !m_aAvoidInfoForests)
			m_aAvoidInfoForests = {};

		if (setupAreaSplines || !m_aAvoidInfoLakes)
			m_aAvoidInfoLakes = {};
	}

	//------------------------------------------------------------------------------------------------
	//! Get all obstacle's SCR_ObstacleDetectorSplineInfo
	// TODO: separate areas (Forest for now) vs splines
	//! \return null if obstacles are not initialised
	array<ref SCR_ObstacleDetectorSplineInfo> GetObstacles()
	{
		if (!m_aAvoidInfoRoads) // a null array means obstacles were cleared or never initialised
			return null;

		array<ref SCR_ObstacleDetectorSplineInfo> result = {};
		array<array<ref SCR_ObstacleDetectorSplineInfo>> arrays = {
			m_aAvoidInfoRoads,
			m_aAvoidInfoRivers,
			m_aAvoidInfoPowerLines,
			m_aAvoidInfoTracks,
			m_aAvoidInfoForests,
			m_aAvoidInfoLakes,
		};

		foreach (array<ref SCR_ObstacleDetectorSplineInfo> arr : arrays)
		{
			foreach (SCR_ObstacleDetectorSplineInfo info : arr)
			{
				result.Insert(info);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Empties obstacles information - a SCR_ObstacleDetector.RefreshObstaclesBy*() method must be called to use HasObstacle again
	void ClearObstacles()
	{
		m_aAvoidInfoRoads = null;
		m_aAvoidInfoRivers = null;
		m_aAvoidInfoPowerLines = null;
		m_aAvoidInfoTracks = null;
		m_aAvoidInfoForests = null;
		m_aAvoidInfoLakes = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	//! AllSplineQueryFilter, RoadSplineQueryFilter, AreaSplineQueryFilter call it
	//! \param[in] getRoadSplines get "road-like" splines - roads, rivers, powerlines
	//! \param[in] getAreaSplines get area splines - lakes, forests
	//! \return true when done
	protected bool BaseSplineQueryFilter(IEntity entity, bool getRoadSplines, bool getAreaSplines)
	{
		if (!getRoadSplines && !getAreaSplines)
			return true;

		ShapeEntity shapeEntity = ShapeEntity.Cast(entity);
		if (!shapeEntity)
			return true;

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
			return true;

		IEntitySource shapeSource = worldEditorAPI.EntityToSource(shapeEntity);
		if (!shapeSource)
			return true;

		IEntitySource generatorSource;
		typename generatorTypename;
		bool isLake;
		for (int i, count = shapeSource.GetNumChildren(); i < count; ++i)
		{
			generatorSource = shapeSource.GetChild(i);
			generatorTypename = generatorSource.GetClassName().ToType();
			if (!generatorTypename)
				continue;

			if (getRoadSplines)
			{
				if (generatorTypename.IsInherited(RoadGeneratorEntity))
				{
					m_aAvoidInfoRoads.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}

				if (generatorTypename.IsInherited(RiverEntity))
				{
					m_aAvoidInfoRivers.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}

				if (generatorTypename.IsInherited(SCR_PowerlineGeneratorEntity))
				{
					m_aAvoidInfoPowerLines.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}

				if (generatorTypename.IsInherited(SCR_TrackGeneratorEntity))
				{
					m_aAvoidInfoTracks.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}
			}

			if (getAreaSplines)
			{
				if (generatorTypename.IsInherited(ForestGeneratorEntity))
				{
					m_aAvoidInfoForests.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}

				if (generatorTypename.IsInherited(LakeGeneratorEntity) && generatorSource.Get("m_bIsLake", isLake) && isLake)
				{
					m_aAvoidInfoLakes.Insert(new SCR_ObstacleDetectorSplineInfo(shapeSource, shapeEntity, generatorSource));
					break;
				}
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool AllSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, true, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool RoadSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, true, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills m_aAvoidInfo* arrays from RefreshObstaclesBy* request for HasObstacle usage
	protected bool AreaSplineQueryFilter(IEntity entity)
	{
		return BaseSplineQueryFilter(entity, false, true);
	}
}

class SCR_ObstacleDetectorSplineInfo
{
	float m_fClearance;
	float m_fLakeSurfaceY;
	ref array<float> m_a2DPoints;			//!< World coordinates. Lakes only
	ref array<vector> m_aTesselatedPoints;	//!< World coordinates. Roads/Rivers/Powerlines/Tracks
	ref array<vector> m_aMinsWithClearance;
	ref array<vector> m_aMaxsWithClearance;
	vector m_vMinWithClearance;
	vector m_vMaxWithClearance;

	//------------------------------------------------------------------------------------------------
	//! Non-expensive approximation check if the position is near a road-like spline and its clearance
	//! \return true if close, false if not
	bool IsNearRoadSplineXZ(vector point)
	{
		float x = point[0];
		float z = point[2];

		if (x < m_vMinWithClearance[0] || m_vMaxWithClearance[0] < x ||
			z < m_vMinWithClearance[2] || m_vMaxWithClearance[2] < z)
			return false; // nowhere near

		vector max;
		foreach (int index, vector min : m_aMinsWithClearance)
		{
			max = m_aMaxsWithClearance[index];
			if (min[0] <= x && x <= max[0])
			{
				if (min[2] <= z && z <= max[2])
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in,out] mins needed to obtain min, -clearance is also added to all the mins
	//! \param[in,out] maxs needed to obtain max, +clearance is also added to all the maxs
	//! \param[out] min the bottom-left corner - clearance
	//! \param[out] max the top-right corner + clearance
	// minor performance gains to add clearance to mins/maxs but gains nonetheless (e.g 500ms on 20s calculation so ~= 2.5% on big area)
	protected void GetAndSetMinMaxWithClearance(inout notnull array<vector> mins, inout notnull array<vector> maxs, out vector min, out vector max)
	{
		vector vectorClearance = { m_fClearance, 0, m_fClearance };
		int minsCount = mins.Count();
		int maxsCount = maxs.Count();

		if (minsCount < 1 || maxsCount < 1 || minsCount != maxsCount)
		{
			min = -vectorClearance;
			max = vectorClearance;
			return;
		}

		min = mins[0];
		for (int i; i < minsCount; i++)
		{
			mins[i] = mins[i] - vectorClearance;
			for (int j; j < 3; j++)
			{
				if (min[j] > mins[i][j])
					min[j] = mins[i][j];
			}
		}

		max = maxs[0];
		for (int i; i < maxsCount; i++)
		{
			maxs[i] = maxs[i] + vectorClearance;
			for (int j; j < 3; j++)
			{
				if (max[j] < maxs[i][j])
					max[j] = maxs[i][j];
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get shape's 2D and/or 3D points in WORLD coordinates
	//! \param[in] shapeEntity
	//! \param[out] pos2D can be null and won't be filled
	//! \param[out] pos3D can be null and won't be filled
	//! \return number of points, or -1 in case of no array to fill
	protected static int GetPoints2D3D(notnull ShapeEntity shapeEntity, out array<float> pos2D, out array<vector> pos3D)
	{
		if (!pos2D && !pos3D)
			return -1;

		array<vector> points3D = {};
		shapeEntity.GetPointsPositions(points3D);

		if (pos2D)
			pos2D.Clear();

		if (pos3D)
			pos3D.Clear();

		foreach (vector point : points3D)
		{
			point = shapeEntity.CoordToParent(point);
			if (pos2D)
			{
				pos2D.Insert(point[0]);
				pos2D.Insert(point[2]);
			}

			if (pos3D)
				pos3D.Insert(point);
		}

		return points3D.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetTesselatedWorldPoints(notnull ShapeEntity shapeEntity)
	{
		array<vector> result = {};

		shapeEntity.GenerateTesselatedShape(result);
		for (int i, count = result.Count(); i < count; i++)
		{
			result[i] = shapeEntity.CoordToParent(result[i]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] shapeEntitySource
	//! \param[in] shapeEntity
	//! \param[in] generatorSource
	void SCR_ObstacleDetectorSplineInfo(notnull IEntitySource shapeEntitySource, notnull ShapeEntity shapeEntity, notnull IEntitySource generatorSource)
	{
		typename generatorTypeName = generatorSource.GetClassName().ToType();
		if (!generatorTypeName)
			return;

		m_aMinsWithClearance = {};
		m_aMaxsWithClearance = {};

		/*
			ROAD
		*/
		if (generatorTypeName.IsInherited(RoadGeneratorEntity))
		{
			generatorSource.Get("RoadClearance", m_fClearance);
			m_aTesselatedPoints = GetTesselatedWorldPoints(shapeEntity);
			shapeEntity.GetAllInfluenceBBoxes(shapeEntitySource, m_aMinsWithClearance, m_aMaxsWithClearance);
			GetAndSetMinMaxWithClearance(m_aMinsWithClearance, m_aMaxsWithClearance, m_vMinWithClearance, m_vMaxWithClearance);
			return;
		}

		/*
			RIVER,
			POWER LINE
		*/
		if (generatorTypeName.IsInherited(RiverEntity) ||
			generatorTypeName.IsInherited(SCR_PowerlineGeneratorEntity))
		{
			generatorSource.Get("Clearance", m_fClearance);
			m_aTesselatedPoints = GetTesselatedWorldPoints(shapeEntity);
			shapeEntity.GetAllInfluenceBBoxes(shapeEntitySource, m_aMinsWithClearance, m_aMaxsWithClearance);
			GetAndSetMinMaxWithClearance(m_aMinsWithClearance, m_aMaxsWithClearance, m_vMinWithClearance, m_vMaxWithClearance);
			return;
		}

		/*
			TRACK
		*/
		if (generatorTypeName.IsInherited(SCR_TrackGeneratorEntity))
		{
			generatorSource.Get("m_fClearance", m_fClearance);
			m_aTesselatedPoints = GetTesselatedWorldPoints(shapeEntity);
			shapeEntity.GetAllInfluenceBBoxes(shapeEntitySource, m_aMinsWithClearance, m_aMaxsWithClearance);
			GetAndSetMinMaxWithClearance(m_aMinsWithClearance, m_aMaxsWithClearance, m_vMinWithClearance, m_vMaxWithClearance);
			return;
		}

		/*
			FOREST
		*/
		if (generatorTypeName.IsInherited(ForestGeneratorEntity))
		{
			m_a2DPoints = {};
			GetPoints2D3D(shapeEntity, m_a2DPoints, null);
			return;
		}

		/*
			LAKE
		*/
		if (generatorTypeName.IsInherited(LakeGeneratorEntity))
		{
			m_a2DPoints = {};
			array<vector> a3DPoints = {};
			GetPoints2D3D(shapeEntity, m_a2DPoints, a3DPoints);

			bool flattenByBottomPlane;
			generatorSource.Get("m_bFlattenByBottomPlane", flattenByBottomPlane);

			if (a3DPoints.IsEmpty())
				return;

			// all this to be replaced with a later LakeGeneratorEntity method
			float minOrMaxY = a3DPoints[0][1];
			foreach (vector a3DPoint : a3DPoints)
			{
				if (flattenByBottomPlane)
				{
					if (a3DPoint[1] < minOrMaxY)
						minOrMaxY = a3DPoint[1];
				}
				else
				{
					if (a3DPoint[1] > minOrMaxY)
						minOrMaxY = a3DPoint[1];
				}
			}

			m_fLakeSurfaceY = minOrMaxY;
			return;
		}
	}
}

#endif // WORKBENCH
