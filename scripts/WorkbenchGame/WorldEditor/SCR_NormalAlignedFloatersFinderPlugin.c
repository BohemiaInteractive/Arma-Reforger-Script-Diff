#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Normal-Aligned Floaters Finder",
	category: SCR_PluginCategory.WORLDEDITOR_ENTITY_CHECK,
	description: "Find entities that are not properly aligned to terrain's normal",
	shortcut: "Ctrl+Alt+Shift+PageUp",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0xE09A)]
class SCR_NormalAlignedFloatersFinderPlugin : WorldEditorPlugin
{
	/*
		Category: Search
	*/

	[Attribute(defvalue: "", desc: "Which Prefabs should be checked for proper normal alignment (exact Prefab match)", category: "Search")]
	protected ref array<ResourceName> m_aPrefabsToCheck;

	[Attribute(defvalue: "0", desc: "Look for misplaced vegetation (trees & bushes) - does not impact searching by above Prefabs", category: "Search")]
	protected bool m_bSearchForVegetation;

	[Attribute(defvalue: "-0.1", desc: "Maximum distance the entity or its bounding box corners can be BELOW terrain", params: "-inf 0 0.01", precision: 2, category: "Search")] // max 1 as tracing is set to go max 1m
	protected float m_fNegativeToleranceFromTerrain;

	[Attribute(defvalue: "0.1", desc: "Maximum distance the entity or its bounding box corners can be ABOVE terrain", params: "0 inf 0.01", precision: 2, category: "Search")] // max 1 as tracing is set to go max 1m
	protected float m_fPositiveToleranceFromTerrain;

	[Attribute(defvalue: "1", desc: "Only check for entities that are above water (ocean) level, whether above or below terrain level", category: "Search")]
	protected bool m_bAboveWaterLevelOnly;

	[Attribute(defvalue: "5", uiwidget: UIWidgets.Slider, desc: "Maximum angle the entity can have from terrain slope [degrees]", params: "0 90 0.01", precision: 2, category: "Search")]
	protected float m_fMaxAngleFromTerrainNormal;

	[Attribute(defvalue: (Math.Sqrt(2) * 0.5).ToString(), desc: "Bounding box lower corners are checked - this multiplier makes checked corners go from the centre to the real bounding box corners\n- 0.5 = half X/Z\n- 0.707 = approximate ellipsis \"corner\"\n- 1 = bounding box exact corner positions", params: "0.25 1 0.001", category: "Search")]
	protected float m_fBoundingBoxMultiplier;

	/*
		Category: Selection
	*/

	[Attribute(defvalue: "1", desc: "Current layer only", category: "Selection")]
	protected bool m_bCurrentLayerOnly;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.Slider, desc: "Search in a radius around camera [m]\n0 = search whole terrain", params: "0 5000 100", category: "Selection")]
	protected float m_fMaxDistanceFromCamera;

//	[Attribute(defvalue: SCR_ESelectionBrushToolLayer.ALL_LAYERS.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ESelectionBrushToolLayer, category: "Selection")]
//	protected SCR_ESelectionBrushToolLayer m_eLayerSelection;

