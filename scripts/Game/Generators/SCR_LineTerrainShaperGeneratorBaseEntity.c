class SCR_LineTerrainShaperGeneratorBaseEntityClass : SCR_LineGeneratorBaseEntityClass
{
}

// TODO: terrain shaping's line offset support
class SCR_LineTerrainShaperGeneratorBaseEntity : SCR_LineGeneratorBaseEntity
{
	/*
		Terrain Sculpting
	*/

	[Attribute(defvalue: "0", desc: "Adjust terrain's Y around the track to fit the spline's location and clearance", category: "Terrain Sculpting")]
	protected bool m_bSculptTerrain;

	[Attribute(defvalue: "0", desc: "Priority of terrain sculpting", category: "Terrain Sculpting")]
	protected int m_iTerrainSculptingPriority;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Width of the path before fall-off, the flat surface around the shape", category: "Terrain Sculpting", params: "0 100 0.1")]
	protected float m_fTerrainSculptingPathWidth;

	[Attribute(defvalue: "20", uiwidget: UIWidgets.EditBox, desc: "Width of the path fall-off", category: "Terrain Sculpting", params: "0 100 0.1")]
	protected float m_fTerrainSculptingFallOffWidth;

#ifdef WORKBENCH

	//! Read by WorldEditorAPI.AddTerrainFlatterEntity, CANNOT be renamed
	//! defined at the last moment in UpdateTerrainSimple from m_ParentShapeSource / m_OffsetShapeSource
	protected ShapeEntity m_ShapeEntity;

	protected static const ref array<string> TERRAIN_UPDATE_KEYS = {
		"m_bSculptTerrain", "m_iTerrainSculptingPriority",
		"m_fTerrainSculptingPathWidth", "m_fTerrainSculptingFallOffWidth",
	};
	protected static const string OFFSET_SHAPE_COLOUR = "1 0 1 1"; // RGBA - magic pink (#F0F)

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeTransformInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeTransformInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		UpdateTerrainSimple(false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		UpdateTerrainSimple(false);
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring()) // sorry Ctrl+Z, but you cannot act here
			return false;

		if (!worldEditorAPI.AreGeneratorEventsEnabled())
			return false;

		BaseContainerTools.WriteToInstance(this, src);

