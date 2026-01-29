class SCR_GeneratorBaseEntityClass : GeneratorBaseEntityClass
{
	[Attribute()]
	ref Color m_Color;
}

//! SCR_GeneratorBaseEntity responsibilities:
//! - trigger a warning if the generator is not the child of a shape
//! - keep the generator at shape's {0,0,0}, at angles {0,0,0}, at scale 1
//! - delete all child entities if no parent is set
//! - set generator's and shape's "Editor Only" flag
//! - provide methods to get offset shapes through SCR_ParallelShapeHelper usage (for parallel lines or smaller/bigger areas)
//! - manage the m_Source variable
//! - manage the m_ParentShapeSource variable
//! - manage the m_iSourceLayerID variable
//! - carry a debug shape manager reference
class SCR_GeneratorBaseEntity : GeneratorBaseEntity
{
	/*
		Generation
	*/

	[Attribute(defvalue: "1", category: "Generation", desc: "Allow entities generation by this generator - useful in order to adjust shape and settings without generating unwanted entities\nKeep it ticked before saving!\nNote: does not prevent terrain shaping and other generator features")]
	protected bool m_bEnableGeneration;

	[Attribute(defvalue: DEFAULT_SEED.ToString(), category: "Generation", desc: "Randomisation Seed used by this generator", params: string.Format("%1 %2", MIN_RANDOM_SEED, MAX_RANDOM_SEED))]
	protected int m_iSeed;

	[Attribute(defvalue: "1", category: "Generation", desc: "Randomise the above seed every change")]
	protected bool m_bRandomiseSeedOnUse;

	// these consts must be above ifdef as they are used in attributes
	protected static const int DEFAULT_SEED = 42;
	protected static const int MIN_RANDOM_SEED = 0;
	protected static const int MAX_RANDOM_SEED = 0x7FFF; // 32767

#ifdef WORKBENCH

	protected IEntitySource m_Source;				//!< the generator's entity source
	protected IEntitySource m_ParentShapeSource;	//!< the parent shape's entity source, if any - must be managed ONLY by SCR_GeneratorBaseEntity
	protected int m_iSourceLayerID;

	protected ref RandomGenerator m_RandomGenerator;

	protected bool m_bIsChangingWorkbenchKey;

