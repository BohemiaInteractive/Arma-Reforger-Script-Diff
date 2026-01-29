#ifdef WORKBENCH
[WorkbenchToolAttribute(
	"Shape Area Tool",
	"Turns polylines and splines into specific shapes"
	+ "\nor creates a shape in the middle of the screen"
	+ "\n"
	+ "\nTo make a \"diamond\" (rhombus),"
	+ "\nuse Circle and set the number of points to 4"
	+ "\n"
	+ "\nPoints are created clockwise.",
	awesomeFontCode: 0xF5EE)]
class SCR_ShapeAreaTool : WorldEditorTool
{
	/*
		Category: Global
	*/

	//! - 0 = circle
	//! - 1 = rectangle
	//! - 2 = star
	[Attribute(
		defvalue: "0",
		desc: "Shape type to be created",
		uiwidget: UIWidgets.ComboBox,
		enums: SCR_ParamEnumArray.FromString("Circle,Can be used to create polygons;Rectangle;Star"),
		category: "Global")]
	protected int m_iShapeType;

	[Attribute(defvalue: "48", desc: "Wanted shape's width", params: "0.001 inf", category: "Global")]
	protected float m_fWidth;

	[Attribute(defvalue: "32", desc: "Wanted shape's length", params: "0.001 inf", category: "Global")]
	protected float m_fLength;

	/*
		Category: Circle
	*/

	[Attribute(defvalue: "12", desc: "a minimum of 12 points is recommended for a round circle", params: "3 inf", category: "Circle")]
	protected int m_iCircleSegmentsCount;

	/*
		Category: Rectangle
	*/

	[Attribute(defvalue: "1", desc: "Nomber of segments per side", params: "1 inf", category: "Rectangle")]
	protected int m_iRectangleSegmentsPerSide;

	/*
		Category: Star
	*/

	[Attribute(defvalue: "5", desc: "Number of branches in the star", params: "2 inf", category: "Star")]
	protected int m_iStarBranchesCount;

	[Attribute(defvalue: "0.5", desc: "Star inner radius ratio - e.g 0.75 * width-length radius", uiwidget: UIWidgets.Slider, params: "0.01 1", category: "Star")]
	protected float m_fStarInnerRadiusRatio;

	/*
		Category: Points
	*/

	[Attribute(defvalue: "1", desc: "Snap shape points to terrain", category: "Points")]
	protected bool m_bSnapToTerrain;

	[Attribute(defvalue: "1", desc: "Whether or not points should be around the shape entity's origin or if the points should be in positive position only (shape origin being bottom-left)", category: "Points")]
	protected bool m_bCentreOnPosition;