		if (TERRAIN_UPDATE_KEYS.Contains(key))
		{
			bool forceUpdate;
			if (key == "m_bSculptTerrain")
			{
				if (m_bSculptTerrain)
				{
					forceUpdate = true;
				}
				else
				{
					worldEditorAPI.RemoveTerrainFlatterEntity(this, true);

//					// delete offset shape
//					IEntitySource offsetShapeSource;
//					for (int i, count = m_ParentShapeSource.GetNumChildren(); i < count; ++i)
//					{
//						offsetShapeSource = m_ParentShapeSource.GetChild(i);
//						if (offsetShapeSource.GetClassName().ToType() && offsetShapeSource.GetClassName().ToType().IsInherited(ShapeEntity))
//						{
//							worldEditorAPI.DeleteEntity(offsetShapeSource);
//							break;
//						}
//					}
				}
			}

			UpdateTerrainSimple(forceUpdate);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src)
	{
		super._WB_OnCreate(src);

		UpdateTerrainSimple();
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		if (m_ParentShapeSource)
			UpdateTerrainSimple(true);
		else
			worldEditorAPI.RemoveTerrainFlatterEntity(this, true);
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnDelete(IEntitySource src)
	{
		super._WB_OnDelete(src);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		worldEditorAPI.RemoveTerrainFlatterEntity(this, true);
	}

	//------------------------------------------------------------------------------------------------
	//! wrapper method around UpdateTerrain that sets m_ShapeEntity up
	//! \param[in] forceUpdate WorldEditorAPI.AddTerrainFlatterEntity parameter
	protected void UpdateTerrainSimple(bool forceUpdate = false)
	{
		if (!m_bSculptTerrain || !m_ParentShapeSource)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

//		IEntitySource offsetShapeSource;
//		for (int i, count = m_ParentShapeSource.GetNumChildren(); i < count; ++i)
//		{
//			offsetShapeSource = m_ParentShapeSource.GetChild(i);
//			if (offsetShapeSource.GetClassName().ToType() && offsetShapeSource.GetClassName().ToType().IsInherited(ShapeEntity))
//				break;
//
//			offsetShapeSource = null;
//		}
//
//		if (offsetShapeSource)
//			worldEditorAPI.DeleteEntity(offsetShapeSource);
//
//		offsetShapeSource = worldEditorAPI.CreateEntity(m_ParentShapeSource.GetClassName(), string.Empty, 0, m_ParentShapeSource, vector.Zero, vector.Zero);
//		if (!offsetShapeSource)
//			return;
//
//		worldEditorAPI.SetVariableValue(offsetShapeSource, null, "LineColor", OFFSET_SHAPE_COLOUR);
//		foreach (int i, vector anchorPoint : m_ShapeNextPointHelper.GetAnchorPoints())
//		{
//			worldEditorAPI.CreateObjectArrayVariableMember(offsetShapeSource, null, "Points", "ShapePoint", i);
//			worldEditorAPI.SetVariableValue(offsetShapeSource, { new ContainerIdPathEntry("Points", i) }, "Position", string.Format("%1 %2 %3", anchorPoint[0], anchorPoint[1], anchorPoint[2]));
//		}
//
//		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(offsetShapeSource));
		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (shapeEntity)
		{
			array<vector> updateMins = {};
			array<vector> updateMaxes = {};
//			shapeEntity.GetAllInfluenceBBoxes(offsetShapeSource, updateMins, updateMaxes);
			shapeEntity.GetAllInfluenceBBoxes(m_ParentShapeSource, updateMins, updateMaxes);

			UpdateTerrain(shapeEntity, forceUpdate, updateMins, updateMaxes);
		}

//		worldEditorAPI.DeleteEntity(offsetShapeSource);
	}

	//------------------------------------------------------------------------------------------------
	// where everything happens
	//! \param[in] shapeEntity the shape to follow for terrain update - is assigned to m_ShapeEntity as C++ reads this variable ("hardcoded" C++-side)
	//! \param[in] forceUpdate
	//! \param[in] updateMins
	//! \param[in] updateMaxes
	protected void UpdateTerrain(notnull ShapeEntity shapeEntity, bool forceUpdate, notnull array<vector> updateMins, notnull array<vector> updateMaxes)
	{
		m_ShapeEntity = shapeEntity;

		if (!m_bSculptTerrain)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		array<vector> points = {};
		shapeEntity.GenerateTesselatedShape(points);

		if (points.Count() < 2)
		{
			Print("Terrain Flattener requires a shape with at least two points", LogLevel.WARNING);
			return;
		}

		vector firstPoint = shapeEntity.CoordToParent(points[0]);
		vector mins = firstPoint;
		vector maxs = firstPoint;

		foreach (vector point : points)
		{
			vector pos = shapeEntity.CoordToParent(point);
			for (int i = 0; i < 3; ++i)
			{
				float val = pos[i];

				if (val < mins[i])
					mins[i] = val;

				if (val > maxs[i])
					maxs[i] = val;
			}
		}

		worldEditorAPI.AddTerrainFlatterEntity(this, mins, maxs, m_iTerrainSculptingPriority, m_fTerrainSculptingPathWidth * 0.5, m_fTerrainSculptingFallOffWidth, forceUpdate, updateMins, updateMaxes);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_LineTerrainShaperGeneratorBaseEntity(IEntitySource src, IEntity parent)
	{
		if (!_WB_GetEditorAPI())
			return;

		m_ShapeEntity = ShapeEntity.Cast(parent);
	}
#endif // WORKBENCH
}
