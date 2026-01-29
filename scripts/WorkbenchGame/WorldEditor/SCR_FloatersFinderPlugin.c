#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Floaters Finder", category: "Object Placement", wbModules: { "WorldEditor" }, shortcut: "Ctrl+Alt+Page Up", awesomeFontCode: 0xF338)] // 0xF338 = ↨
class SCR_FloatersFinderPlugin : WorkbenchPlugin
{
	/*
		CATEGORY: Search
	*/

	[Attribute(defvalue: "1", desc: "Only search in the currently active layer", category: "Search")]
	protected bool m_bActiveLayerOnly;

	[Attribute(defvalue: "1", desc: "Check for entities entirely below terrain by bounding box vertices' altitude", category: "Search")]
	protected bool m_bSearchForEntitiesBelowTerrain;

	[Attribute(defvalue: "1", desc: "Look for misplaced vegetation (trees & bushes) - floating up or too deep in the ground, etc.", category: "Search")]
	protected bool m_bSearchForVegetation;

//	[Attribute(defvalue: "1", desc: "Check for out-of-bounds entities", category: "Search")]
//	protected bool m_bSearchForOutOfBoundsEntities;

	// TODO - static buildings, destructible buildings
//	[Attribute(defvalue: "1", desc: "Look for buildings entering the terrain", category: "Search")]
//	protected bool m_bSearchForBuildings;

	[Attribute(defvalue: "false", desc: "If set, uses the Prefab's random angle as acceptable margin; otherwise, uses below setting", category: "Angle")]
	protected bool m_bUsePrefabAngles;

	[Attribute(defvalue: "180", uiwidget: UIWidgets.Slider, desc: "Maximum allowed angle to not consider the tree as fallen. 0 = fully vertical tree, 90 = horizontal tree, 180 = no angle check (needs \"Use Prefab Angles\" unchecked)", params: "0 180 0.5", category: "Angle", precision: 1)]
	protected float m_fMaxTreeAngle;

	/*
		CATEGORY: Vertical Offset
	*/

	[Attribute(defvalue: "1", desc: "Check object's altitude from OBJECTS below it (2× slower as it uses Trace)", category: "Vertical Offset")]
	protected bool m_bCheckAboveEntitiesSurface;

	[Attribute(defvalue: "0", desc: "If set, uses the Prefab's vertical offset range; otherwise, uses below settings", category: "Vertical Offset")]
	protected bool m_bUsePrefabVerticalOffset;

	[Attribute(defvalue: "-1", desc: "Minimum (included) vertical offset (needs \"Use Prefab Vertical Offset\" unchecked)", params: "-100 100 0.1", category: "Vertical Offset", precision: 1)]
	protected float m_fMinVerticalOffset;

	[Attribute(defvalue: "0.5", desc: "Maximum (included) vertical offset (needs \"Use Prefab Vertical Offset\" unchecked)", params: "-100 100 0.1", category: "Vertical Offset", precision: 1)]
	protected float m_fMaxVerticalOffset;

	[Attribute(defvalue: "0", desc: "Check for items being below water level (Ocean only for now). If under water, whatever the offset, the entity will be selected", category: "Vertical Offset")]
	protected bool m_bCheckBelowWater;

	/*
		CATEGORY: Performance
	*/

	[Attribute(defvalue: "1000", uiwidget: UIWidgets.Slider, desc: "Search radius around camera position - 0 = all entities", params: "0 5000 1", category: "Performance")]
	protected int m_iCameraSearchRadius;

	[Attribute(defvalue: "1", desc: "Show radius sphere on entity search", category: "Performance")]
	protected bool m_bShowSearchRadiusSphere;

	[Attribute(defvalue: "2000", uiwidget: UIWidgets.Slider, desc: "Maximum number of selected entities (UI performance)", params: "1 5000 1", category: "Performance")]
	protected int m_iMaxSelectedEntities;

