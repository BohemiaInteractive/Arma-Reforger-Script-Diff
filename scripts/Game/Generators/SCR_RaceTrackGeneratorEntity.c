[EntityEditorProps(category: "GameScripted/Generators", description: "Race Track Generator", dynamicBox: true, visible: false)]
class SCR_RaceTrackGeneratorEntityClass : SCR_LineGeneratorBaseEntityClass
{
	[Attribute(defvalue: DEFAULT_RACE_ENTITY_PREFAB, desc: "Race entity Prefab to spawn if none present", params: "et")]
	ResourceName m_sRaceEntityPrefab;

	[Attribute(defvalue: DEFAULT_CHECKPOINT_START_ENTITY_PREFAB, desc: "Start Checkpoint entity Prefab", params: "et")]
	ResourceName m_sStartCheckpointEntityPrefab;

	[Attribute(defvalue: DEFAULT_CHECKPOINT_ROAD_ENTITY_PREFAB, desc: "Road Checkpoint entity Prefab", params: "et")]
	ResourceName m_sRoadCheckpointEntityPrefab;

	[Attribute(defvalue: DEFAULT_CHECKPOINT_END_ENTITY_PREFAB, desc: "End Checkpoint entity Prefab", params: "et")]
	ResourceName m_sEndCheckpointEntityPrefab;

	static const ResourceName DEFAULT_RACE_ENTITY_PREFAB = "{2D8C01F63D855C07}Prefabs/Props/RallyDLC/Race_Base.et";

	static const ResourceName DEFAULT_CHECKPOINT_START_ENTITY_PREFAB = "{44787A531EED4158}Prefabs/Props/RallyDLC/RaceCheckpointStart_Base.et";
	static const ResourceName DEFAULT_CHECKPOINT_ROAD_ENTITY_PREFAB = "{803FC166C2B1D8AC}Prefabs/Props/RallyDLC/RaceCheckpointRoad_Base.et";
	static const ResourceName DEFAULT_CHECKPOINT_END_ENTITY_PREFAB = "{1D8097344C93C0A3}Prefabs/Props/RallyDLC/RaceCheckpointEnd_Base.et";
}

class SCR_RaceTrackGeneratorEntity : SCR_LineGeneratorBaseEntity
{
	[Attribute(defvalue: "1", desc: "Draw debug shapes")]
	protected bool m_bDrawDebugShapes;

#ifdef WORKBENCH

	protected ref SCR_DebugShapeManager m_DebugShapeManager;

	protected static const string CHECKPOINT_ID_ATTRIBUTE_NAME = "m_iCheckpointID";
	protected static const string RACE_POINT_INFO_ATTRIBUTE_NAME = "m_raceStartingPoint";
	protected static const string RACE_POINT_INFO_OFFSET_ATTRIBUTE_NAME = "Offset";
	protected static const string RACE_POINT_INFO_CLASSNAME = string.ToString(PointInfo);
	protected static const vector START_RELATIVE_OFFSET = "0 0 -10";

	protected static const int DEBUG_COLOUR_START_POS = 0x33FFFFFF;			// white
	protected static const int DEBUG_COLOUR_START_POS_LINE = 0xFFFFFFFF;	// white
	protected static const int DEBUG_COLOUR_START = 0x8800FF00;				// green
	protected static const int DEBUG_COLOUR_MIDDLE = 0x88CCCC00;			// yellow
	protected static const int DEBUG_COLOUR_END = 0x88FF0000;				// red
	protected static const int DEBUG_COLOUR_LINE = 0xFFCC00CC;				// purple

	protected static const float DEBUG_START_POS_SIZE = 3;
	protected static const float DEBUG_CHECKPOINT_SIZE = 5;
	protected static const float DEBUG_CHECKPOINT_ARROW_SIZE = 15;

