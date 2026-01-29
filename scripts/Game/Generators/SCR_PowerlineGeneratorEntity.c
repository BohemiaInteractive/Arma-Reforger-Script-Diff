[EntityEditorProps(category: "GameScripted/Generators", description: "Power Line Generator", dynamicBox: true, visible: false)]
class SCR_PowerlineGeneratorEntityClass : SCR_LineTerrainShaperGeneratorEntityClass
{
}

//! SLOT SYSTEM:
//!
//! Poles have slot groups of a certain type
//! (Low Voltage, High Voltage, Ultra High Voltage, Telephone, etc - see SCR_EPoleCableType)\n
//! - only distance to slot group is considered - same line factor is not\n
//! - only one connection per cable type is possible as of now (closest slots will connect)
//! e.g closest LV to closest LV, closest Telephone to closest Telephone

// TODO: move most of the properties to PrefabData (https://community.bistudio.com/wiki/Arma_Reforger:Prefab_Data)
class SCR_PowerlineGeneratorEntity : SCR_LineTerrainShaperGeneratorEntity
{
	/*
		Pole Setup
	*/

	[Attribute(defvalue: "30", desc: "How far should the poles be from each other", params: "1 " + MAX_CABLE_LENGTH, category: "Pole Setup")]
	protected float m_fDistance;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_DefaultPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_StartPole;

	[Attribute(params: "et", category: "Pole Setup")]
	protected ResourceName m_EndPole;

	[Attribute(params: "et", category: "Pole Setup", desc: "The junction pole used when no Prefab is defined in shape's point data")]
	protected ResourceName m_DefaultJunctionPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawStartPole;

	[Attribute(defvalue: "0", category: "Pole Setup")]
	protected bool m_bRotate180DegreeYawEndPole;

	[Attribute(defvalue: "0", desc: "Expected empty space around the poles line by other generators", params: "0 100 1", category: "Pole Setup")]
	protected float Clearance; //!< Used by Obstacle Detector

	/*
		Randomisation
	*/

