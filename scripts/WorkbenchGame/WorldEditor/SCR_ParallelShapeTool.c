#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Parallel Shape Tool",
	description: "Create a selected polyline's/spline's parallel shape",
	awesomeFontCode: 0xF7A5)]
class SCR_ParallelShapeTool : WorldEditorTool
{
	/*
		Category: Margins
	*/

	[Attribute(defvalue: "1", desc: "[m] Offset safety for neater inner curves (Spline only)", params: "0 10 0.01", precision: 2, uiwidget: UIWidgets.Slider, category: "Margins")]
	protected float m_fOffsetSafetyMargin;

	[Attribute(defvalue: "1", desc: "Minimum distance between points, as to not saturate inner curves (Spline only)", params: "0 20 0.1", precision: 1, uiwidget: UIWidgets.Slider, category: "Margins")]
	protected float m_fMinDistanceBetweenPoints;

	/*
		Category: Debug
	*/

	[Attribute(defvalue: "0", desc: "Show more logs", category: "Debug")]
	protected bool m_bShowDebugLog;

	/*
		Category: Shape Creation
	*/

	[Attribute(defvalue: "1", desc: "Snap the newly created shapes' points to the ground", category: "Shape Creation")]
	protected bool m_bSnapToGround;

	[Attribute(defvalue: "0", desc: "Use spline if shape is a spline, may help in sharp turns; generally not needed, polyline should be enough", category: "Shape Creation")]
	protected bool m_bUseSplineIfPossible;

	[Attribute(defvalue: "1", desc: "Create shapes as the selected shape(s)' children; otherwise, create them on the same level", category: "Shape Creation")]
	protected bool m_bCreateShapeAsChild;

	[Attribute(defvalue: "1", desc: "Create shapes on the provided offsets and their negative, bordering the selected shape(s)", category: "Shape Creation")]
	protected bool m_bSymmetricalShapes;

	[Attribute(defvalue: "1", desc: "Cleans up the duplicated offsets to only keep unique ones - also applies to Symmetrical Shapes above", category: "Shape Creation")]
	protected bool m_bRemoveDuplicates;

	[Attribute(defvalue: "0", desc: "New shapes' offset(s) - negative means \"to the left\" of the shape, positive means \"to the right\" (from its first to its last point)", params: "-50 50 0.1", precision: 1, uiwidget: UIWidgets.Slider, category: "Shape Creation")]
	protected ref array<float> m_aOffsets;

	protected static const ref array<float> DEFAULT_OFFSETS = { -10.0, 10.0 };

