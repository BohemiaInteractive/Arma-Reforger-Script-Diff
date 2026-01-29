#ifdef WORKBENCH
// [WorkbenchPluginAttribute(name: "Floaters Fixer", wbModules: { "WorldEditor" }, shortcut: "Ctrl+Alt+Page Down", awesomeFontCode: 0xE069)] // 0xE069 = plane-slash
class SCR_FloatersFixerPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "1", desc: "Set object's origin from OBJECTS or terrain (2× slower as it uses Trace)")]
	protected bool m_bSetOnTerrainAndEntitiesSurface;

	[Attribute(defvalue: "50", desc: "Trace origin's distance from above the object/terrain", uiwidget: UIWidgets.Slider, params: "10 500 10")]
	protected int m_iTraceOriginDistance;

	[Attribute(defvalue: "0", desc: "If set, uses the Prefab's vertical offset range; otherwise, uses below settings")]
	protected bool m_bUsePrefabVerticalOffset;

	[Attribute(defvalue: "1", desc: "If set, uses the Prefab's align to normal setting; otherwise uses Align To Normal Override")]
	protected bool m_bUsePrefabAlignToNormal;

	[Attribute(defvalue: "0", desc: "Forces Align To Normal; requires Use Prefab Align To Normal above to be off")]
	protected bool m_bAlignToNormalOverride;

	protected BaseWorld m_BaseWorld;

	protected static const ResourceName BUSH_BASE = "{D7163D1B571F4C0C}Prefabs/Vegetation/Core/Bush_Base.et";
	protected static const ResourceName TREE_BASE = "{388AE316D09D0680}Prefabs/Vegetation/Core/Tree_Base.et";
	protected static const int MANY_ENTITIES_THRESHOLD = 10; // threshold between detailed logs and batch logs (inclusive, 1-10 = detailed)

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Init())
			return;

		Print("Floaters Fixer - Run method started", LogLevel.NORMAL);

		IEntity entity;
		IEntitySource entitySource;
		BaseContainerList editorData;
		BaseContainer firstEditorData;
		vector entityPos, entityPosOffset, entityYawPitchRoll, normalAngles, randomVerticalOffset;
		float altitude, terrainY;
		bool alignToSurfaceNormal;
		TraceParam traceParam = new TraceParam();
		float traceRatio;
		float minVerticalOffset, maxVerticalOffset, maxPitch, maxRoll;

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();

		int selectedEntitiesCount = worldEditorAPI.GetSelectedEntitiesCount();
		bool manyEntities = selectedEntitiesCount > MANY_ENTITIES_THRESHOLD;
		RandomGenerator randomGenerator = new RandomGenerator();

		if (selectedEntitiesCount < 1)
			return;

		int firstTick = System.GetTickCount();
		worldEditorAPI.BeginEntityAction();
		for (int i; i < selectedEntitiesCount; i++)
		{
			entitySource = worldEditorAPI.GetSelectedEntity(i);
			if (!entitySource)
				continue;

			entity = worldEditorAPI.SourceToEntity(entitySource);
			editorData = null;
			alignToSurfaceNormal = m_bAlignToNormalOverride;
			minVerticalOffset = 0;
			maxVerticalOffset = 0;
			maxPitch = 0;
			maxRoll = 0;

			editorData = entitySource.GetObjectArray("editorData");
			if (editorData && editorData.Count() /* > 0 */)
			{
				firstEditorData = editorData.Get(0);

				randomVerticalOffset = vector.Zero;
				firstEditorData.Get("randomVertOffset", randomVerticalOffset);
				minVerticalOffset = randomVerticalOffset[0];
				maxVerticalOffset = randomVerticalOffset[1];

				firstEditorData.Get("randomPitchAngle", maxPitch);
				firstEditorData.Get("randomRollAngle", maxRoll);

				if (!m_bAlignToNormalOverride && m_bUsePrefabAlignToNormal)
					firstEditorData.Get("alignToNormal", alignToSurfaceNormal);
			}

			// --------------------------------------------------
			// get terrainY
			// if alignToNormal or if aboveEntities
			//     trace
			//     set new Y
			//     set normal
			//     if alignToNormal
			//         align to normal
			// set pos
			// --------------------------------------------------

			// origin
			entityPosOffset = vector.Zero;
			entityPos = entity.GetOrigin();
			entityPosOffset = entityPos - SCR_BaseContainerTools.GetLocalCoords(entitySource.GetParent(), entityPos);

			// altitude
			terrainY = worldEditorAPI.GetTerrainSurfaceY(entityPos[0], entityPos[2]);
			altitude = entityPos[1] - terrainY;
			if (altitude < 0)
				entityPos[1] = terrainY;

			// trace if alignToNormal or Entities check
			if (m_bUsePrefabAlignToNormal || alignToSurfaceNormal || m_bSetOnTerrainAndEntitiesSurface)
			{
				traceParam.Exclude = entity;

				if (m_bSetOnTerrainAndEntitiesSurface)
				{
					traceParam.Start = { entityPos[0], entityPos[1] + m_iTraceOriginDistance, entityPos[2] };
					traceParam.End = { entityPos[0], terrainY, entityPos[2] };
					traceParam.Flags = TraceFlags.ENTS | TraceFlags.WORLD;

					traceRatio = m_BaseWorld.TraceMove(traceParam, FilterNonVegetationCallback);

					if (!manyEntities)
					{
						Print("----- Tracing -----", LogLevel.NORMAL);
						Print(string.Format(".OBJ %1", entityPos), LogLevel.NORMAL);
						Print(string.Format("FROM %1", traceParam.Start), LogLevel.NORMAL);
						Print(string.Format("..TO %1", traceParam.End), LogLevel.NORMAL);
						Print(string.Format("DIST %1", traceParam.Start[1] - terrainY), LogLevel.NORMAL);
						Print(string.Format("DONE %1pct (%2m)", Math.Round(traceRatio * 10000) * 0.01, (traceParam.Start[1] - terrainY) * traceRatio), LogLevel.NORMAL);
					}
				}
				else // terrain only
				{
					traceParam.Start = { entityPos[0], terrainY + 1, entityPos[2] };
					traceParam.End = { entityPos[0], terrainY - 1, entityPos[2] };
					traceParam.Flags = TraceFlags.WORLD;

					traceRatio = m_BaseWorld.TraceMove(traceParam, null);
				}

				if (alignToSurfaceNormal)
				{
					normalAngles = GetXYZAnglesFromNormal(entity, traceParam.TraceNorm);
					worldEditorAPI.SetVariableValue(entitySource, null, "angles", normalAngles.ToString(false));
				}

				if (m_bSetOnTerrainAndEntitiesSurface)
					entityPos[1] = traceParam.Start[1] - (traceParam.Start[1] - traceParam.End[1]) * traceRatio;
				else
					entityPos[1] = terrainY;
			}

			entityPos -= entityPosOffset;

			// ATL vs World
			if (entity.GetFlags() & EntityFlags.RELATIVE_Y)
				entityPos[1] = entityPos[1] - terrainY;

			if (m_bUsePrefabVerticalOffset && (minVerticalOffset != 0 || maxVerticalOffset != 0))
				entityPos[1] = entityPos[1] + randomGenerator.RandFloatXY(minVerticalOffset, maxVerticalOffset);

			worldEditorAPI.SetVariableValue(entitySource, null, "coords", entityPos.ToString(false));
		}
		worldEditorAPI.EndEntityAction();

		Print(string.Format("Fixed %1 floating entities in %2ms", selectedEntitiesCount, System.GetTickCount() - firstTick), LogLevel.NORMAL);
		Print("Floaters Fixer - Run method ended", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Init()
	{
		if (!SCR_Global.IsEditMode())
		{
			Print("Floaters Fixer - Run method stopped because non-Workbench run", LogLevel.NORMAL);
			return false;
		}

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
		{
			Print("Floaters Fixer - Run method stopped because… World Editor is unavailable from World Editor? o_o", LogLevel.WARNING);
			return false;
		}

		if (worldEditor.IsPrefabEditMode())
		{
			Print("Floaters Fixer - Run method stopped because World Editor is in Prefab edit mode", LogLevel.NORMAL);
			return false;
		}

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
		{
			Print("Floaters Fixer - Run method stopped because World Editor API was not found", LogLevel.WARNING);
			return false;
		}

		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		if (!baseWorld)
		{
			Print("Floaters Fixer - Run method stopped because base world was not found", LogLevel.WARNING);
			return false;
		}

		m_BaseWorld = baseWorld;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterNonVegetationCallback(notnull IEntity entity, vector start = "0 0 0", vector dir = "0 0 0")
	{
		if (entity.IsInherited(GenericTerrainEntity))
			return true;

		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		IEntitySource source = worldEditorAPI.EntityToSource(entity);
		if (!source)
			return false;

		string resourceName = SCR_BaseContainerTools.GetTopMostAncestor(source).GetResourceName();
		return
			resourceName != BUSH_BASE &&
			resourceName != TREE_BASE;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetXYZAnglesFromNormal(IEntity entity, vector normal)
	{
		// TODO
		vector worldTransform[4];
		entity.GetWorldTransform(worldTransform);
		Math3D.DirectionAndUpMatrix(worldTransform[2], normal, worldTransform);

		// Print("normal = " + normal, LogLevel.DEBUG);
		// Print("angles = " + Math3D.MatrixToAngles(worldTransform), LogLevel.DEBUG);

		return normal;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
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
