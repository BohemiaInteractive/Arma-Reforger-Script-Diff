#ifdef WORKBENCH
[WorkbenchToolAttribute(name: "Normal-Aligned Floaters Finder", category: "Object Placement", description: "<Description>", shortcut: "", awesomeFontCode: 0xE09A)]
class SCR_NormalAlignedFloatersFinderPlugin : WorldEditorPlugin
{
	/*
		Category: Search
	*/

	[Attribute(defvalue: "", desc: "Which Prefabs should be checked for proper normal alignment (exact Prefab match)", category: "Search")]
	protected ref array<ResourceName> m_aPrefabsToCheck;

	[Attribute(defvalue: "0", desc: "Look for misplaced vegetation (trees & bushes) - does not impact searching by above Prefabs", category: "Search")]
	protected bool m_bSearchForVegetation;

	[Attribute(defvalue: "0.1", desc: "Maximum distance from the terrain the entity can be", params: "0 1 0.01", precision: 2, category: "Search")] // max 1 as tracing is set to go max 1m
	protected float m_fToleranceFromTerrain;

	[Attribute(defvalue: "1", desc: "Only check for entities that are above water (ocean) level, whether above or below terrain level", category: "Search")]
	protected bool m_bAboveWaterLevelOnly;

	[Attribute(defvalue: "0.01", desc: "Tolerance within which two vectorUps are considered close enough (0.707 = 45°)", params: "0.01 0.707 0.001", category: "Search")]
	protected float m_fVectorUpTolerance;

	[Attribute(defvalue: "0.707", desc: "Bounding box lower corners are checked - this multiplier makes checked corners go from the centre to the real bounding box corners\n- 0.5 = half X/Z\n- 0.707 = approximate ellipsis \"corner\"\n- 1 = bounding box corner positions", params: "0.25 1 0.001", category: "Search")]
	protected float m_fBoundingBoxMultiplier;

	/*
		Category: Selection
	*/

//	[Attribute(defvalue: SCR_ESelectionBrushToolLayer.ALL_LAYERS.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ESelectionBrushToolLayer, category: "Selection")]
//	SCR_ESelectionBrushToolLayer m_eLayerSelection;

//	[Attribute(defvalue: "0", desc: "Select the topmost 3D parent, otherwise select the 3D world entity", category: "Selection")]
//	protected bool m_bSelectParentOnly;

	[Attribute(defvalue: "2000", desc: "Performance-related selection limit - 0 for no limit", uiwidget: UIWidgets.Slider, params: "0 10000 100", category: "Selection")]
	protected int m_iMaxSelectedEntities;

	protected static const int MAX_SUS_ENTITIES_LISTED = 10;

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (Workbench.ScriptDialog("Normal-Aligned Floaters Finder", "Please fill Prefabs To Check below - if left empty, this plugin will scan all world entities", this) == 0)
			return;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		vector mins, maxs;
		baseWorld.GetBoundBox(mins, maxs);
		array<IEntity> worldEntities = SCR_WorldEditorToolHelper.QueryEntitiesByAABB(baseWorld, mins, maxs);

		int worldEntitiesCount = worldEntities.Count();
		if (worldEntitiesCount < 1)
		{
			Workbench.Dialog("Info", "No world entities were found");
			return;
		}

		FilterPrefabs();

		if (m_aPrefabsToCheck.IsEmpty())
		{
			if (!Workbench.ScriptDialog("Warning", "There are no Prefabs to check in the list - continuing will scan all " + worldEntitiesCount + " world entities!", new SCR_OKCancelWorkbenchDialog()))
				return;
		}

		Debug.BeginTimeMeasure();
		array<IEntity> floatingEntities = ProcessEntities(worldEntities);
		Debug.EndTimeMeasure(string.Format("Found %1 floaters amongst %2 entities", floatingEntities.Count(), worldEntitiesCount));