//	[Attribute(defvalue: "0", desc: "Select the topmost 3D parent, otherwise select the 3D world entity", category: "Selection")]
//	protected bool m_bSelectParentOnly;

	[Attribute(defvalue: "2000", desc: "Performance-related selection limit - 0 for no limit", uiwidget: UIWidgets.Slider, params: "0 10000 100", category: "Selection")]
	protected int m_iMaxSelectedEntities;

	protected static const int MAX_SUS_ENTITIES_LISTED = 10;
	protected static const float MAX_ENTITY_NORMAL_DIFFERENCE = 90; //!< in degrees

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		if (Workbench.ScriptDialog("Normal-Aligned Floaters Finder", "Please fill Prefabs To Check below - if left empty, this plugin will scan all world entities", this) == 0)
			return;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		BaseWorld baseWorld = worldEditorAPI.GetWorld();
		vector mins, maxs;
		baseWorld.GetBoundBox(mins, maxs);

		int worldEntitiesCount = worldEditorAPI.GetEditorEntityCount();
		if (worldEntitiesCount < 1)
		{
			Workbench.Dialog("Info", "No world entities were found");
			return;
		}

		FilterPrefabs();

		if (!m_bCurrentLayerOnly && m_aPrefabsToCheck.IsEmpty())
		{
			if (!Workbench.ScriptDialog("Warning", "There are no Prefabs to check in the list - continuing will scan all " + worldEntitiesCount + " world entities!", new WorkbenchDialog_OKCancel()))
				return;
		}

		Debug.BeginTimeMeasure();
		array<IEntitySource> susEntitySources = GetSusEntitySources();
		Debug.EndTimeMeasure(string.Format("Found %1 floaters amongst %2 entities", susEntitySources.Count(), worldEntitiesCount));

		OutputEntities(susEntitySources);
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
	//! \return suspect entities, empty array if everything is fine - never returns null
	protected array<IEntitySource> GetSusEntitySources()
	{
		bool scanAllEntities = m_aPrefabsToCheck.IsEmpty();

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
		vector corners[4];

		array<IEntitySource> result = {};

		int currentLayerId;
		if (m_bCurrentLayerOnly)
			currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();

		bool scanInRadius = m_fMaxDistanceFromCamera > 0;
		float radiusSq;
		vector cameraPos;
		if (scanInRadius)
		{
			cameraPos = SCR_WorldEditorToolHelper.GetWorldEditorCameraPosition();
			radiusSq = m_fMaxDistanceFromCamera * m_fMaxDistanceFromCamera;
		}

		IEntity entity;
		IEntitySource entitySource;
		IEntitySource ancestor;

		int count = worldEditorAPI.GetEditorEntityCount();

		for (int i; i < count; ++i)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!entitySource)
				continue;

			if (m_bCurrentLayerOnly && entitySource.GetLayerID() != currentLayerId)
				continue;

			ancestor = entitySource.GetAncestor();
			if (!ancestor)
				continue;

			ResourceName resourceName = ancestor.GetResourceName();
			if (!scanAllEntities && !m_aPrefabsToCheck.Contains(resourceName))
				continue;

			entity = worldEditorAPI.SourceToEntity(entitySource);
			if (!entity)
				continue;

			if (!m_bSearchForVegetation && Tree.Cast(entity) != null)
				continue;

			vector entityPos = entity.GetOrigin();
			if (doOceanCheck && entityPos[1] < oceanLevel)
				continue;

			if (scanInRadius && vector.DistanceSq(cameraPos, entityPos) > radiusSq)
				continue; // outside range

			// no bounding box, continue
			vector mins, maxs;
			entity.GetBounds(mins, maxs);
			if (mins == vector.Zero && maxs == vector.Zero)
				continue;

			float surfaceY = baseWorld.GetSurfaceY(entityPos[0], entityPos[2]);

			// is close to terrain?
			float diffY = entityPos[1] - surfaceY;
			if (m_fNegativeToleranceFromTerrain > diffY || diffY > m_fPositiveToleranceFromTerrain) // Math.Abs costs more than that
			{
				result.Insert(entitySource);
				continue;
			}

			// is terrain-aligned?
			traceParam.Start = { entityPos[0], surfaceY + 1, entityPos[2] };
			traceParam.End = { entityPos[0], surfaceY - 1, entityPos[2] };

			if (baseWorld.TraceMove(traceParam, null) >= 1) // did not hit terrain
				continue;

			vector terrainNormal = traceParam.TraceNorm;
			if (terrainNormal == vector.Zero) // something wrong
				continue;

			vector absEntityUpVector = vector.Direction(entityPos, entity.CoordToParent(vector.Up)).Normalized();
			if (m_fMaxAngleFromTerrainNormal < Math.Acos(vector.Dot(terrainNormal, absEntityUpVector)) * Math.RAD2DEG)
			{
				result.Insert(entitySource); // angled enough
				continue;
			}

			mins *= m_fBoundingBoxMultiplier;
			maxs *= m_fBoundingBoxMultiplier;

			corners[0] = entity.CoordToParent({ mins[0], 0, maxs[2] });
			corners[1] = entity.CoordToParent({ maxs[0], 0, maxs[2] });
			corners[2] = entity.CoordToParent({ mins[0], 0, mins[2] });
			corners[3] = entity.CoordToParent({ maxs[0], 0, mins[2] });

			for (int j; j < 4; ++j)
			{
				diffY = corners[j][1] - baseWorld.GetSurfaceY(corners[j][0], corners[j][2]);

				if (m_fNegativeToleranceFromTerrain > diffY || diffY > m_fPositiveToleranceFromTerrain) // Math.Abs costs more than that
				{
					result.Insert(entitySource);
					break;
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entitySources the entity sources to output/select
	protected void OutputEntities(notnull array<IEntitySource> entitySources)
	{
		int entitiesCount = entitySources.Count();
		if (entitiesCount == 1)
			Print("1 suspect entity", level: LogLevel.NORMAL);
		else
			PrintFormat("%1 suspect entities", entitiesCount, level: LogLevel.NORMAL);

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		worldEditorAPI.ClearEntitySelection();

		bool ellipsisPrinted = m_iMaxSelectedEntities < 1;
		int entitiesToSelect = m_iMaxSelectedEntities;
		foreach (int i, IEntitySource entitySource : entitySources)
		{
			if (!ellipsisPrinted)
			{
				if (i < MAX_SUS_ENTITIES_LISTED)
				{
					Print("Suspect entity " + Debug.GetEntityLinkString(worldEditorAPI.SourceToEntity(entitySource)), level: LogLevel.NORMAL);
				}
				else
				{
					Print("(...)", LogLevel.NORMAL);
					ellipsisPrinted = true;
				}
			}

			worldEditorAPI.AddToEntitySelection(entitySource);
			if (m_iMaxSelectedEntities > 0 && --entitiesToSelect < 1)
			{
				Print("Maximum selection reached (" + m_iMaxSelectedEntities + ")", LogLevel.WARNING);
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

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}
#endif