	[Attribute(defvalue: "0", desc: "Maximum random yaw angle (×2, -value to +value) - only applied to default poles", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomYawAngle;

	[Attribute(defvalue: "0", desc: "Maximum random pitch angle (×2, -value to +value)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomPitchAngle;

	[Attribute(defvalue: "0", desc: "Maximum random roll angle (×2, -value to +value)", params: "0 180 1", category: "Randomisation")]
	protected float m_fRandomRollAngle;

	[Attribute(defvalue: "0", desc: "Apply pitch and roll randomisation to default poles", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollDefault;

	[Attribute(defvalue: "0", desc: "Apply randomisation to start pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollStart;

	[Attribute(defvalue: "0", desc: "Apply randomisation to end pole", category: "Randomisation")]
	protected bool m_bApplyPitchAndRollEnd;

	/*
		Cables
	*/

	[Attribute(desc: "Cable types-emat correspondence list", category: "Cables")]
	protected ref array<ref SCR_PoleCable> m_aCables;

	[Attribute(defvalue: DEFAULT_CABLE_PREFAB, desc: "Cable Prefab", params: "et class=" + POWER_LINE_CLASS, category: "Cables")]
	protected ResourceName m_sPowerLinePrefab;

	[Attribute(defvalue: "{836210B285A81785}Assets/Structures/Infrastructure/Power/Powerlines/data/default_powerline_wire.emat", desc: "Default cable material if none above match", params: "emat", category: "Cables")]
	protected ResourceName m_PowerlineMaterial; // m_s

	/*
		Debug
	*/

	[Attribute(defvalue: "0", category: "Debug")]
	protected bool m_bDrawDebugShapes;

	protected static const string POWER_LINE_CLASS = "PowerlineEntity";
	protected static const ResourceName DEFAULT_CABLE_PREFAB = "{613763D114E7CD1C}Prefabs/WEGenerators/Powerline/Powerline.et";
	protected static const int MAX_CABLE_LENGTH = 500; // error when cable > 500

#ifdef WORKBENCH

	protected static ref SCR_DebugShapeManager s_DebugShapeManager; // static because SetSeed modifies the EntitySource, re-creating the entity

	// TODO: remove from static
	//! <generatorSource, <objectSource, objectPosition>>
	protected static ref map<IEntitySource, ref map<IEntitySource, vector>> s_mGeneratorAndPolesPlusParentWorldPosMap = new map<IEntitySource, ref map<IEntitySource, vector>>();

	protected static const float JUNCTION_DISTANCEXZ = 0.1;
	protected static const float JUNCTION_DISTANCEXZ_SQ = JUNCTION_DISTANCEXZ * JUNCTION_DISTANCEXZ;
	protected static const float MIN_CABLE_LENGTH_SQ = 0.01;	// 0.1 * 0.1
	protected static const float MAX_CABLE_LENGTH_SQ = MAX_CABLE_LENGTH * MAX_CABLE_LENGTH;
	protected static const float GENERATOR_BBOX_TOLERANCE = 5;	// in metres

	protected static const float POLE_SAFEZONE_DISTANCE = 1.0;	// any pole closer than that will be dropped

	// DEBUG variables (color, pos, size)

	protected static const int DEBUG_POLE_COLOUR_POS = Color.BLUE & 0x00FFFFFF | 0x88000000;
	protected static const int DEBUG_POLE_COLOUR_ERROR = Color.RED & 0x00FFFFFF | 0x88000000;
	protected static const float DEBUG_POLE_SIZE = 1;
	protected static const vector DEBUG_POLE_POS = "0 5 0";

	protected static const int DEBUG_SLOT_COLOUR_POS = Color.DARK_GREEN & 0x00FFFFFF | 0x88000000;
	// protected static const int DEBUG_SLOT_COLOUR_ERROR = Color.RED & 0x00FFFFFF | 0x88000000;
	protected static const int DEBUG_SLOT_COLOUR_UNUSED = Color.ORANGE & 0x00FFFFFF | 0x88000000;
	protected static const float DEBUG_SLOT_SIZE = 0.1;

	protected static const int DEBUG_CABLE_LENGTH_COLOUR_ERROR_SPHERE = Color.RED & 0x00FFFFFF | 0xAA000000;
	protected static const int DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE = Color.RED;
	protected static const vector DEBUG_CABLE_LENGTH_POS = "0 3 0";
	protected static const float DEBUG_CABLE_LENGTH_POS_MAX_DIST = DEBUG_CABLE_LENGTH_POS.Length();
	protected static const float DEBUG_CABLE_LENGTH_SIZE = 0.5;

	protected static const int DEBUG_CABLE_COLOUR_POS = Color.YELLOW & 0x00FFFFFF | 0x88000000;	//!< virtual power cables

	//------------------------------------------------------------------------------------------------
	//! Entry point to all current line's generation (and connected lines's fixes)
	protected void Generate()
	{
		if (!m_ParentShapeSource)
			return;

		if (!m_bEnableGeneration)
		{
			Print("Power line generation is disabled for this shape (beware of junction issues with other lines) - tick it back on before saving", LogLevel.NORMAL);
			return;
		}

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring() || !worldEditorAPI.AreGeneratorEventsEnabled())
			return;

		bool manageEntityAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEntityAction)
			worldEditorAPI.BeginEntityAction();

		SetSeed();

		int childrenCount = m_Source.GetNumChildren();
		if (childrenCount > 0)
		{
			Debug.BeginTimeMeasure();
			DeleteAllChildren();
			Debug.EndTimeMeasure("Delete " + childrenCount + " children");
		}

		if (m_bDrawDebugShapes)
		{
			if (s_DebugShapeManager)
				s_DebugShapeManager.Clear();
			else
				s_DebugShapeManager = new SCR_DebugShapeManager();
		}
		else
		{
			s_DebugShapeManager = null;
		}

		// get on anchor generators
		array<vector> worldAnchorPoints = GetWorldAnchorPoints(m_ParentShapeSource);
		int worldAnchorPointsCount = worldAnchorPoints.Count();
		if (worldAnchorPointsCount < 2)
			return;

		map<IEntitySource, IEntitySource> existingJunctions = new map<IEntitySource, IEntitySource>();

		IEntitySource existingObjectSource, existingPoleSource;
		if (GetObjectAndPoleSourceAt(worldAnchorPoints[0], existingObjectSource, existingPoleSource))
			existingJunctions.Insert(existingObjectSource, existingPoleSource);

		if (GetObjectAndPoleSourceAt(worldAnchorPoints[worldAnchorPointsCount - 1], existingObjectSource, existingPoleSource))
			existingJunctions.Insert(existingObjectSource, existingPoleSource);

		s_mGeneratorAndPolesPlusParentWorldPosMap.Clear();
		foreach (IEntitySource objectSource, IEntitySource poleSource : existingJunctions)
		{
			IEntitySource otherGenerator = objectSource.GetParent();
			if (!otherGenerator)
				continue;

			map<IEntitySource, vector> otherJunctionPositions;
			if (!s_mGeneratorAndPolesPlusParentWorldPosMap.Find(otherGenerator, otherJunctionPositions))
			{
				otherJunctionPositions = new map<IEntitySource, vector>();
				s_mGeneratorAndPolesPlusParentWorldPosMap.Insert(otherGenerator, otherJunctionPositions);
			}

			otherJunctionPositions.Insert(poleSource, worldEditorAPI.SourceToEntity(objectSource).GetOrigin());
		}

		Debug.BeginTimeMeasure();
		map<IEntitySource, IEntitySource> createdJunctions = GenerateJunctions();
		Debug.EndTimeMeasure(createdJunctions.Count().ToString() + " junctions generation");

		map<IEntitySource, vector> junctionPositions = new map<IEntitySource, vector>();
		foreach (IEntitySource objectSource, IEntitySource poleSource : createdJunctions)
		{
			junctionPositions.Insert(poleSource, worldEditorAPI.SourceToEntity(objectSource).GetOrigin());
		}

		s_mGeneratorAndPolesPlusParentWorldPosMap.Insert(m_Source, junctionPositions);

		Debug.BeginTimeMeasure();
		GeneratePoles();
		Debug.EndTimeMeasure("Poles generation");

		ReconnectOtherGenerators(m_Source, createdJunctions);

		if (manageEntityAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetObjectAndPoleSourceAt(vector worldPos, out IEntitySource parentSource, out IEntitySource poleSource)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		set<IEntity> entities = new set<IEntity>();
		worldEditorAPI.GetWorld().QueryEntitiesBySphere(worldPos, JUNCTION_DISTANCEXZ, entities.Insert);

		foreach (IEntity entity : entities)
		{
			if (SCR_PowerPole.Cast(entity))
			{
				parentSource = worldEditorAPI.EntityToSource(entity);
				poleSource = parentSource;
				return true;
			}

			IEntity child = entity.GetChildren();
			while (child)
			{
				if (SCR_PowerPole.Cast(child))
				{
					parentSource = worldEditorAPI.EntityToSource(entity);
					poleSource = worldEditorAPI.EntityToSource(child);
					return true;
				}

				child = child.GetSibling();
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetFirstCablePoleEntityInEntityOrChildren(notnull IEntity entity)
	{
		if (SCR_PowerPole.Cast(entity))
			return entity;

		IEntity child = entity.GetChildren();
		while (child)
		{
			if (SCR_PowerPole.Cast(entity))
				return entity;

			IEntity result = GetFirstCablePoleEntityInEntityOrChildren(child);
			if (result)
				return result;

			child = child.GetSibling();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] generator the generator that generated its poles
	//! \param[in] junctions the junctions on its line
	protected void ReconnectOtherGenerators(notnull IEntitySource generator, notnull map<IEntitySource, IEntitySource> junctions)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		foreach (IEntitySource parentSource, IEntitySource poleSource : junctions)
		{
			vector parentPos = worldEditorAPI.SourceToEntity(parentSource).GetOrigin();

			set<IEntity> entitiesSet = new set<IEntity>();
			if (!world.QueryEntitiesBySphere(parentPos, JUNCTION_DISTANCEXZ, entitiesSet.Insert))
				continue; // huh?

			set<IEntitySource> otherGeneratorSources = new set<IEntitySource>();
			foreach (IEntity entity : entitiesSet)
			{
				if (!SCR_PowerlineGeneratorEntity.Cast(entity))
					continue;

				IEntitySource otherGeneratorSource = worldEditorAPI.EntityToSource(entity);
				if (!otherGeneratorSource)
					continue;

				if (otherGeneratorSource == generator)
					continue;

				IEntitySource shapeSource = otherGeneratorSource.GetParent();
				if (!shapeSource)
					continue;

				ShapeEntity parentShape = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeSource));
				if (!parentShape)
					continue;

				array<vector> shapePoints = GetWorldAnchorPoints(shapeSource);
				if (!shapePoints)
					continue;

				int shapesPointCount = shapePoints.Count();
				if (shapesPointCount < 2)
					continue;

				if (vector.DistanceSqXZ(shapePoints[0], parentPos) < JUNCTION_DISTANCEXZ_SQ
					|| vector.DistanceSqXZ(shapePoints[shapesPointCount - 1], parentPos) < JUNCTION_DISTANCEXZ_SQ)
					otherGeneratorSources.Insert(otherGeneratorSource);
			}

			foreach (IEntitySource otherGeneratorSource : otherGeneratorSources)
			{
				if (!ReconnectGeneratorToPole(otherGeneratorSource, poleSource))
					Print("Cannot reconnect generator to pole " + Debug.GetEntityLinkString(worldEditorAPI.SourceToEntity(poleSource)), LogLevel.WARNING);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reconnect a line ending to the provided pole
	//! - Does support start or end of line connecting to another line junction
	//! - Does NOT support middle of a line connecting to another line junction
	//! \param[in] generatorSource the line to reconnect
	//! \param[in] otherLinePoleSource the pole to which reconnect said line
	//! \return true on success, false otherwise
	protected bool ReconnectGeneratorToPole(notnull IEntitySource generatorSource, notnull IEntitySource otherLinePoleSource)
	{
		BaseContainer parentShape = generatorSource.GetParent();
		if (!parentShape || !parentShape.GetClassName().ToType() || !parentShape.GetClassName().ToType().IsInherited(ShapeEntity))
			return false;

		int childrenCount = generatorSource.GetNumChildren();
		if (childrenCount < 1)
			return false;

		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return false;

		SCR_PowerPole otherLinePole = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(otherLinePoleSource));
		if (!otherLinePole)
			return false;

		IEntitySource localPoleSource;
		if (childrenCount == 1)
		{
			localPoleSource = generatorSource.GetChild(0);
			if (!localPoleSource
				|| !localPoleSource.GetClassName().ToType()
				|| !localPoleSource.GetClassName().ToType().IsInherited(SCR_PowerPole))
				return false;
		}
		else	// can be either of the extremities OR a central point (why...)
		{		// second case is NOT covered - use it properly or don't use it
			array<vector> worldAnchorPoints = GetWorldAnchorPoints(parentShape);
			int worldAnchorPointsCount = worldAnchorPoints.Count();
			if (worldAnchorPointsCount < 2)
				return false;

			bool fromStart = vector.DistanceSqXZ(worldAnchorPoints[0], otherLinePole.GetOrigin()) < JUNCTION_DISTANCEXZ_SQ;
			localPoleSource = GetFirstCablePoleFromExtremity(generatorSource, fromStart);
			if (!localPoleSource)
				return false;

			array<IEntitySource> cablesToDelete = GetCablesBetweenPoleSources(localPoleSource, otherLinePoleSource);
			worldEditorAPI.DeleteEntities(cablesToDelete);
		}

		return CreatePowerLines(generatorSource, localPoleSource, otherLinePoleSource) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] generatorSource
	//! \param[in] fromStart true to get from start, false otherwise
	//! \return can return null (if no matching children)
	protected static IEntitySource GetFirstCablePoleFromExtremity(notnull IEntitySource generatorSource, bool fromStart)
	{
		int count = generatorSource.GetNumChildren();
		if (count < 1)
			return null;

		int start, increment;
		if (fromStart)
		{
//			start = 0;
			increment = 1;
		}
		else
		{
			start = count - 1;
			increment = -1;
		}

		for (; start >= 0 && start < count; start += increment)
		{
			IEntitySource entitySource = generatorSource.GetChild(start);
			if (entitySource && entitySource.GetClassName().ToType() && entitySource.GetClassName().ToType().IsInherited(SCR_PowerPole))
				return entitySource;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] generatorPoleSource pole source from generator in which cables must be found
	//! \param[in] otherPoleSource other pole source (can be from same generator or not)
	//! \return all poles's common cables or null on error
	protected static array<IEntitySource> GetCablesBetweenPoleSources(notnull IEntitySource generatorPoleSource, notnull IEntitySource otherPoleSource)
	{
		array<IEntitySource> result = {};
		set<IEntity> entitiesAroundGeneratorPole = new set<IEntity>(); // using set to use directly in QueryEntitiesByAABB
		set<IEntity> entitiesAroundOtherPole = new set<IEntity>();

		vector mins, maxes;
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		BaseWorld world = worldEditorAPI.GetWorld();

		worldEditorAPI.SourceToEntity(generatorPoleSource).GetWorldBounds(mins, maxes);
		if (mins != maxes && !world.QueryEntitiesByAABB(mins, maxes, entitiesAroundGeneratorPole.Insert))
			return null;

		worldEditorAPI.SourceToEntity(otherPoleSource).GetWorldBounds(mins, maxes);
		if (mins != maxes && !world.QueryEntitiesByAABB(mins, maxes, entitiesAroundOtherPole.Insert))
			return null;

		IEntitySource generatorSource = generatorPoleSource.GetParent();
		foreach (IEntity entityAroundGeneratorPole : entitiesAroundGeneratorPole)
		{
			if (!entitiesAroundOtherPole.Contains(entityAroundGeneratorPole))
				continue; // not in common

			if (!entityAroundGeneratorPole.Type() || !entityAroundGeneratorPole.Type().IsInherited(PowerlineEntity))
				continue; // not a cable

			IEntitySource entitySource = worldEditorAPI.EntityToSource(entityAroundGeneratorPole);
			if (!entitySource || entitySource.GetParent() != generatorSource)
				continue; // not a generator cable

			result.Insert(entitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Generate junctions, happening before generating poles
	//! \return map of <parentSource, poleSource> (that can be the same EntitySource), or null on error
	protected map<IEntitySource, IEntitySource> GenerateJunctions()
	{
		array<vector> anchorPoints = m_ShapeNextPointHelper.GetAnchorPoints();
		int anchorPointsCountMinus1 = anchorPoints.Count() - 1;

		if (anchorPointsCountMinus1 < 1)
			return new map<IEntitySource, IEntitySource>();

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();

		map<IEntitySource, IEntitySource> result = new map<IEntitySource, IEntitySource>();

		SCR_PowerlineGeneratorPointData pointData;
		SCR_PowerlineGeneratorJunctionData junctionData;
		IEntitySource objectSource;
		IEntitySource poleSource;
		map<int, ref ShapePointDataScriptBase> pointDataMap = GetFirstPointDataMap(SCR_PowerlineGeneratorPointData);
		foreach (int i, vector anchorPoint : anchorPoints)
		{
			pointData = SCR_PowerlineGeneratorPointData.Cast(pointDataMap.Get(i));
			if (!pointData)
				continue;

			junctionData = pointData.m_JunctionData;
			if (!junctionData)
				continue;

			ResourceName junctionResourceName = junctionData.m_sJunctionPrefab;
			if (junctionResourceName.IsEmpty())
			{
				if (m_DefaultJunctionPole.IsEmpty())
					continue;

				junctionResourceName = m_DefaultJunctionPole;
			}

			vector vectorDir;
			if (i == 0)
			{
				vectorDir = vector.Direction(anchorPoint, anchorPoints[1]);
				if (m_bRotate180DegreeYawStartPole)
					vectorDir = -vectorDir;
			}
			else if (i == anchorPointsCountMinus1)
			{
				vectorDir = vector.Direction(anchorPoints[i - 1], anchorPoint);
				if (m_bRotate180DegreeYawEndPole)
					vectorDir = -vectorDir;
			}
			else
			{
				vectorDir = vector.Direction(
					vector.Direction(anchorPoint, anchorPoints[i - 1]).Normalized(),
					vector.Direction(anchorPoint, anchorPoints[i + 1]).Normalized());
			}

			vectorDir = vectorDir.VectorToAngles(); // variable reuse
			vectorDir = { 0, Math.Repeat(vectorDir[0] + junctionData.m_fYawOffset, 360), 0 };

			if (junctionData.m_bApplyPitchAndRollRandomisation)
			{
				if (m_fRandomPitchAngle != 0)	// pitch
					vectorDir[0] = m_RandomGenerator.RandFloatXY(-m_fRandomPitchAngle, m_fRandomPitchAngle);

				if (m_fRandomRollAngle != 0)	// roll
					vectorDir[2] = m_RandomGenerator.RandFloatXY(-m_fRandomRollAngle, m_fRandomRollAngle);
			}

			if (junctionData.m_fYOffset)
				anchorPoint[1] = anchorPoint[1] + junctionData.m_fYOffset;

			objectSource = worldEditorAPI.CreateEntity(junctionResourceName, string.Empty, m_iSourceLayerID, m_Source, anchorPoint, vectorDir);
			if (!objectSource)
				continue;

			poleSource = GetPowerPoleSourceFromEntitySource(objectSource);
			if (!poleSource)
			{
				worldEditorAPI.DeleteEntity(objectSource);
				continue;
			}

			if (junctionData.m_bPowerSource)
				worldEditorAPI.SetVariableValue(poleSource, null, "PowerSource", "1");

			result.Insert(objectSource, poleSource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		if (key == "coords")
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		BaseContainerTools.WriteToInstance(this, worldEditorAPI.EntityToSource(this));

		Generate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates cable pole positions first then creates poles and cables if valid
	//! Only called by Generate()
	protected void GeneratePoles()
	{
		if (!m_ParentShapeSource)
		{
			Print("Parent Shape Source is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		if (!ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource))) // wat - parent shape... is not a shape
		{
			Print("Parent Shape is null, cannot generate power poles", LogLevel.ERROR);
			return;
		}

		array<vector> anchorPointsRelPos = m_ShapeNextPointHelper.GetAnchorPoints();
		if (anchorPointsRelPos.Count() < 2)
		{
			Print("Not enough shape points", LogLevel.ERROR);
			return;
		}

		// POLES'S POSITION
		array<vector> polePointsRelPos = {};
		map<int, ref SCR_PowerlineGeneratorPointData> pointDataMap = new map<int, ref SCR_PowerlineGeneratorPointData>();

		if (!GetPolesRelPosition(polePointsRelPos, pointDataMap))
			return;

		if (polePointsRelPos.IsEmpty())
		{
			Print("Power Line Generator shape is invalid; no poles will be generated", LogLevel.ERROR);
			return;
		}

		// POLES (and cables) CREATION
		CreatePolesAndCables(polePointsRelPos, anchorPointsRelPos, pointDataMap);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates poles relative positions for powerline generator based on anchor points, tesselated points and powerline data
	//! \param[in] pointsData
	//! \param[out] polePointsRelPos pole positions
	//! \param[out] pointDataMap map of <pole point index, pointData> (unrelated to position, only index)
	//! \return true on success, false on failure (no shape helper, etc)
	protected bool GetPolesRelPosition(
		notnull out array<vector> polePointsRelPos,
		notnull out map<int, ref SCR_PowerlineGeneratorPointData> pointDataMap)
	{
		if (!m_ShapeNextPointHelper || !m_ShapeNextPointHelper.IsValid())
		{
			Print("[SCR_PowerlineGeneratorEntity.GetPolesRelPosition] invalid shapeNextPointHelper (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.ERROR);
			return false;
		}

		if (m_fDistance < POLE_SAFEZONE_DISTANCE)
			m_fDistance = POLE_SAFEZONE_DISTANCE;

		// ANCHORS DATA COLLECTION
		map<int, ref ShapePointDataScriptBase> pointsData = GetFirstPointDataMap(SCR_PowerlineGeneratorPointData);

		IEntity parentShapeEntity = _WB_GetEditorAPI().SourceToEntity(m_ParentShapeSource);

		float poleOffset;
		bool generatePerPoint;
		vector lastPoleRelPos;
		SCR_PowerlineGeneratorPointData pointData;
		array<vector> anchorPointsRelPos = m_ShapeNextPointHelper.GetAnchorPoints();
		int anchorPointsCountMinus1 = anchorPointsRelPos.Count() - 1;
		foreach (int i, vector anchorPointRelPos : anchorPointsRelPos)
		{
			bool isStart = i == 0;
			bool isEnd = i == anchorPointsCountMinus1;
			bool hasJunction;

			bool wasGeneratedPerPoint = generatePerPoint;
			float previousOffset = poleOffset;

			pointData = SCR_PowerlineGeneratorPointData.Cast(pointsData.Get(i));
			if (pointData)
			{
				poleOffset = pointData.m_fPoleOffset;
				generatePerPoint = pointData.m_bGeneratePerPoint;

				// hardcoded safety
				if (m_fDistance + poleOffset < POLE_SAFEZONE_DISTANCE)
					poleOffset = POLE_SAFEZONE_DISTANCE - m_fDistance;

				hasJunction = pointData.m_JunctionData != null && (m_DefaultJunctionPole || pointData.m_JunctionData.m_sJunctionPrefab);
			}
			else
			{
				poleOffset = 0;
			}

			if (!isStart && !wasGeneratedPerPoint)
			{
				// add offset if it exists
				if (previousOffset != 0 && m_ShapeNextPointHelper.GetNextPoint(m_fDistance + previousOffset, lastPoleRelPos, i, xzMode: m_bNextPointMeasurementXZ))
						polePointsRelPos.Insert(lastPoleRelPos);

				// straight to the current anchor point!
				while (m_ShapeNextPointHelper.GetNextPoint(m_fDistance, lastPoleRelPos, i, xzMode: m_bNextPointMeasurementXZ))
				{
					polePointsRelPos.Insert(lastPoleRelPos);
				}
			}

			if (wasGeneratedPerPoint || generatePerPoint || hasJunction || isEnd || isStart || vector.DistanceSq(lastPoleRelPos, anchorPointRelPos) == 0)
			{
				int pointsCount = polePointsRelPos.Count();
				if (hasJunction)
				{
					if (pointData)
						pointDataMap.Insert(pointsCount, pointData);
				}
				else
				{
					if (pointsCount > 0 && vector.DistanceSq(lastPoleRelPos, anchorPointRelPos) < POLE_SAFEZONE_DISTANCE)
						polePointsRelPos.Remove(pointsCount - 1);
				}

				polePointsRelPos.Insert(anchorPointRelPos);

				m_ShapeNextPointHelper.SetOnAnchor(i);
				lastPoleRelPos = anchorPointRelPos;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates poles and cables between given points with optional random pitch and roll, snaps to terrain if no junction found
	//! \param[in] polePointsRelpos
	//! \param[in] anchorPointsRelPos
	//! \param[in] pointsData
	protected void CreatePolesAndCables(notnull array<vector> polePointsRelPos, notnull array<vector> anchorPointsRelPos, notnull map<int, ref SCR_PowerlineGeneratorPointData> pointsData)
	{
		bool snapToGround = m_bSnapOffsetShapeToTheGround && !m_bSculptTerrain;
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		int poleWorldPointsCountMinus1 = polePointsRelPos.Count() - 1;
		vector prevPoleRelPoint;
		ResourceName prefab;
		vector vectorDir; // no need for it to ever be normalised as it is .ToYaw()'d

		IEntitySource currPoleObject;
		IEntitySource prevPoleSource;
		IEntitySource currPoleSource;

		SCR_PowerPole prevPoleEntity;
		SCR_PowerPole currPoleEntity;
		foreach (int i, vector currPoleRelPoint : polePointsRelPos)
		{
			bool applyRandomPitchAndRoll;
			bool isStart = i == 0;
			bool isEnd = i == poleWorldPointsCountMinus1;
			bool isAnchor = isStart || isEnd;
			if (!isAnchor)
			{
				foreach (vector anchorPointRelPos : anchorPointsRelPos)
				{
					if (currPoleRelPoint == anchorPointRelPos) // a bit rough, but it works for now
					{
						isAnchor = true;
						break;
					}
				}
			}

			bool isSameLine = true;

			if (isAnchor)	// anchor: start, end or potential junction
			{
				// one must look for junction on all points
				// as another generator could make a junction here
				currPoleSource = FindJunctionOnPosXZ(m_Source, CoordToParent(currPoleRelPoint), isSameLine);

				if (currPoleSource) // found junction
				{
					if (s_DebugShapeManager)
						s_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), 2, 0x88800080);

					applyRandomPitchAndRoll = isSameLine;

					// cannot set junction's yaw here
					// as other lines joining before this line is initialised would not match the new junction's direction
				}
				else
				{
					if (isStart)																	// start pole
					{
						vectorDir = vector.Direction(currPoleRelPoint, polePointsRelPos[1]);
						prefab = m_StartPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollStart;
						if (m_bRotate180DegreeYawStartPole)
							vectorDir = -vectorDir;
					}
					else if (isEnd)																	// end pole
					{
						vectorDir = vector.Direction(prevPoleRelPoint, currPoleRelPoint);
						prefab = m_EndPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollEnd;
						if (m_bRotate180DegreeYawEndPole)
							vectorDir = -vectorDir;
					}
					else																			// normal pole that happens to be EXACTLY on an anchor pos
					{
						vectorDir = vector.Direction(
							vector.Direction(currPoleRelPoint, prevPoleRelPoint).Normalized(),
							vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]).Normalized());

						prefab = m_DefaultPole;
						applyRandomPitchAndRoll = m_bApplyPitchAndRollDefault;
					}
				}
			}
			else																					// normal pole
			{
				vectorDir = vector.Direction(
					vector.Direction(currPoleRelPoint, prevPoleRelPoint).Normalized(),
					vector.Direction(currPoleRelPoint, polePointsRelPos[i + 1]).Normalized());

				prefab = m_DefaultPole;
				applyRandomPitchAndRoll = m_bApplyPitchAndRollDefault;
			}

			if (!currPoleSource)
			{
				if (snapToGround)
				{
					vector worldPos = CoordToParent(currPoleRelPoint);
					worldPos[1] = worldEditorAPI.GetTerrainSurfaceY(worldPos[0], worldPos[2]);
					currPoleRelPoint = CoordToLocal(worldPos) + vector.Up * m_vShapeOffset[1];
				}

				vector angles = { 0, vectorDir.ToYaw(), 0 };

				if (m_fRandomYawAngle != 0)
					angles[1] = angles[1] + m_RandomGenerator.RandFloatXY(-m_fRandomYawAngle, m_fRandomYawAngle);

				if (applyRandomPitchAndRoll)
				{
					if (m_fRandomPitchAngle != 0)
						angles[0] = m_RandomGenerator.RandFloatXY(-m_fRandomPitchAngle, m_fRandomPitchAngle);

					if (m_fRandomRollAngle != 0)
						angles[2] = m_RandomGenerator.RandFloatXY(-m_fRandomRollAngle, m_fRandomRollAngle);
				}

				currPoleObject = worldEditorAPI.CreateEntity(prefab, string.Empty, m_iSourceLayerID, m_Source, currPoleRelPoint, angles);
				currPoleSource = GetPowerPoleSourceFromEntitySource(currPoleObject);

				if (s_DebugShapeManager)
				{
					s_DebugShapeManager.AddSphere(CoordToParent(currPoleRelPoint), DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_POS, ShapeFlags.NOOUTLINE);
					s_DebugShapeManager.AddArrow(CoordToParent(currPoleRelPoint), CoordToParent(currPoleRelPoint + 3 * vector.FromYaw(angles[1])));
				}
			}

			if (i > 0)
			{
				prevPoleEntity = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(prevPoleSource));
				if (prevPoleEntity)
				{
					currPoleEntity = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(currPoleSource));
					if (currPoleEntity && prevPoleEntity != currPoleEntity)
						CreatePowerLines(m_Source, prevPoleSource, currPoleSource);
				}
			}

			prevPoleSource = currPoleSource;
			prevPoleRelPoint = currPoleRelPoint;
			currPoleObject = null;
			currPoleSource = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] generatorSource
	//! \param[in] worldPos
	//! \param[out] isSameLine
	//! \return
	protected static IEntitySource FindJunctionOnPosXZ(notnull IEntitySource generatorSource, vector worldPos, out bool isSameLine)
	{
		map<IEntitySource, vector> selfMap = s_mGeneratorAndPolesPlusParentWorldPosMap.Get(generatorSource);
		if (!selfMap)
			return null;

		foreach (IEntitySource poleSource, vector poleWorldPos : selfMap)
		{
			if (vector.DistanceSqXZ(worldPos, poleWorldPos) < JUNCTION_DISTANCEXZ_SQ)
			{
				isSameLine = true;
				return poleSource;
			}
		}

		foreach (IEntitySource otherGeneratorSource, map<IEntitySource, vector> junctionPosMap : s_mGeneratorAndPolesPlusParentWorldPosMap)
		{
			if (otherGeneratorSource == generatorSource)
				continue;

			foreach (IEntitySource poleSource, vector poleWorldPos : junctionPosMap)
			{
				if (vector.DistanceSqXZ(worldPos, poleWorldPos) < JUNCTION_DISTANCEXZ_SQ)
				{
					isSameLine = false;
					return poleSource;
				}
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates power lines between two power poles based on their cable types, if they have compatible slots
	//! \param[in] generatorSource
	//! \param[in] existingPoleSource pole from which to connect (the previously inserted one)
	//! \param[in] addedPoleSource pole to which to connect (the newly inserted one or the junction)
	//! \return the created power line entity sources, or null array on error / no cables
	// TODO: fix cable bug on rotated shape
	protected static array<IEntitySource> CreatePowerLines(notnull IEntitySource generatorSource, notnull IEntitySource existingPoleSource, notnull IEntitySource addedPoleSource)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return null;

		SCR_PowerPole existingPole = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(existingPoleSource));
		SCR_PowerPole addedPole = SCR_PowerPole.Cast(worldEditorAPI.SourceToEntity(addedPoleSource));
		vector existingPoleWorldPos = existingPole.GetOrigin();
		vector addedPoleWorldPos = addedPole.GetOrigin();

		bool isSameLine = existingPoleSource.GetParent() == addedPoleSource.GetParent();

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> existingPoleGroups = existingPole.GetClosestCableSlotGroupsForEachCableType(addedPoleWorldPos, isSameLine);
		if (existingPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the existing power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (s_DebugShapeManager)
				s_DebugShapeManager.AddSphere(existingPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> addedPoleGroups = addedPole.GetClosestCableSlotGroupsForEachCableType(existingPoleWorldPos, isSameLine);
		if (addedPoleGroups.IsEmpty())
		{
			Print("No valid slots found on the new power pole, please check the setup of your power poles.", LogLevel.WARNING);

			if (s_DebugShapeManager)
				s_DebugShapeManager.AddSphere(addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);

			return null;
		}

		array<SCR_EPoleCableType> cableTypes = {};
		if (SCR_Enum.GetEnumValues(SCR_EPoleCableType, cableTypes) < 1) // like, wat
			return null;

		array<SCR_EPoleCableType> cableTypesInCommon = {};
		foreach (SCR_EPoleCableType cableType : cableTypes)
		{
			if (existingPoleGroups.Contains(cableType) && addedPoleGroups.Contains(cableType))
				cableTypesInCommon.Insert(cableType);

			// code below could be done here directly, but for clarity's sake, one foreach won't kill perfs
		}

		if (cableTypesInCommon.IsEmpty())
		{
			Print("No cable types in common between two poles - be sure to select compatible poles in Generator Prefab at " + Debug.GetEntityLinkString(existingPole), LogLevel.WARNING);

			if (s_DebugShapeManager)
			{
				s_DebugShapeManager.AddSphere(existingPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);
				s_DebugShapeManager.AddSphere(addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_SIZE, DEBUG_POLE_COLOUR_ERROR, ShapeFlags.NOOUTLINE);
				s_DebugShapeManager.AddLine(existingPoleWorldPos + DEBUG_POLE_POS, addedPoleWorldPos + DEBUG_POLE_POS, DEBUG_POLE_COLOUR_ERROR);
			}

			return null;
		}

		ResourceName powerLineMaterial;
		generatorSource.Get("m_PowerlineMaterial", powerLineMaterial);

		ResourceName cablePrefab;
		generatorSource.Get("m_sPowerLinePrefab", cablePrefab);

		int generatorLayerID = generatorSource.GetLayerID();

		array<ref SCR_PoleCable> poleCables;
		generatorSource.Get("m_aCables", poleCables);

		string powerLineEntity;
		if (cablePrefab) // !.IsEmpty()
			powerLineEntity = cablePrefab;
		else
			powerLineEntity = DEFAULT_CABLE_PREFAB;

		array<IEntitySource> result = {};

		array<vector> startPoints = {};		// world position
		array<vector> endPoints = {};		// world position
		SCR_PoleCableSlotGroup existingGroup;
		SCR_PoleCableSlotGroup addedGroup;
		IEntitySource powerLineEntitySource;
		IEntity referenceEntity;
		array<ref ContainerIdPathEntry> containerPath;
		foreach (SCR_EPoleCableType cableType : cableTypesInCommon)
		{
			existingGroup = existingPoleGroups.Get(cableType);
			int existingPoleCableCount = existingGroup.m_aSlots.Count();	// cannot be zero

			addedGroup = addedPoleGroups.Get(cableType);
			int addedPoleCableCount = addedGroup.m_aSlots.Count();			// cannot be zero

			int cableCount;
			if (existingPoleCableCount < addedPoleCableCount)
				cableCount = existingPoleCableCount;
			else
				cableCount = addedPoleCableCount;

			startPoints.Clear();
			endPoints.Clear();
			for (int i; i < cableCount; ++i)
			{
				startPoints.Insert(existingPole.CoordToParent(existingGroup.m_aSlots[i].m_vPosition));
				endPoints.Insert(addedPole.CoordToParent(addedGroup.m_aSlots[i].m_vPosition));
			}

			if (s_DebugShapeManager) // warn visually about unused slots
			{
				for (int i = cableCount; i < existingPoleCableCount; ++i)
				{
					s_DebugShapeManager.AddSphere(existingPole.CoordToParent(existingGroup.m_aSlots[i].m_vPosition), DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_UNUSED, ShapeFlags.NOOUTLINE);
				}

				for (int i = cableCount; i < addedPoleCableCount; ++i)
				{
					s_DebugShapeManager.AddSphere(addedPole.CoordToParent(addedGroup.m_aSlots[i].m_vPosition), DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_UNUSED, ShapeFlags.NOOUTLINE);
				}
			}

			float normalLengthSq;
			float reversedLengthSq;
			foreach (int i, vector startPoint : startPoints)
			{
				normalLengthSq += vector.DistanceSq(startPoint, endPoints[i]);
				reversedLengthSq += vector.DistanceSq(startPoint, endPoints[cableCount - i - 1]);
			}

			// don't cross the streams!
			if (reversedLengthSq < normalLengthSq)
				SCR_ArrayHelperT<vector>.Reverse(endPoints);

			if (s_DebugShapeManager)
			{
				foreach (int i, vector startPoint : startPoints)
				{
					s_DebugShapeManager.AddSphere(startPoint, DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_POS, ShapeFlags.NOOUTLINE);
					s_DebugShapeManager.AddSphere(endPoints[i], DEBUG_SLOT_SIZE, DEBUG_SLOT_COLOUR_POS, ShapeFlags.NOOUTLINE);
					s_DebugShapeManager.AddLine(startPoint, endPoints[i], DEBUG_CABLE_COLOUR_POS);
				}
			}

			// power line preparation
			powerLineEntitySource = null;

			vector startPos, endPos;
			// Create cables for the entity

			int cableIndex;
			foreach (int i, vector startPoint : startPoints)
			{
				vector endPoint = endPoints[i];

				if (powerLineEntitySource)
				{
					referenceEntity = worldEditorAPI.SourceToEntity(powerLineEntitySource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoint);
				}
				else
				{
					referenceEntity = worldEditorAPI.SourceToEntity(generatorSource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoint);
				}

				if (vector.DistanceSqXZ(startPos, endPos) < MIN_CABLE_LENGTH_SQ)
				{
					PrintFormat("Cable's 2D length is too small! Skipping (%1m < %2m)", vector.DistanceXZ(startPos, endPos).ToString(-1, 2), Math.Sqrt(MIN_CABLE_LENGTH_SQ), level: LogLevel.WARNING);

					if (s_DebugShapeManager)
					{
						float localX = existingPole.CoordToLocal(startPoint)[0];
						vector forward = vector.Direction(startPoint, endPoint); // not normalised
						vector right = -{
							forward[0] * Math.Cos(Math.PI_HALF) - forward[2] * Math.Sin(Math.PI_HALF),
							0,													// Y rotation - 2D only
							forward[0] * Math.Sin(Math.PI_HALF) + forward[2] * Math.Cos(Math.PI_HALF),
						};

						if (localX > DEBUG_CABLE_LENGTH_POS_MAX_DIST)
							right *= DEBUG_CABLE_LENGTH_POS_MAX_DIST;
						else
							right *= localX;

						vector spherePos = (startPoint + endPoint) * 0.5 + DEBUG_CABLE_LENGTH_POS + right;

						s_DebugShapeManager.AddSphere(spherePos, DEBUG_CABLE_LENGTH_SIZE, DEBUG_CABLE_LENGTH_COLOUR_ERROR_SPHERE, ShapeFlags.NOOUTLINE);
						s_DebugShapeManager.AddLine(spherePos, startPoint, DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE);
						s_DebugShapeManager.AddLine(spherePos, endPoint, DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE);
					}

					continue;
				}

				if (vector.DistanceSq(startPos, endPos) > MAX_CABLE_LENGTH_SQ)
				{
					PrintFormat("Cable's 3D length is too big! Skipping (%1m > %2m)", vector.Distance(startPos, endPos).ToString(-1, 2), Math.Sqrt(MAX_CABLE_LENGTH_SQ), level: LogLevel.WARNING);

					if (s_DebugShapeManager)
						s_DebugShapeManager.AddLine(startPoint, endPoint, DEBUG_CABLE_LENGTH_COLOUR_ERROR_LINE);

					continue;
				}

				if (!powerLineEntitySource)
				{
					powerLineEntitySource = worldEditorAPI.CreateEntity(powerLineEntity, string.Empty, generatorLayerID, generatorSource, referenceEntity.CoordToLocal(existingPoleWorldPos), vector.Zero);
					if (!powerLineEntitySource)
					{
						if (powerLineEntity != POWER_LINE_CLASS)
						{
							Print("CreateEntity returned null trying to create " + powerLineEntity, LogLevel.ERROR);
							powerLineEntity = POWER_LINE_CLASS;
							powerLineEntitySource = worldEditorAPI.CreateEntity(powerLineEntity, string.Empty, generatorLayerID, generatorSource, referenceEntity.CoordToLocal(existingPoleWorldPos), vector.Zero);
						}

						if (!powerLineEntitySource)
						{
							Print("CreateEntity returned null trying to create " + powerLineEntity, LogLevel.ERROR);
							return null; // it would fail for the other ones too
						}
					}

					// get the most adapted material
					ResourceName cableMaterial;
					foreach (SCR_PoleCable cable : poleCables)
					{
						if (cable.m_eType == cableType && cable.m_sMaterial) // !.IsEmpty()
						{
							cableMaterial = cable.m_sMaterial;
							break;
						}
					}

					if (!cableMaterial)
						cableMaterial = powerLineMaterial; // default value

					worldEditorAPI.BeginEditSequence(powerLineEntitySource);

					if (cableMaterial)
						worldEditorAPI.SetVariableValue(powerLineEntitySource, null, "Material", cableMaterial);

					referenceEntity = worldEditorAPI.SourceToEntity(powerLineEntitySource);
					startPos = referenceEntity.CoordToLocal(startPoint);
					endPos = referenceEntity.CoordToLocal(endPoint);
				}
				else
				{
					worldEditorAPI.BeginEditSequence(powerLineEntitySource);
				}

				worldEditorAPI.CreateObjectArrayVariableMember(powerLineEntitySource, null, "Cables", "Cable", cableIndex);
				containerPath = { new ContainerIdPathEntry("Cables", cableIndex) };
				worldEditorAPI.SetVariableValue(powerLineEntitySource, containerPath, "StartPoint", startPos.ToString(false));
				worldEditorAPI.SetVariableValue(powerLineEntitySource, containerPath, "EndPoint", endPos.ToString(false));

				worldEditorAPI.EndEditSequence(powerLineEntitySource);

				cableIndex++;
			}

			if (powerLineEntitySource)
				result.Insert(powerLineEntitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the first power pole, either the entity or one of its direct children
	//! \param[in] powerPoleEntitySource the power pole's entity source
	//! \return the source for the power pole itself or null if not found
	protected static IEntitySource GetPowerPoleSourceFromEntitySource(notnull IEntitySource powerPoleEntitySource)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (powerPoleEntitySource.GetClassName().ToType().IsInherited(SCR_PowerPole))
			return powerPoleEntitySource;

		typename childType;
		for (int i, childrenCount = powerPoleEntitySource.GetNumChildren(); i < childrenCount; i++)
		{
			childType = powerPoleEntitySource.GetChild(i).GetClassName().ToType();
			if (childType && childType.IsInherited(SCR_PowerPole))
				return powerPoleEntitySource.GetChild(i);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);
		if (!m_ParentShapeSource)
			return;

		Generate();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		Generate();
	}
#endif // WORKBENCH
}