	[Attribute(defvalue: "0", desc: "Define whether to close, open or leave as is the selected shape(s)", uiwidget: UIWidgets.ComboBox, enums: SCR_ParamEnumArray.FromString("Leave as is;Open;Close"), category: "Points")]
	protected int m_iShapeClosing;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create Polyline")]
	protected void CreatePolyline()
	{
		CreateShape(false);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Create Spline")]
	protected void CreateSpline()
	{
		CreateShape(true);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Convert Sel. Shapes")]
	protected void ConvertSelectedShapes()
	{
		array<IEntitySource> shapeEntitySources = GetSelectedShapeEntitySources();
		if (shapeEntitySources.IsEmpty())
		{
			Print("[SCR_ShapeAreaTool] No shapes were selected, no conversion needed", LogLevel.NORMAL);
			return;
		}

		m_API.BeginEntityAction();

		ConvertShapes(shapeEntitySources);

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] isSpline true if spline, false if polyline
	protected void CreateShape(bool isSpline)
	{
		vector worldStart, worldPos, worldNormal;
		if (!m_API.TraceWorldPos(m_API.GetScreenWidth() * 0.5, m_API.GetScreenHeight() * 0.5, TraceFlags.WORLD, worldStart, worldPos, worldNormal))
			return;

		m_API.BeginEntityAction();

		IEntitySource shapeEntitySource;
		if (isSpline)
			shapeEntitySource = m_API.CreateEntity(((typename)SplineShapeEntity).ToString(), string.Empty, m_API.GetCurrentEntityLayerId(), null, worldPos, vector.Zero);
		else
			shapeEntitySource = m_API.CreateEntity(((typename)PolylineShapeEntity).ToString(), string.Empty, m_API.GetCurrentEntityLayerId(), null, worldPos, vector.Zero);

		m_API.AddToEntitySelection(shapeEntitySource);

		ConvertShapes({ shapeEntitySource });

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntitySource> GetSelectedShapeEntitySources()
	{
		array<IEntitySource> result = SCR_WorldEditorToolHelper.GetSelectedWorldEntitySources();

		// remove all non-shapes
		for (int i = result.Count() - 1; i >= 0; --i)
		{
			if (!m_API.SourceToEntity(result[i]).IsInherited(ShapeEntity))
				result.Remove(i);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Convert the provided shapes to the wanted shape
	//! \param[in] shapeEntitySources
	protected void ConvertShapes(notnull array<IEntitySource> shapeEntitySources)
	{
		if (shapeEntitySources.IsEmpty())
			return;

		array<vector> points = GetShapePoints();

		foreach (IEntitySource shapeEntitySource : shapeEntitySources)
		{
			bool isSelected = m_API.IsEntitySelected(shapeEntitySource);
			if (isSelected)
				m_API.RemoveFromEntitySelection(shapeEntitySource); // prevents a Vector Tool point selection issue

			SetShapePoints(shapeEntitySource, points);

			if (m_iShapeClosing == 1)	// open - default shape state
				m_API.ClearVariableValue(shapeEntitySource, null, "IsClosed");
			else
			if (m_iShapeClosing == 2)	// close
				m_API.SetVariableValue(shapeEntitySource, null, "IsClosed", "1");
//			else // if (m_iShapeClosing == 0) // as is
//				leave as is

			if (isSelected)
				m_API.AddToEntitySelection(shapeEntitySource);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetShapePoints()
	{
		array<vector> result;
		if (m_iShapeType == 0)
			result = GetCirclePoints();
		else
		if (m_iShapeType == 2)
			result = GetStarPoints();
		else // default
			result = GetRectanglePoints();

		if (result.IsEmpty())
		{
			Print("Cannot modify shape(s), points calculation went wrong", LogLevel.ERROR);
			return null;
		}

		if (!m_bCentreOnPosition)
		{
			vector offset = { m_fWidth * 0.5, 0, m_fLength * 0.5 };
			foreach (int i, vector point : result)
			{
				result[i] = point + offset;
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetCirclePoints()
	{
		array<vector> result = {};
		float angleSize = Math.PI2 / m_iCircleSegmentsCount;

		for (float i = Math.PI_HALF + Math.PI2, min = Math.PI_HALF; i > min; i -= angleSize) // clockwise
		{
			if (float.AlmostEqual(i, min)) // avoids a sometimes "closed" shape
				break;

			result.Insert({ Math.Cos(i) * m_fWidth * 0.5, 0, Math.Sin(i) * m_fLength * 0.5 });
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetRectanglePoints()
	{
		if (m_iRectangleSegmentsPerSide)
			return {
				{ m_fWidth * 0.5, 0, m_fLength * 0.5 },		// top-right
				{ m_fWidth * 0.5, 0, -m_fLength * 0.5 },	// bottom-right
				{ -m_fWidth * 0.5, 0, -m_fLength * 0.5 },	// bottom-left
				{ -m_fWidth * 0.5, 0, m_fLength * 0.5 },	// top-left
			};

		// multiple segments per side, go

		array<vector> data = {
			{ m_fWidth * 0.5, 0, m_fLength * 0.5 },					// top-right
			{ 0, 0, -m_fLength / m_iRectangleSegmentsPerSide },		// offset

			{ m_fWidth * 0.5, 0, -m_fLength * 0.5 },				// bottom-right
			{ -m_fWidth / m_iRectangleSegmentsPerSide, 0, 0 },		// offset

			{ -m_fWidth * 0.5, 0, -m_fLength * 0.5 },				// bottom-left
			{ 0, 0, m_fLength / m_iRectangleSegmentsPerSide },		// offset

			{ -m_fWidth * 0.5, 0, m_fLength * 0.5 },				// top-left
			{ m_fWidth  / m_iRectangleSegmentsPerSide, 0, 0 },		// offset
		};

		array<vector> result = {};
		for (int i, count = data.Count(); i < count; i += 2) // step 2
		{
			vector startPoint = data[i];
			vector offset = data[i + 1];

			for (int j = 0; j < m_iRectangleSegmentsPerSide; ++j)
			{
				result.Insert(startPoint + (float)j * offset);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetStarPoints() // it is an oval of 4 points after all
	{
		array<vector> result = {};
		float angleSize = Math.PI2 / m_iStarBranchesCount;

		for (float i = Math.PI_HALF + Math.PI2, min = Math.PI_HALF; i > min; i -= angleSize) // clockwise
		{
			if (float.AlmostEqual(i, min)) // avoids a sometimes closed shape
				break;

			// point and small radius point
			result.Insert({ Math.Cos(i) * m_fWidth * 0.5, 0, Math.Sin(i) * m_fLength * 0.5 });
			result.Insert({ Math.Cos(i - 0.5 * angleSize) * m_fWidth * 0.5 * m_fStarInnerRadiusRatio, 0, Math.Sin(i - 0.5 * angleSize) * m_fLength * 0.5 * m_fStarInnerRadiusRatio });
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entitySource
	//! \param[in] points
	protected void SetShapePoints(notnull IEntitySource entitySource, notnull array<vector> points)
	{
		int originalPointsCount = entitySource.GetObjectArray("Points").Count();
		bool warned = false;

		array<vector> pointsCopy = {};
		pointsCopy.Copy(points);
		if (m_bSnapToTerrain)
		{
			IEntity entity = m_API.SourceToEntity(entitySource);
			if (entity)
			{
				foreach (int i, vector point : pointsCopy)
				{
					vector terrainPos = entity.CoordToParent(point);
					float y;
					if (!m_API.TryGetTerrainSurfaceY(terrainPos[0], terrainPos[2], y))
						continue;

					terrainPos[1] = y;
					pointsCopy[i] = entity.CoordToLocal(terrainPos);
				}
			}
			else
			{
				Print("Cannot snap shape(s) to terrain, entity not found from source", LogLevel.WARNING);
			}
		}

		foreach (int i, vector point : pointsCopy)
		{
			m_API.CreateObjectArrayVariableMember(entitySource, null, "Points", "ShapePoint", i + originalPointsCount);
			m_API.SetVariableValue(entitySource, { new ContainerIdPathEntry("Points", i + originalPointsCount) }, "Position", string.Format("%1 %2 %3", point[0], point[1], point[2]));
		}

		for (int i = originalPointsCount - 1; i >= 0; --i)
		{
			m_API.RemoveObjectArrayVariableMember(entitySource, null, "Points", i);
		}
	}
}
#endif // WORKBENCH