	protected static const int BASE_GENERATOR_COLOUR = Color.WHITE;
	//------------------------------------------------------------------------------------------------
	override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		m_Source = src;
		m_ParentShapeSource = m_Source.GetParent();
		if (!m_ParentShapeSource
			|| !m_ParentShapeSource.GetClassName().ToType()
			|| !m_ParentShapeSource.GetClassName().ToType().IsInherited(ShapeEntity))
			DeleteAllChildren();
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent); // true or false does not matter here

		if (m_bIsChangingWorkbenchKey)
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return true;

		m_ParentShapeSource = GetShapeSource();
		bool sameParentChange = parent && worldEditorAPI.EntityToSource(parent) == m_ParentShapeSource;

		// keep the scale at 1
		if (key == "scale")
		{
			bool isSetDirectly = src.IsVariableSetDirectly("scale");

			if (sameParentChange)
				Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

			m_bIsChangingWorkbenchKey = true;
			if (isSetDirectly)
			{
				worldEditorAPI.ClearVariableValue(src, null, "scale");
			}
			else
			{
				worldEditorAPI.SetVariableValue(src, null, "scale", "1");
				Print("Generator has scale set in Prefab! " + src.GetResourceName(), LogLevel.WARNING);
			}

			m_bIsChangingWorkbenchKey = false;
		}

		// keep the angles at 0
		if (key == "angles")
		{
			bool isSetDirectly = src.IsVariableSetDirectly("angles");

			if (sameParentChange)
				Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

			m_bIsChangingWorkbenchKey = true;
			if (isSetDirectly)
			{
				worldEditorAPI.ClearVariableValue(src, null, "angles");
			}
			else // angleValue != 0
			{
				worldEditorAPI.SetVariableValue(src, null, "angles", "0 0 0");
				Print("Generator has angles set in Prefab! " + src.GetResourceName(), LogLevel.WARNING);
			}

			m_bIsChangingWorkbenchKey = false;
		}

		// keep the generator at (relative) 0 0 0 as long as it has a parent
		// if no parent, do not set to 0 0 0
		// do not warn about no parent here as the constructor does it
		if (parent && key == "coords")
		{
			vector coords;
			src.Get("coords", coords);
			bool isSetDirectly = src.IsVariableSetDirectly("coords");
			if (isSetDirectly || coords != vector.Zero) // because this can trigger when changing parent shape's points
			{
				if (sameParentChange)
					Print("Do not modify a generator entity itself! Change its parent shape instead", LogLevel.WARNING);

				m_bIsChangingWorkbenchKey = true;
				if (isSetDirectly)
				{
					worldEditorAPI.ClearVariableValue(src, null, "coords");
				}
				else
				{
					worldEditorAPI.SetVariableValue(src, null, "coords", "0 0 0");
					Print("Generator has coords set in Prefab! " + src.GetResourceName(), LogLevel.WARNING);
				}

				m_bIsChangingWorkbenchKey = false;
			}
		}

		ShapeEntity parentShape = ShapeEntity.Cast(parent);
		if (parentShape)
			m_ParentShapeSource = worldEditorAPI.EntityToSource(parentShape);
		else
			m_ParentShapeSource = null;

		// let's not save here for now
		// BaseContainerTools.WriteToInstance(this, worldEditorAPI.EntityToSource(this));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_CanSelect(IEntitySource src)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnIntersectingShapeChangedXZInternal(IEntitySource shapeEntitySrc, IEntitySource other, array<vector> mins, array<vector> maxes)
	{
		super.OnIntersectingShapeChangedXZInternal(shapeEntitySrc, other, mins, maxes);
		m_ParentShapeSource = shapeEntitySrc;
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);
		m_ParentShapeSource = shapeEntitySrc;
		ResetGeneratorPosition(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);
		m_ParentShapeSource = shapeEntitySrc;
		// no WorldEditorAPI operations here!
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetGeneratorPosition(ShapeEntity shapeEntity = null)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		vector coords;
		if (m_Source.IsVariableSetDirectly("coords"))
			worldEditorAPI.ClearVariableValue(m_Source, null, "coords");

		if (m_Source.Get("coords", coords) && coords != vector.Zero)
			Print("Generator has coords set in Prefab! " + m_Source.GetResourceName(), LogLevel.WARNING);

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Delete all children without distinction, using WorldEditorAPI
	protected void DeleteAllChildren()
	{
		int count = m_Source.GetNumChildren();
		if (count < 1)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		array<IEntitySource> entities = {};
		for (int i = m_Source.GetNumChildren() - 1; i >= 0; --i)
		{
			entities.Insert(m_Source.GetChild(i));
		}

		worldEditorAPI.DeleteEntities(entities);
	}

	//------------------------------------------------------------------------------------------------
	//! \return World EditorAPI or null if error
	protected static WorldEditorAPI GetWorldEditorAPI()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return null;

		return worldEditor.GetApi();
	}

	//------------------------------------------------------------------------------------------------
	[Obsolete("Use GetAnchorPoints instead")]
	protected static array<vector> GetPoints(notnull IEntitySource shapeEntitySrc)
	{
		return GetAnchorPoints(shapeEntitySrc);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntitySrc
	//! \param[in] offset offset from the current shape; negative = to the left, positive = to the right
	//! \param[in] isShapeClosed
	//! \return 3D anchor points relative to the provided shape source; or empty array if not a shape (never null)
	protected static array<vector> GetAnchorPoints(notnull IEntitySource shapeEntitySrc, float offset = 0, bool isShapeClosed = false)
	{
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (!points)
			return {};

		array<vector> result = {};
		vector pos;
		for (int i, count = points.Count(); i < count; ++i)
		{
			points.Get(i).Get("Position", pos);
			result.Insert(pos);
		}

		if (offset == 0)
			return result;
		else
			return SCR_ParallelShapeHelper.GetOffsetPointsFromPoints(result, offset, isShapeClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntitySrc
	//! \return 3D anchor points in absolute (world) coordinates or null if WorldEditorAPI is not available / source is not a shape
	protected static array<vector> GetWorldAnchorPoints(notnull IEntitySource shapeEntitySrc)
	{
		WorldEditorAPI worldEditorAPI = GetWorldEditorAPI();
		if (!worldEditorAPI)
			return null;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
		if (!shapeEntity)
			return null;

		array<vector> result = {};
		shapeEntity.GetPointsPositions(result);

		foreach (int i, vector point : result)
		{
			result[i] = shapeEntity.CoordToParent(point);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntitySrc
	//! \param[in] offset
	//! \param[in] isShapeClosed
	//! \return 3D points relative to the provided shape source or null if WorldEditorAPI is not available / source is not a shape
	protected static array<vector> GetTesselatedShapePoints(notnull IEntitySource shapeEntitySrc, float offset = 0, bool isShapeClosed = false)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
			return null;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
		if (!shapeEntity)
			return null;

		array<vector> result = {};
		shapeEntity.GenerateTesselatedShape(result);
		if (offset == 0)
			return result;
		else
			return SCR_ParallelShapeHelper.GetOffsetPointsFromPoints(result, offset, isShapeClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntitySrc
	//! \return 3D points in absolute (world) coordinates or null if WorldEditorAPI is not available / source is not a shape
	protected static array<vector> GetWorldTesselatedShapePoints(notnull IEntitySource shapeEntitySrc)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
			return null;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
		if (!shapeEntity)
			return null;

		array<vector> result = {};
		shapeEntity.GenerateTesselatedShape(result);

		foreach (int i, vector point : result)
		{
			result[i] = shapeEntity.CoordToParent(point);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] wantedType wanted data type - if all of them are wanted, use "ShapePointDataScriptBase"
	//! \return a map of <anchorIndex, dataPoint> - if there is no filtered data on one point, the index is not present in the map
	map<int, ref ShapePointDataScriptBase> GetFirstPointDataMap(typename wantedType = ShapePointDataScriptBase)
	{
		if (!m_ParentShapeSource)
			return null;

		BaseContainerList points = m_ParentShapeSource.GetObjectArray("Points");
		if (!points)
			return null;

		map<int, ref ShapePointDataScriptBase> result = new map<int, ref ShapePointDataScriptBase>();

		array<ref ShapePointDataScriptBase> pointDataArray;
		ShapePointDataScriptBase pointData;
		for (int i, pointsCount = points.Count(); i < pointsCount; ++i)
		{
			if (!points.Get(i).Get("Data", pointDataArray))
				continue;

			if (wantedType && wantedType != ShapePointDataScriptBase)
			{
				for (int j = 0, countJ = pointDataArray.Count(); j < countJ; ++j)
				{
					pointData = pointDataArray[j];
					if (pointData.IsInherited(wantedType))
					{
						result.Insert(i, pointData);
						break;
					}
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] wantedType wanted data type - if all of them are wanted, use "ShapePointDataScriptBase"
	//! \return a map of <anchorIndex, dataPoint> - if there is no filtered data on one point, the index is not present in the map
	map<int, ref array<ref ShapePointDataScriptBase>> GetPointDataMap(typename wantedType = ShapePointDataScriptBase)
	{
		if (!m_ParentShapeSource)
			return null;

		BaseContainerList points = m_ParentShapeSource.GetObjectArray("Points");
		if (!points)
			return null;

		map<int, ref array<ref ShapePointDataScriptBase>> result = new map<int, ref array<ref ShapePointDataScriptBase>>();

		array<ref ShapePointDataScriptBase> pointDataArray;
		for (int i, pointsCount = points.Count(); i < pointsCount; ++i)
		{
			if (!points.Get(i).Get("Data", pointDataArray))
				continue;

			if (wantedType && wantedType != ShapePointDataScriptBase)
			{
				for (int j = pointDataArray.Count() - 1; j >= 0; --j)
				{
					if (!pointDataArray[j].IsInherited(wantedType))
						pointDataArray.RemoveOrdered(j);
				}
			}

			if (!pointDataArray.IsEmpty())
				result.Insert(i, pointDataArray);

			pointDataArray = null; // required to generate a new array every Get("Data")
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// (Re)set the random generator's seed
	protected void SetSeed()
	{
		int wantedSeed;
		if (m_bRandomiseSeedOnUse)
			wantedSeed = Math.RandomInt(MIN_RANDOM_SEED, MAX_RANDOM_SEED);
		else
			wantedSeed = m_iSeed;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		IEntitySource source = worldEditorAPI.EntityToSource(this);
		IEntitySource ancestor = source.GetAncestor();
		int ancestorSeed;
		if (!ancestor || !ancestor.Get("m_iSeed", ancestorSeed))
			ancestorSeed = DEFAULT_SEED;

		m_bIsChangingWorkbenchKey = true;

		if (wantedSeed == ancestorSeed)
			worldEditorAPI.ClearVariableValue(source, null, "m_iSeed");
		else
			worldEditorAPI.SetVariableValue(source, null, "m_iSeed", wantedSeed.ToString());

		m_bIsChangingWorkbenchKey = false;

		m_RandomGenerator.SetSeed(wantedSeed);
		m_iSeed = wantedSeed;
	}

	//------------------------------------------------------------------------------------------------
	protected Color GetColor()
	{
		SCR_GeneratorBaseEntityClass prefabData = SCR_GeneratorBaseEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return Color.FromInt(BASE_GENERATOR_COLOUR);

		return Color.FromInt(prefabData.m_Color.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnCreate(IEntitySource src) // TODO: change event (_WB_OnInit?) beware of stack overflow by action edit
	{
		super._WB_OnCreate(src);

		ColorShape();

		// when generator entity gets created by any edit activity (given by _WB_OnCreate event) then re-generate its generated content
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		// re-generate content only if it's created with a parent
		IEntitySource shapeEntitySrc = src.GetParent();
		if (shapeEntitySrc)
		{
			ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(shapeEntitySrc));
			if (shapeEntity)
				OnShapeInit(shapeEntitySrc, shapeEntity);
		}

		EntityFlags flags;
		src.Get("Flags", flags);
		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		worldEditorAPI.SetVariableValue(src, null, "Flags", (flags | EntityFlags.EDITOR_ONLY).ToString());

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Set shape's line colour to GetColor() value
	protected void ColorShape()
	{
		if (!m_Source)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		IEntitySource parentSource = m_Source.GetParent();
		if (!parentSource)
			return;

		array<ref ContainerIdPathEntry> containerPath = {};

		Color color = GetColor();
		string colorString = string.Format("%1 %2 %3 %4", color.R(), color.G(), color.B(), color.A());

		bool manageEditAction = !worldEditorAPI.IsDoingEditAction();
		if (manageEditAction)
			worldEditorAPI.BeginEntityAction();

		worldEditorAPI.SetVariableValue(parentSource, containerPath, "LineColor", colorString);

		if (manageEditAction)
			worldEditorAPI.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Get the parent shape above this generator (has to be the direct parent)
	protected IEntitySource GetShapeSource()
	{
		if (!m_Source)
		{
			Print("[SCR_GeneratorBaseEntity.GetShapeSource] no generator source available (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.ERROR);
			return null;
		}

		IEntitySource parentSource = m_Source.GetParent();
		if (!parentSource)
			return null;

		if (!parentSource.GetClassName().ToType() || !parentSource.GetClassName().ToType().IsInherited(ShapeEntity))
			return null;

		return parentSource;

//		while (parentSource)
//		{
//			if (parentSource.GetClassName().ToType() && parentSource.GetClassName().ToType().IsInherited(ShapeEntity))
//				return parentSource;
//
//			parentSource = parentSource.GetParent();
//		}
//
//		return null;
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_GeneratorBaseEntity(IEntitySource src, IEntity parent)
	{

#ifdef WORKBENCH

		if (!_WB_GetEditorAPI()) // thumbnail generation
			return;

		m_Source = src;
		m_iSourceLayerID = m_Source.GetLayerID();

		m_RandomGenerator = new RandomGenerator();

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (worldEditor && !ShapeEntity.Cast(parent) && !worldEditor.IsPrefabEditMode())
		{
			Print("A Generator is not a direct child of a shape at " + GetOrigin(), LogLevel.WARNING);
			return;
		}

		m_ParentShapeSource = m_Source.GetParent();

#endif // WORKBENCH

	}
}