	[Attribute(defvalue: "1", desc: "[requires Search For Vegetation] Trace only checks for the vegetation's full height, allowing it to be below a bridge but may be missing a small tree entirely inside a rock", category: "Performance")]
	protected bool m_bTraceVegetationPrecisely;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.Slider, desc: "Trace origin's distance (in metres) from above the entity to determine if it is below another entity AND Trace Vegetation Precisely is unchecked", params: "10 500 10", category: "Performance")]
	protected int m_iTraceOriginDistance;

	/*
		CATEGORY: Output
	*/

	[Attribute(defvalue: "0", desc: "If ticked, write a file with links to all findings.", category: "Output")]
	protected bool m_bOutputFindingsToFile;

	[Attribute(defvalue: "0", desc: "Prefix file links with https prefix", category: "Output")]
	protected bool m_bUseWebPrefix;

	protected static const ref array<IEntitySource> WORLD_ENTITIES = {}; // non-nullable due to const
	protected static ref Shape s_DetectionRadiusSphere;

	protected static const int DEBUG_COLOUR = 0x99025D00;
	protected static const int DEBUG_DURATION = 333; // ms
	protected static const string OUTPUT_FILENAME = "FoundFloatingEntities.txt";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Init())
			return;

		Print("Floaters Finder - Run method started", LogLevel.NORMAL);

		int firstTick;
		WORLD_ENTITIES.Clear();

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		bool useSelectedEntities = worldEditorAPI.GetSelectedEntity() != null;
		if (useSelectedEntities) // use selected entities
		{
			for (int i, cnt = worldEditorAPI.GetSelectedEntitiesCount(); i < cnt; i++)
			{
				WORLD_ENTITIES.Insert(worldEditorAPI.GetSelectedEntity(i));
			}

			Print(string.Format("Going with the current selection of %1 entities", WORLD_ENTITIES.Count()), LogLevel.NORMAL);
		}
		else // detect entities
		{
			firstTick = System.GetTickCount();
			GetEntities(baseWorld);
			Print(string.Format("Entity getter duration: %1ms for %2 entities", System.GetTickCount() - firstTick, WORLD_ENTITIES.Count()), LogLevel.NORMAL);
		}

		// filter
		array<IEntitySource> filteredEntities = {};
		firstTick = System.GetTickCount();
		FilterEntities(baseWorld, filteredEntities);
		Print(string.Format("Entity filter duration: %1ms for %2 entities (output: %3 entities)", System.GetTickCount() - firstTick, WORLD_ENTITIES.Count(), filteredEntities.Count()), LogLevel.NORMAL);

		// select all found entities in UI
		SelectEntities(filteredEntities);

		// show after time-consuming operations in order to not "blink" it
		if (m_bShowSearchRadiusSphere && !useSelectedEntities && m_iCameraSearchRadius > 0)
		{
			vector cameraMatrix[4];
			baseWorld.GetCurrentCamera(cameraMatrix);
			s_DetectionRadiusSphere = Shape.CreateSphere(DEBUG_COLOUR, ShapeFlags.BACKFACE | ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP, cameraMatrix[3], m_iCameraSearchRadius);
			thread DeleteSphereThread();
		}

		// final report
		if (worldEditorAPI.GetSelectedEntitiesCount() != filteredEntities.Count())
		{
			Print(
				string.Format(
					"Could not select all entities: Treated %1, Detected %2, Selected %3",
					WORLD_ENTITIES.Count(),
					filteredEntities.Count(),
					worldEditorAPI.GetSelectedEntitiesCount()),
				LogLevel.WARNING);
		}
		else
		{
			Print(
				string.Format(
					"%1 entities treated, %2 selected properly",
					WORLD_ENTITIES.Count(),
					filteredEntities.Count()),
				LogLevel.NORMAL);
		}

		// output to file if set to do so
		if (m_bOutputFindingsToFile)
		{
			firstTick = System.GetTickCount();
			OutputEntitiesToFile(filteredEntities);
			Print(
				string.Format(
					"%1 entities successfully written to file %2 in %3ms",
					filteredEntities.Count(),
					FilePath.ToSystemFormat(OUTPUT_FILENAME),
					System.GetTickCount() - firstTick),
				LogLevel.NORMAL);
		}

		Print("Floaters Finder - Run method ended", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (!SCR_Global.IsEditMode())
		{
			Print("Floaters Finder - Run method stopped because non-Workbench run", LogLevel.NORMAL);
			return false;
		}

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (worldEditor.IsPrefabEditMode())
		{
			Print("Floaters Finder - Run method stopped because World Editor is in Prefab edit mode", LogLevel.NORMAL);
			return false;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("Floaters Finder - Run method stopped because World Editor API was not found", LogLevel.WARNING);
			return false;
		}

		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		if (!baseWorld)
		{
			Print("Floaters Finder - Run method stopped because base world was not found", LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void GetEntities(BaseWorld baseWorld)
	{
		if (m_iCameraSearchRadius > 0)
		{
			vector cameraMatrix[4];
			baseWorld.GetCurrentCamera(cameraMatrix);
			baseWorld.QueryEntitiesBySphere(cameraMatrix[3], m_iCameraSearchRadius, InsertEntity);
		}
		else
		{
			vector minPos, maxPos;
			baseWorld.GetBoundBox(minPos, maxPos);
			baseWorld.QueryEntitiesByAABB(minPos, maxPos, InsertEntity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void DeleteSphereThread()
	{
		Sleep(DEBUG_DURATION);
		s_DetectionRadiusSphere = null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool InsertEntity(notnull IEntity entity)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		WORLD_ENTITIES.Insert(worldEditorAPI.EntityToSource(entity));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterEntities(notnull BaseWorld baseWorld, notnull out array<IEntitySource> filteredEntities)
	{
		BaseContainerList editorData;
		BaseContainer firstEditorData;
		BaseContainer ancestorContainer;
		vector randomVerticalOffset;
		bool isVegetation;
		bool insertEntity;

		int placementMode;

		bool isOcean = baseWorld.IsOcean();
		float oceanHeight = baseWorld.GetOceanBaseHeight();

		vector entityPos, bboxMin, bboxMax, tempPos;
		vector bboxCorners[15]; // 8 vertices + 6 face centres + 1 centre
		float altitude, terrainY, tempTerrainY;

		TraceParam traceParam = new TraceParam();
		float traceRatio;
		float minVerticalOffset = m_fMinVerticalOffset;
		float maxVerticalOffset = m_fMaxVerticalOffset;
		float maxPitch = m_fMaxTreeAngle;
		float maxRoll = m_fMaxTreeAngle;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool manyEntities = WORLD_ENTITIES.Count() > 10;
		int currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();
		int underWaterNb, verticalOffsetNb, angleOffsetNb, fullyUndergroundNb;

		filteredEntities.Clear();
		foreach (IEntitySource entitySource : WORLD_ENTITIES)
		{
			if (!m_bOutputFindingsToFile && filteredEntities.Count() >= m_iMaxSelectedEntities)
				break;

			if (!entitySource)
				continue;

			if (m_bActiveLayerOnly && entitySource.GetLayerID() != currentLayerId)
				continue;

			IEntity entity = worldEditorAPI.SourceToEntity(entitySource);
			isVegetation = Tree.Cast(entity) != null;

			if (!(m_bSearchForEntitiesBelowTerrain || isVegetation))
				continue;

			if (isVegetation && entitySource.Get("placement", placementMode) && placementMode == 1) // 1 = slopelandcontact
				continue;

			minVerticalOffset = m_fMinVerticalOffset;
			maxVerticalOffset = m_fMaxVerticalOffset;
			maxPitch = m_fMaxTreeAngle;
			maxRoll = m_fMaxTreeAngle;

			editorData = entitySource.GetObjectArray("editorData");
			if (editorData && editorData.Count() > 0)
			{
				firstEditorData = editorData.Get(0);

				if (m_bUsePrefabVerticalOffset)
				{
					firstEditorData.Get("randomVertOffset", randomVerticalOffset);
					minVerticalOffset = randomVerticalOffset[0];
					maxVerticalOffset = randomVerticalOffset[1];
				}

				if (m_bUsePrefabAngles)
				{
					firstEditorData.Get("randomPitchAngle", maxPitch);
					firstEditorData.Get("randomRollAngle", maxRoll);
				}
			}

			insertEntity = false;

			entityPos = entity.GetOrigin();
			terrainY = worldEditorAPI.GetTerrainSurfaceY(entityPos[0], entityPos[2]);

			// underwater check - needs WORLD coords
			if (!insertEntity && m_bCheckBelowWater && isOcean && oceanHeight > entityPos[1])
			{
				if (!manyEntities)
					Print("below water level", LogLevel.NORMAL);

				underWaterNb++;
				insertEntity = true;
			}

			// angles check - if out of angle, leave checks
			if (!insertEntity && isVegetation && (m_bUsePrefabAngles || m_fMaxTreeAngle < 180))
			{
				vector angles;
				if (entitySource.Get("angles", angles)
					&& (
						angles[0] < -maxRoll || angles[2] < -maxPitch || angles[0] > maxRoll || angles[2] > maxPitch
					)
				)
				{
					if (!manyEntities)
						Print(string.Format("ignoring pitch %1/%2 / roll %3/%4", angles[2], maxPitch, angles[0], maxRoll), LogLevel.NORMAL);

					angleOffsetNb++;
					continue;
				}
			}

			// terrain pos/neg Y distance check
			altitude = entityPos[1] - terrainY;
			entity.GetBounds(bboxMin, bboxMax);

			// bbox checking
			if (
				!insertEntity &&
				m_bSearchForEntitiesBelowTerrain &&
				altitude < 0 &&
				(bboxMin != vector.Zero || bboxMax != vector.Zero) &&
				!LakeGeneratorEntity.Cast(entity) &&							// no lakes as some lakes' centre is not exposed
				!ShapeEntity.Cast(entity) &&									// no spline or polyline
				(
					!entity.GetParent() ||
					(
						!entity.GetParent().IsInherited(BaseBuilding) &&
						!entity.GetParent().IsInherited(StaticModelEntity)		// no undeletable prefab's sub-entity
					)
				)
			)
			{
				// vertices
				bboxCorners[0] = Vector(bboxMin[0], bboxMax[1], bboxMax[2]); // top-front-left
				bboxCorners[1] = Vector(bboxMax[0], bboxMax[1], bboxMax[2]); // top-front-right
				bboxCorners[2] = Vector(bboxMin[0], bboxMax[1], bboxMin[2]); // top-back-left
				bboxCorners[3] = Vector(bboxMax[0], bboxMax[1], bboxMin[2]); // top-back-right
				bboxCorners[4] = Vector(bboxMin[0], bboxMin[1], bboxMax[2]); // bottom-front-left
				bboxCorners[5] = Vector(bboxMax[0], bboxMin[1], bboxMax[2]); // bottom-front-right
				bboxCorners[6] = Vector(bboxMin[0], bboxMin[1], bboxMin[2]); // bottom-back-left
				bboxCorners[7] = Vector(bboxMax[0], bboxMin[1], bboxMin[2]); // bottom-back-right

				// box centre
				bboxCorners[14] = Vector((bboxMax[0] + bboxMin[1]) * 0.5, (bboxMax[1] + bboxMin[1]) * 0.5, (bboxMax[2] + bboxMin[2]) * 0.5);

				// face centres
				for (int i = 8; i < 14; i++) // creating then editing vectors is faster than inline-calculating each
				{
					bboxCorners[i] = bboxCorners[14];
				}

				bboxCorners[08][2] = bboxMax[2]; // front
				bboxCorners[09][0] = bboxMax[0]; // right
				bboxCorners[10][2] = bboxMin[2]; // back
				bboxCorners[11][0] = bboxMin[0]; // left
				bboxCorners[12][1] = bboxMax[1]; // top
				bboxCorners[13][1] = bboxMin[1]; // bottom

				insertEntity = true;
				for (int i; i < 15; i++)
				{
					tempPos = entity.CoordToParent(bboxCorners[i]);
					tempTerrainY = baseWorld.GetSurfaceY(tempPos[0], tempPos[2]);
					if (tempPos[1] > tempTerrainY)
					{
						insertEntity = false;
						break;
					}
				}

				if (insertEntity)
					fullyUndergroundNb++;
			}

			if (!insertEntity && isVegetation && m_bSearchForVegetation)
			{
				if (m_bCheckAboveEntitiesSurface && altitude >= minVerticalOffset) // trace trace
				{
					// TODO: align trace with entity's Y axis (entity.CoordToParent)… AND check relative altitude (two traces?)
					traceParam.Start = entityPos;

					if (m_bTraceVegetationPrecisely || bboxMax[1] > m_iTraceOriginDistance)
						traceParam.Start[1] = traceParam.Start[1] + bboxMax[1];
					else
						traceParam.Start[1] = traceParam.Start[1] + m_iTraceOriginDistance;

					if (traceParam.Start[1] > terrainY)
					{
						traceParam.End = entityPos;
						traceParam.End[1] = terrainY;
						traceParam.Flags = TraceFlags.ENTS;
						traceParam.Exclude = entity;
						traceRatio = baseWorld.TraceMove(traceParam, NoVegetationFilterCallback);
						if (!manyEntities)
						{
							Print("----- Tracing -----", LogLevel.NORMAL);
							Print(string.Format("OBJ %1", entityPos), LogLevel.NORMAL);
							Print(string.Format("FROM %1", traceParam.Start), LogLevel.NORMAL);
							Print(string.Format("TO %1", traceParam.End), LogLevel.NORMAL);
							Print(
								string.Format(
									"DONE %1pct (%2m/%3m)",
									Math.Round(traceRatio * 10000) * 0.01,
									vector.Distance(traceParam.Start, traceParam.End) * traceRatio,
									vector.Distance(traceParam.Start, traceParam.End)),
								LogLevel.NORMAL);
						}

						altitude -= (traceParam.Start[1] - traceParam.End[1]) * (1 - traceRatio);
					}
				}

				insertEntity = altitude > maxVerticalOffset || altitude < minVerticalOffset || traceParam.Start[1] <= terrainY;
				if (insertEntity)
				{
					if (!manyEntities)
						Print(string.Format("altitude %1 DOES NOT MATCH the [%2, %3] range", altitude, minVerticalOffset, maxVerticalOffset), LogLevel.NORMAL);

					verticalOffsetNb++;
				}
			}

			if (!insertEntity)
				continue;

			filteredEntities.Insert(entitySource);
		}

		Print(
			string.Format(
				"Filtered: %1 below water level, %2 vertical offset, %3 entirely underground, ignored %4 angle offset entities",
				underWaterNb,
				verticalOffsetNb,
				fullyUndergroundNb,
				angleOffsetNb),
			LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected bool NoVegetationFilterCallback(notnull IEntity entity, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return Tree.Cast(entity) == null;
	}

	//------------------------------------------------------------------------------------------------
	protected void SelectEntities(notnull array<IEntitySource> entities)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		worldEditorAPI.ClearEntitySelection();
		for (int i, cnt = Math.Min(m_iMaxSelectedEntities, entities.Count()); i < cnt; i++)
		{
			worldEditorAPI.AddToEntitySelection(entities[i]);
		}
		worldEditorAPI.UpdateSelectionGui();
	}

	//------------------------------------------------------------------------------------------------
	protected void OutputEntitiesToFile(notnull array<IEntitySource> filteredEntities)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		string worldName;
		worldEditorAPI.GetWorldPath(worldName);

		string link;
		if (filteredEntities.IsEmpty())
			link = ".";
		else
			link = ":";

		FileHandle fileHandle = FileIO.OpenFile(OUTPUT_FILENAME, FileMode.WRITE);
		fileHandle.WriteLine("===========================================================================");
		fileHandle.WriteLine("===== File generated by Floaters Finder Plugin on " + SCR_DateTimeHelper.GetDateTimeLocal() + " =====");
		fileHandle.WriteLine("===========================================================================");
		fileHandle.WriteLine(string.Empty);
		fileHandle.WriteLine(filteredEntities.Count().ToString() + " misplaced entities were found on " + worldName + " terrain" + link);
		if (!filteredEntities.IsEmpty())
			fileHandle.WriteLine(string.Empty);

		IEntity entity;
		vector transformation[4];
		transformation[0] = vector.Right;
		transformation[1] = vector.Up;
		transformation[2] = vector.Forward; // point North
		vector bboxMin, bboxMax, centre;
		foreach (IEntitySource entitySource : filteredEntities)
		{
			entity = worldEditorAPI.SourceToEntity(entitySource);
			entity.GetBounds(bboxMin, bboxMax);
			float diagonal = (bboxMax - bboxMin).Length();
			if (diagonal < 5)
				diagonal = 5;

			centre = entity.CoordToParent(Vector((bboxMax[0] + bboxMin[1]) * 0.5, (bboxMax[1] + bboxMin[1]) * 0.5, (bboxMax[2] + bboxMin[2]) * 0.5));
			transformation[3] = centre - 1.25 * diagonal * vector.Forward; // camera South by 1.25×'diagonal' metres

			fileHandle.WriteLine(SCR_CoordsTool.GetWorldEditorLink(transformation, m_bUseWebPrefix));
		}
		fileHandle.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog("Configure 'Floaters Finder' plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected bool ButtonOK()
	{
		return true;
	}
}
#endif // WORKBENCH