	protected static const string RACE_ENTITY_CLASSNAME = string.ToString(SCR_RaceEntity);
	protected static const string CHECKPOINT_START_ENTITY_CLASSNAME = string.ToString(SCR_RaceStartCheckpoint);
	protected static const string CHECKPOINT_ROAD_ENTITY_CLASSNAME = string.ToString(SCR_RaceRoadCheckpoint);
	protected static const string CHECKPOINT_END_ENTITY_CLASSNAME = string.ToString(SCR_RaceEndCheckpoint);

	//------------------------------------------------------------------------------------------------
	override protected void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		DrawDebugShapes(m_bDrawDebugShapes);
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_OnCreate(IEntitySource src)
	{
		super._WB_OnCreate(src);

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		if (m_ParentShapeSource)
			Generate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all children BUT the first-found SCR_RaceEntity (also delete its children checkpoints)
	override protected void DeleteAllChildren()
	{
		int count = m_Source.GetNumChildren();
		if (count < 1)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		array<IEntitySource> entitiesToDelete = {};
		IEntitySource raceEntitySource;
		BaseContainer child;
		for (int i = count - 1; i >= 0; --i)
		{
			child = m_Source.GetChild(i);
			if (!child)
				continue;

			if (!raceEntitySource && child.GetClassName().ToType() && child.GetClassName().ToType().IsInherited(SCR_RaceEntity))
			{
				raceEntitySource = child;
				for (int j = raceEntitySource.GetNumChildren() - 1; j >= 0; --j)
				{
					entitiesToDelete.Insert(raceEntitySource.GetChild(j));
				}
			}
			else
			{
				entitiesToDelete.Insert(child);
			}
		}

		if (raceEntitySource)
			delete worldEditorAPI.SourceToEntity(raceEntitySource); // avoids null checkpoint array endless issues

		worldEditorAPI.DeleteEntities(entitiesToDelete);
	}

	//------------------------------------------------------------------------------------------------
	protected void Generate()
	{
		if (!m_ParentShapeSource)
			return;

		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		if (worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		DeleteAllChildren();

		bool isClosed;
		m_ParentShapeSource.Get("IsClosed", isClosed);

		SCR_RaceTrackGeneratorEntityClass prefabData = SCR_RaceTrackGeneratorEntityClass.Cast(GetPrefabData());

		worldEditorAPI.BeginEditSequence(m_Source);

		IEntitySource raceEntitySource = GetOrCreateRaceEntity(worldEditorAPI, prefabData);
		if (!raceEntitySource) // used all possibilities really
		{
			Print(ClassName() + " - cannot create Race entity", LogLevel.ERROR);
			return;
		}

		int anchorsCount = m_ShapeNextPointHelper.GetAnchorPointIndices().Count(); // TODO: replace with GetAnchorPointsCount when available
		if (anchorsCount < 1)
		{
			Print("Parent shape has no points: " + Debug.GetEntityLinkString(this), LogLevel.ERROR);
			return;
		}

		// generate checkpoints
		Debug.BeginTimeMeasure();
		GenerateCheckpoints(worldEditorAPI, raceEntitySource, prefabData, anchorsCount, isClosed);
		Debug.EndTimeMeasure("Checkpoints generation");

		// PointInfo created -after- checkpoint creation to have start checkpoint's relative position
		SetPointInfoIfMissing(worldEditorAPI, raceEntitySource);

		worldEditorAPI.EndEditSequence(m_Source);

		DrawDebugShapes(m_bDrawDebugShapes);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] worldEditorAPI
	//! \param[in] prefabData
	protected IEntitySource GetOrCreateRaceEntity(notnull WorldEditorAPI worldEditorAPI, SCR_RaceTrackGeneratorEntityClass prefabData)
	{
		BaseContainer result;
		for (int i = m_Source.GetNumChildren() - 1; i >= 0; --i)
		{
			result = m_Source.GetChild(i);
			if (!result)
				continue;

			if (result.GetClassName().ToType() && result.GetClassName().ToType().IsInherited(SCR_RaceEntity))
				break;

			result = null;
		}

		if (!result)
		{
			result = CreateEntity(worldEditorAPI, m_Source, vector.Zero, vector.Zero, SCR_RaceTrackGeneratorEntityClass.DEFAULT_RACE_ENTITY_PREFAB, RACE_ENTITY_CLASSNAME);
			if (!result)
				return null;
		}

		vector coordsOrAngles;
		result.Get("coords", coordsOrAngles);
		if (coordsOrAngles != vector.Zero)
			worldEditorAPI.SetVariableValue(result, null, "coords", "0 0 0");

		result.Get("angles", coordsOrAngles);
		if (coordsOrAngles != vector.Zero)
			worldEditorAPI.SetVariableValue(result, null, "angles", "0 0 0");

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create start, middle and end (if needed) checkpoints
	//! \param[in] worldEditorAPI
	//! \param[in] raceEntitySource
	//! \param[in] prefabData
	//! \param[in] anchorsCount
	//! \param[in] isLoop whether or not the race is looped (goes back to Start) or has an End to be spawned
	protected void GenerateCheckpoints(
		notnull WorldEditorAPI worldEditorAPI,
		notnull IEntitySource raceEntitySource,
		SCR_RaceTrackGeneratorEntityClass prefabData,
		int anchorsCount,
		bool isLoop)
	{
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(SCR_RaceTrackGeneratorPointData);
		if (!pointDataMap)
			return;

		// generate start
		m_ShapeNextPointHelper.SetOnAnchor(0); // weird it is needed for non-vectorZero first point
		vector position = m_ShapeNextPointHelper.GetCurrentPosition();
		vector direction = m_ShapeNextPointHelper.GetCurrentDirection();
		SCR_RaceTrackGeneratorPointData pointData = SCR_RaceTrackGeneratorPointData.Cast(pointDataMap.Get(0));
		float yawOffset;
		if (pointData)
		{
			yawOffset = pointData.m_fCheckpointYawOffset;
			pointDataMap.Remove(0);
		}

		IEntitySource startCheckpointSource = CreateEntity(
			worldEditorAPI,
			raceEntitySource,
			position,
			direction,
			prefabData.m_sStartCheckpointEntityPrefab,
			SCR_RaceTrackGeneratorEntityClass.DEFAULT_CHECKPOINT_START_ENTITY_PREFAB,
			CHECKPOINT_START_ENTITY_CLASSNAME,
			yawOffset: yawOffset);

		if (!startCheckpointSource)
		{
			Print("No Start Checkpoint created", LogLevel.ERROR);
			return;
		}

		if (pointData && pointData.m_sPrefab && !CreateEntity(
			worldEditorAPI,
			startCheckpointSource,
			vector.Zero,
			vector.Zero,
			pointData.m_sPrefab,
			yawOffset: pointData.m_fPrefabYawOffset))
		{
			PrintFormat(
				"Cannot create Prefab %1 on start checkpoint at %2",
				pointData.m_sPrefab,
				Debug.GetPositionLinkString(CoordToParent(position), DEBUG_CHECKPOINT_SIZE),
				level: LogLevel.WARNING);
		}

		int lastIndex = anchorsCount - 1;

		IEntitySource endCheckpointSource;
		if (!isLoop)
		{
			// generate end
			if (!m_ShapeNextPointHelper.SetOnAnchor(lastIndex))
			{
				Print("Cannot set position on last anchor", LogLevel.ERROR);
				return;
			}

			pointData = SCR_RaceTrackGeneratorPointData.Cast(pointDataMap.Get(lastIndex));
			if (pointData)
			{
				yawOffset = pointData.m_fCheckpointYawOffset;
				pointDataMap.Remove(lastIndex);
			}
			else
			{
				yawOffset = 0;
			}

			position = m_ShapeNextPointHelper.GetCurrentPosition();
			direction = m_ShapeNextPointHelper.GetCurrentDirection();
			endCheckpointSource = CreateEntity(
				worldEditorAPI,
				raceEntitySource,
				position,
				direction,
				prefabData.m_sEndCheckpointEntityPrefab,
				SCR_RaceTrackGeneratorEntityClass.DEFAULT_CHECKPOINT_END_ENTITY_PREFAB,
				CHECKPOINT_END_ENTITY_CLASSNAME,
				yawOffset);

			if (!endCheckpointSource)
			{
				Print("No End Checkpoint created", LogLevel.ERROR);
				return;
			}

			if (pointData && pointData.m_sPrefab && !CreateEntity(
				worldEditorAPI,
				endCheckpointSource,
				vector.Zero,
				vector.Zero,
				pointData.m_sPrefab,
				yawOffset: pointData.m_fPrefabYawOffset))
			{
					PrintFormat(
						"Cannot create Prefab %1 on end checkpoint at %2",
						pointData.m_sPrefab,
						Debug.GetPositionLinkString(CoordToParent(position), DEBUG_CHECKPOINT_SIZE),
						level: LogLevel.WARNING);
			}
		}

		array<int> indices = SCR_MapHelperT<int, ShapePointDataScriptBase>.GetKeys(pointDataMap);
		indices.Sort();

		int checkpointIndex;
		IEntitySource entitySource;
		IEntitySource parent;

		// normal checkpoints
		foreach (int anchorIndex : indices)
		{
			pointData = SCR_RaceTrackGeneratorPointData.Cast(pointDataMap.Get(anchorIndex));
			if (!pointData)
				continue;

			if (!pointData.m_bIsCheckpoint && !pointData.m_sPrefab) // skip
				continue;

			if (!m_ShapeNextPointHelper.SetOnAnchor(anchorIndex))
				continue;

			position = m_ShapeNextPointHelper.GetCurrentPosition();
			direction = m_ShapeNextPointHelper.GetCurrentDirection();

			if (pointData.m_bIsCheckpoint)
			{
				entitySource = CreateEntity(
					worldEditorAPI,
					raceEntitySource,
					position,
					direction,
					prefabData.m_sRoadCheckpointEntityPrefab,
					SCR_RaceTrackGeneratorEntityClass.DEFAULT_CHECKPOINT_ROAD_ENTITY_PREFAB,
					CHECKPOINT_ROAD_ENTITY_CLASSNAME,
					pointData.m_fCheckpointYawOffset);

				if (entitySource)
					worldEditorAPI.SetVariableValue(entitySource, null, CHECKPOINT_ID_ATTRIBUTE_NAME, (++checkpointIndex).ToString());
			}
			else
			{
				entitySource = null;
			}

			if (pointData.m_sPrefab)
			{
				if (entitySource)
				{
					position = vector.Zero;
					direction = vector.Zero;
				}
				else
				{
					entitySource = raceEntitySource;
				}

				if (!CreateEntity(worldEditorAPI, entitySource, position, direction, pointData.m_sPrefab, yawOffset: pointData.m_fPrefabYawOffset))
				{
					PrintFormat(
						"Cannot create Prefab %1 on anchor %2 at %3",
						pointData.m_sPrefab,
						anchorIndex,
						Debug.GetPositionLinkString(CoordToParent(position), DEBUG_CHECKPOINT_SIZE),
						level: LogLevel.WARNING);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] worldEditorAPI
	//! \param[in] parentSource
	//! \param[in] position
	//! \param[in] direction NOT angles!
	//! \param[in] prefabData
	//! \param[in] resourceName
	//! \param[in] resourceNameFallback
	//! \param[in] className
	//! \param[in] yawOffset
	protected IEntitySource CreateEntity(
		notnull WorldEditorAPI worldEditorAPI,
		notnull IEntitySource parentSource,
		vector position,
		vector direction,
		ResourceName resourceName,
		ResourceName resourceNameFallback = "",
		string className = "",
		float yawOffset = 0)
	{
		int currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();
		direction = { 0, direction.ToYaw() + yawOffset, 0 }; // variable reuse (direction to angles)
		BaseContainer result;
		if (resourceName)
			result = worldEditorAPI.CreateEntity(resourceName, "", currentLayerId, parentSource, position, direction);

		if (!result && resourceNameFallback)
			result = worldEditorAPI.CreateEntity(resourceNameFallback, "", currentLayerId, parentSource, position, direction);

		if (!result && className)
			result = worldEditorAPI.CreateEntity(className, "", currentLayerId, parentSource, position, direction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPointInfoIfMissing(notnull WorldEditorAPI worldEditorAPI, notnull IEntitySource raceEntitySource)
	{
		if (raceEntitySource.GetObject(RACE_POINT_INFO_ATTRIBUTE_NAME))
			return;

		worldEditorAPI.CreateObjectVariableMember(raceEntitySource, null, RACE_POINT_INFO_ATTRIBUTE_NAME, RACE_POINT_INFO_CLASSNAME);
		IEntitySource startCheckpointSource;
		for (int i, count = raceEntitySource.GetNumChildren(); i < count; ++i)
		{
			startCheckpointSource = raceEntitySource.GetChild(i);
			if (startCheckpointSource.GetClassName().ToType() && startCheckpointSource.GetClassName().ToType().IsInherited(SCR_RaceStartCheckpoint))
				break;

			startCheckpointSource = null;
		}

		vector startPos;
		if (startCheckpointSource)
		{
			IEntity raceEntity = worldEditorAPI.SourceToEntity(raceEntitySource);
			IEntity startCheckpoint = worldEditorAPI.SourceToEntity(startCheckpointSource);
			startPos = raceEntity.CoordToLocal(startCheckpoint.CoordToParent(START_RELATIVE_OFFSET));
		}
		else
		{
			startPos = START_RELATIVE_OFFSET;
		}
		
		worldEditorAPI.SetVariableValue(
			raceEntitySource,
			{ new ContainerIdPathEntry(RACE_POINT_INFO_ATTRIBUTE_NAME) },
			RACE_POINT_INFO_OFFSET_ATTRIBUTE_NAME,
			startPos.ToString(false));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShapes(bool drawDebugShapes)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		if (!drawDebugShapes)
		{
			m_DebugShapeManager = null;
			return;
		}

		if (m_DebugShapeManager)
			m_DebugShapeManager.Clear();
		else
			m_DebugShapeManager = new SCR_DebugShapeManager();

		BaseContainer raceSource;
		for (int i, childrenCount = m_Source.GetNumChildren(); i < childrenCount; ++i)
		{
			raceSource = m_Source.GetChild(i);
			if (raceSource.GetClassName().ToType() && raceSource.GetClassName().ToType().IsInherited(SCR_RaceEntity))
				break;

			raceSource = null;
		}

		if (!raceSource)
			return;

		vector worldStartPos;
		BaseContainer pointInfo = raceSource.GetObject(RACE_POINT_INFO_ATTRIBUTE_NAME);
		if (pointInfo && pointInfo.Get(RACE_POINT_INFO_OFFSET_ATTRIBUTE_NAME, worldStartPos))
			worldStartPos = CoordToParent(worldStartPos); // given RaceEntity is at 0 0 0 coords/angles!

		BaseContainer checkpointSource;
		SCR_RaceCheckpoint startCheckpoint, endCheckpoint;
		array<SCR_RaceRoadCheckpoint> middleCheckpoints = {};

		for (int i, childrenCount = raceSource.GetNumChildren(); i < childrenCount; ++i)
		{
			checkpointSource = raceSource.GetChild(i);
			if (!checkpointSource)
				continue;

			typename typeName = checkpointSource.GetClassName().ToType();
			if (!typeName)
				continue;

			if (typeName.IsInherited(SCR_RaceStartCheckpoint))
				startCheckpoint = SCR_RaceStartCheckpoint.Cast(worldEditorAPI.SourceToEntity(checkpointSource));
			else
			if (typeName.IsInherited(SCR_RaceEndCheckpoint))
				endCheckpoint = SCR_RaceEndCheckpoint.Cast(worldEditorAPI.SourceToEntity(checkpointSource));
			else
			if (typeName.IsInherited(SCR_RaceRoadCheckpoint))
				middleCheckpoints.Insert(SCR_RaceRoadCheckpoint.Cast(worldEditorAPI.SourceToEntity(checkpointSource)));
		}

		middleCheckpoints.Sort();

		array<vector> linePoints = {};
		vector upOffset = vector.Up * DEBUG_CHECKPOINT_SIZE;

		vector startCheckpointPos;
		// start checkpoint
		if (startCheckpoint)
		{
			startCheckpointPos = startCheckpoint.GetOrigin();
			DrawCheckpointWallAndArrow(worldEditorAPI, startCheckpoint, DEBUG_COLOUR_START);
			linePoints.Insert(startCheckpointPos + upOffset);

			// starting pos
			if (worldStartPos != vector.Zero)
			{
				m_DebugShapeManager.AddSphere(worldStartPos, DEBUG_START_POS_SIZE, DEBUG_COLOUR_START_POS);
				m_DebugShapeManager.AddArrow(worldStartPos, startCheckpointPos, 0, DEBUG_COLOUR_START_POS_LINE);
			}
		}

		// middle checkpoints
		foreach (SCR_RaceRoadCheckpoint middleCheckpoint : middleCheckpoints)
		{
			DrawCheckpointWallAndArrow(worldEditorAPI, middleCheckpoint, DEBUG_COLOUR_MIDDLE);
			linePoints.Insert(middleCheckpoint.GetOrigin() + upOffset);
		}

		// end checkpoint
		if (endCheckpoint)
		{
			DrawCheckpointWallAndArrow(worldEditorAPI, endCheckpoint, DEBUG_COLOUR_END);
			linePoints.Insert(endCheckpoint.GetOrigin() + upOffset);
		}
		else
		if (startCheckpoint)
		{
			linePoints.Insert(startCheckpointPos + upOffset);
		}

		m_DebugShapeManager.AddPolyLine(linePoints, DEBUG_COLOUR_LINE);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawCheckpointWallAndArrow(notnull WorldEditorAPI worldEditorAPI, notnull SCR_RaceCheckpoint checkpoint, int colour)
	{
		m_DebugShapeManager.AddArrow(
			checkpoint.CoordToParent(vector.Zero),
			checkpoint.CoordToParent(vector.Forward * DEBUG_CHECKPOINT_ARROW_SIZE),
			0,
			colour);

		Physics physics = checkpoint.GetPhysics();
		if (!physics || physics.GetNumGeoms() < 1)
		{
			m_DebugShapeManager.AddSphere(checkpoint.GetOrigin(), DEBUG_CHECKPOINT_SIZE, colour);
		}
		else
		{
			vector min, max;
			physics.GetGeomDirectBounds(0, min, max);
	
			if (min == max)
			{
				m_DebugShapeManager.AddSphere(checkpoint.GetOrigin(), DEBUG_CHECKPOINT_SIZE, colour);
				return;
			}

			int steps = Math.Round(max[1] - min[1]);
			if (steps < 1)
				steps = 1;
	
			float slice = (max[1] - min[1]) / steps;
	
			vector posWithRightOffset = physics.GetGeomDirectPosition(0) + vector.Right * min[0];
			for (int i; i <= steps; ++i)
			{
				vector upOffset = vector.Up * (i * slice + min[1]);
				if (i != steps && upOffset[1] < 0) // the topmost is always drawn, just in case
					continue;

				m_DebugShapeManager.AddRectangle(
					checkpoint.CoordToParent(posWithRightOffset + upOffset),
					checkpoint.VectorToParent(vector.Forward),
					max[0] - min[0],
					max[2] - min[2],
					colour);
			}
		}
	}

#endif // WORKBENCH
}