	protected static const typename POLYLINE_TYPENAME = PolylineShapeEntity;
	protected static const typename SPLINE_TYPENAME = SplineShapeEntity;
	protected static const string POLYLINE_CLASSNAME = POLYLINE_TYPENAME.ToString();
	protected static const string SPLINE_CLASSNAME = SPLINE_TYPENAME.ToString();

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create")]
	protected void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("No WorldEditorAPI", LogLevel.ERROR);
			return;
		}

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
		{
			Print("No World loaded", LogLevel.WARNING);
			return;
		}

		int selectedEntities = worldEditorAPI.GetSelectedEntitiesCount();
		if (selectedEntities < 1)
		{
			Print("No entities selected", LogLevel.WARNING);
			return;
		}

		bool updatePropertyPanel;
		if (m_aOffsets.IsEmpty())
		{
			m_aOffsets.InsertAll(DEFAULT_OFFSETS);
			updatePropertyPanel = true;
		}

		if (m_bRemoveDuplicates)
		{
			int offsetCounts = m_aOffsets.Count();
			SCR_ArrayHelperT<float>.RemoveDuplicates(m_aOffsets); // removes from the end
			if (offsetCounts != m_aOffsets.Count())
				updatePropertyPanel = true;
		}

		if (updatePropertyPanel)
			UpdatePropertyPanel();

		array<float> offsets = {};
		offsets.Copy(m_aOffsets);

		if (m_bSymmetricalShapes)
		{
			offsets.Reserve(offsets.Count() * 2);
			foreach (float offset : m_aOffsets)
			{
				if (offset != 0)
					offsets.Insert(-offset);
			}

			if (m_bRemoveDuplicates)
				SCR_ArrayHelperT<float>.RemoveDuplicates(offsets);
		}

		IEntity entity;
		IEntitySource source;
		ShapeEntity shapeEntity;
		IEntity parent;
		IEntitySource parentSource;
		// creation
		array<vector> newPoints;
		ShapeEntity createdShape;
		IEntitySource createdShapeSource;
		int currentLayerId = worldEditorAPI.GetCurrentEntityLayerId();
		map<IEntitySource, ref array<vector>> shapeSourcePoints = new map<IEntitySource, ref array<vector>>();
		for (int selectedEntityIndex; selectedEntityIndex < selectedEntities; ++selectedEntityIndex)
		{
			source = worldEditorAPI.GetSelectedEntity(selectedEntityIndex);
			if (!source) // ...how
			{
				Print("No entity selected, skipping index " + selectedEntityIndex, LogLevel.WARNING);
				continue;
			}

			entity = worldEditorAPI.SourceToEntity(source);
			if (!entity)
			{
				Print("Null entity; skipping index " + selectedEntityIndex, LogLevel.WARNING);
				continue;
			}

			shapeEntity = ShapeEntity.Cast(entity);
			if (!shapeEntity)
			{
				if (m_bShowDebugLog)
					Print("The selected entity is not a shape (polyline/spline) at index " + selectedEntityIndex, LogLevel.NORMAL);

				continue;
			}

			if (shapeEntity.GetPointCount() < 2)
			{
				Print("The selected shape #" + selectedEntityIndex + " requires at least two points - " + shapeEntity.GetOrigin(), LogLevel.NORMAL);
				continue;
			}

			CreateParallelOffsetShapes(shapeEntity, offsets);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Single offset version of CreateParallelOffsetShapes
	//! \see CreateParallelOffsetShapes
	static IEntitySource CreateParallelOffsetShape(notnull ShapeEntity shapeEntity, float offset, SCR_ParallelShapeToolSettings settings = null)
	{
		array<IEntitySource> shapes = CreateParallelOffsetShapes(shapeEntity, { offset });
		if (!shapes || shapes.IsEmpty())
			return null;

		return shapes[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Create parallel shapes from the provided shape.
	//! The created shapes are created in the active layer if they are not children
	//! \param[in] shapeEntity the model to use
	//! \param[in] offsets array of wanted offset - batch process is more efficient
	//! \param[in] settings
	//! \return the created shape sources, null on error (e.g provided shape only has one point, etc)
	static array<IEntitySource> CreateParallelOffsetShapes(notnull ShapeEntity shapeEntity, notnull array<float> offsets, SCR_ParallelShapeToolSettings settings = null)
	{
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (!worldEditorAPI)
		{
			Print("[SCR_ParallelShapeTool.CreateParallelOffsetShapes] WorldEditorAPI is not available (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
			return null;
		}

		if (shapeEntity.GetPointCount() < 2)
		{
			Print("[SCR_ParallelShapeTool.CreateParallelOffsetShapes] The provided shape requires at least two points (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
			return null;
		}

		array<IEntitySource> result = {};
		int layerId = worldEditorAPI.GetCurrentEntityLayerId();

		array<vector> newPoints;
		IEntitySource createdShapeSource;
		IEntity createdShape;

		IEntitySource parentSource;
		vector origin;
		vector angles;

		if (settings && settings.m_bCreateAsChildren)
		{
			parentSource = worldEditorAPI.EntityToSource(shapeEntity);
		}
		else // create in world
		{
			origin = shapeEntity.GetOrigin();
			angles = shapeEntity.GetAngles();
		}

		bool manageEntityAction = worldEditorAPI.BeginEntityAction();

		foreach (int offsetIndex, float offset : offsets)
		{
			float absOffset = offset;
			if (absOffset < 0)
				absOffset = -absOffset;

			newPoints = SCR_ParallelShapeHelper.GetRelativeOffsetPointsFromShape(shapeEntity, offset, shapeEntity.IsClosed());

			if (settings)
			{
				if (settings.m_fOffsetSafetyMargin > 0)
					SCR_ParallelShapeHelper.RemovePointsCloseToShape(shapeEntity, absOffset - settings.m_fOffsetSafetyMargin, newPoints);

				if (settings.m_bSnapToGround)
					SCR_ParallelShapeHelper.SnapRelativePointsToGround(shapeEntity, newPoints);

				// remove excess points
				if (settings.m_fMinDistanceBetweenPoints > 0 && SplineShapeEntity.Cast(createdShape) != null) // spline only
					SCR_ParallelShapeHelper.ReducePointsDensity(settings.m_fMinDistanceBetweenPoints, newPoints);
			}

			int newPointsCount = newPoints.Count();
			if (newPointsCount < 2)
			{
				float minDistanceBetweenPoints;
				if (settings)
					minDistanceBetweenPoints = settings.m_fMinDistanceBetweenPoints;

				Print(
					"The number of created points (" + newPointsCount + ") is not enough for a proper shape - "
					+ "try changing the offset safety margin (current: " + offset.ToString(-1, 2) + "m) "
					+ "or the min distance between points (current: " + minDistanceBetweenPoints.ToString(-1, 2) + "m)", LogLevel.WARNING);
				continue;
			}

			if (settings
				&& settings.m_bUseSplineForSpline
				&& shapeEntity.ClassName().ToType()
				&& shapeEntity.ClassName().ToType().IsInherited(SPLINE_TYPENAME))
				createdShapeSource = worldEditorAPI.CreateEntity(SPLINE_CLASSNAME, string.Empty, layerId, parentSource, origin, angles);
			else
				createdShapeSource = worldEditorAPI.CreateEntity(POLYLINE_CLASSNAME, string.Empty, layerId, parentSource, origin, angles);

			if (!createdShapeSource)
			{
				Print("Cannot create shape of offset #" + offsetIndex + "'s shape", LogLevel.ERROR);
				continue;
			}

			createdShape = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(createdShapeSource));
			if (!createdShape) // this check is technically useless, but let's cleanup after us just in case
			{
				Print("The created entity is not a shape", LogLevel.ERROR);
				worldEditorAPI.DeleteEntity(createdShapeSource);
				continue;
			}

			foreach (int i, vector newPoint : newPoints)
			{
				if (!worldEditorAPI.CreateObjectArrayVariableMember(createdShapeSource, null, "Points", "ShapePoint", i))
				{
					Print("[SCR_ParallelShapeHelper.CreateParallelOffsetShapes] Cannot create point " + i + " (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.ERROR);
					break;
				}

				newPoints[i] = shapeEntity.CoordToLocal(newPoint);
				worldEditorAPI.SetVariableValue(createdShapeSource, { new ContainerIdPathEntry("Points", i) }, "Position", string.Format("%1 %2 %3", newPoint[0], newPoint[1], newPoint[2]));
			}

			result.Insert(createdShapeSource);
		}

		if (manageEntityAction)
			worldEditorAPI.EndEntityAction();

		return result;
	}
}

class SCR_ParallelShapeToolSettings
{
	bool m_bCreateAsChildren;
	bool m_bSnapToGround;
	bool m_bUseSplineForSpline;

	float m_fMinDistanceBetweenPoints;
	float m_fOffsetSafetyMargin;
}
#endif