		OutputEntities(floatingEntities);
	}

	//------------------------------------------------------------------------------------------------
	//! Clears up m_aPrefabsToCheck from empty entries and duplicates
	protected void FilterPrefabs()
	{
		int count = m_aPrefabsToCheck.Count();
		for (int i = count - 1; i >= 0; --i)
		{
			if (!m_aPrefabsToCheck[i] || m_aPrefabsToCheck.Find(m_aPrefabsToCheck[i]) != i)
				m_aPrefabsToCheck.RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntity> ProcessEntities(notnull array<IEntity> entities)
	{
		bool scanAllEntities = m_aPrefabsToCheck.IsEmpty();
		int worldEntitiesCount = entities.Count();
		// map<IEntity, ResourceName> susEntityPrefabMap = new map<IEntity, ResourceName>();

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		bool doOceanCheck = m_bAboveWaterLevelOnly;

		float oceanLevel;
		if (doOceanCheck)
		{
			if (baseWorld.IsOcean())
				oceanLevel = baseWorld.GetOceanBaseHeight();
			else
				doOceanCheck = false;
		}

		TraceParam traceParam = new TraceParam();
		traceParam.Flags = TraceFlags.WORLD;

		// vector cornerFLWorld; // 1998
		// vector cornerFRWorld; // France
		// vector cornerBLWorld; // 3 - 0
		// vector cornerBRWorld; // Brazil
		vector cornerTraceStarts[4];
		vector cornerTraceEnds[4];

		array<IEntity> result = {};

		IEntitySource ancestor;
		foreach (IEntity entity : entities)
		{
			if (!m_bSearchForVegetation && Tree.Cast(entity) != null)
				continue;

			ancestor = worldEditorAPI.EntityToSource(entity);
			if (!ancestor)
				continue;

			ancestor = ancestor.GetAncestor();
			if (!ancestor)
				continue;

			ResourceName resourceName = ancestor.GetResourceName();
			if (!scanAllEntities && !m_aPrefabsToCheck.Contains(resourceName))
				continue;

			vector entityPos = entity.GetOrigin();
			if (doOceanCheck && entityPos[1] < oceanLevel)
				continue;

			// no bounding box, continue
			vector mins, maxs;
			entity.GetBounds(mins, maxs);
			if (mins == vector.Zero && maxs == vector.Zero)
				continue;

			float surfaceY = baseWorld.GetSurfaceY(entityPos[0], entityPos[2]);

			// is close to terrain?
			float diffY = entityPos[1] - surfaceY;
			if (m_fToleranceFromTerrain < diffY || diffY < -m_fToleranceFromTerrain) // Math.Abs costs more than that
				continue;

			// is terrain-aligned?
			traceParam.Start = { entityPos[0], surfaceY + 1, entityPos[2] };
			traceParam.End = { entityPos[0], surfaceY - 1, entityPos[2] };

			if (baseWorld.TraceMove(traceParam, null) == 1) // did not hit terrain
				continue;

			vector terrainNormal = traceParam.TraceNorm;
			if (terrainNormal == vector.Zero) // something wrong
				continue;

			// compare vectorUps
			if (vector.Distance(terrainNormal, (entity.CoordToParent(vector.Up) - entityPos).Normalized()) > m_fVectorUpTolerance)
				continue; // not close enough

			mins *= m_fBoundingBoxMultiplier;
			maxs *= m_fBoundingBoxMultiplier;

			cornerTraceStarts[0] = entity.CoordToParent({ mins[0], 1, maxs[2] });
			cornerTraceStarts[1] = entity.CoordToParent({ maxs[0], 1, maxs[2] });
			cornerTraceStarts[2] = entity.CoordToParent({ mins[0], 1, mins[2] });
			cornerTraceStarts[3] = entity.CoordToParent({ maxs[0], 1, mins[2] });

			cornerTraceEnds[0] = entity.CoordToParent({ mins[0], -1, maxs[2] });
			cornerTraceEnds[1] = entity.CoordToParent({ maxs[0], -1, maxs[2] });
			cornerTraceEnds[2] = entity.CoordToParent({ mins[0], -1, mins[2] });
			cornerTraceEnds[3] = entity.CoordToParent({ maxs[0], -1, mins[2] });

			for (int i; i < 4; ++i)
			{
				traceParam.Start = cornerTraceStarts[i];
				traceParam.End = cornerTraceEnds[i];
				float cornerDistanceToTerrain = baseWorld.TraceMove(traceParam, null) * 2 - 1; // 2m = -1..+1
				if (m_fToleranceFromTerrain < cornerDistanceToTerrain || cornerDistanceToTerrain < -m_fToleranceFromTerrain) // Math.Abs costs more than that
				{
//					susEntityPrefabMap.Insert(entity, resourceName); // entity is definitely sus
					result.Insert(entity);
					break;
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] parameter
	protected void OutputEntities(notnull array<IEntity> entities)
	{
		int entitiesCount = entities.Count();
		if (entitiesCount == 1)
			Print("1 suspect entity", level: LogLevel.NORMAL);
		else
			PrintFormat("%1 suspect entities", entitiesCount, level: LogLevel.NORMAL);

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		worldEditorAPI.ClearEntitySelection();

		bool ellipsisPrinted = m_iMaxSelectedEntities < 1;
		int entitiesToSelect = m_iMaxSelectedEntities;
		IEntitySource entitySource;
		foreach (int i, IEntity entity : entities)
		{
			if (!ellipsisPrinted)
			{
				if (i < MAX_SUS_ENTITIES_LISTED)
				{
					PrintFormat("Suspect entity %2 - %1", entity.ClassName(), entity.GetOrigin(), level: LogLevel.NORMAL);
					// PrintFormat("Suspect entity @\"ENTITY:%4\" %3 - %1 %2", entity.ClassName(), resourceName, entity.GetOrigin(), worldEditorAPI.EntityToSource(entity).GetID(), level: LogLevel.NORMAL);
				}
				else
				{
					Print("(...)", LogLevel.NORMAL);
					ellipsisPrinted = true;
				}
			}

			entitySource = worldEditorAPI.EntityToSource(entity);
			if (entitySource)
			{
				worldEditorAPI.AddToEntitySelection(entitySource);
				if (m_iMaxSelectedEntities > 0 && --entitiesToSelect < 1)
					break;
			}
		}

		worldEditorAPI.UpdateSelectionGui();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Process", true)]
	protected int ButtonOK()
	{
		return 1;
	}

	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif
