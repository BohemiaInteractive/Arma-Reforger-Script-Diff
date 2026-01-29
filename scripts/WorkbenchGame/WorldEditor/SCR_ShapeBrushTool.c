#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Shape Brush Tool",
	description: "Draw an area or a line using a brush."
		+ "\n- click and drag to draw"
		+ "\n- a width of zero turns line mode on"
		+ "\n- Ctrl+ScrollWheel to modify the brush's radius on the fly"
		+ "\n- it is not possible to switch between line and brush mode while drawing",
	awesomeFontCode: 0xF5CB // or 0xF55D?
)]
class SCR_ShapeBrushTool : WorldEditorTool
{
	/*
		Category: Brush
	*/

	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Brush radius\nCtrl+ScrollWheel to change this value", params: string.Format("%1 %2 %3", RADIUS_MIN, RADIUS_MAX, RADIUS_STEP), category: "Brush")]
	protected float m_fRadius;

	[Attribute(defvalue: "25", uiwidget: UIWidgets.Slider, desc: "Shape point's min step", params: "0 500 10", category: "Brush")]
	protected float m_fMinStep;

	/*
		Category: Shape
	*/

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Type of shape to be drawn", enums: SCR_ParamEnumArray.FromString("Polyline,Multiple straight line segments;Spline,Bézier curve segments"), category: "Shape")]
	protected int m_iShapeType;

	[Attribute(defvalue: "1", desc: "Close the shape or not", category: "Shape")]
	protected bool m_bIsShapeClosed;

	protected bool m_bIsDrawing;

	protected vector m_vLastCameraPosition;
	protected vector m_vLastMousePosition;
	protected ref Shape m_BrushShape;
	protected int m_iBrushShapeColor;

	protected ref array<vector> m_aShapePoints = {};
	protected ref array<float> m_aShapeRadii = {};
	protected ref SCR_DebugShapeManager m_DebugShapeManager = new SCR_DebugShapeManager();

	protected static const float RADIUS_STEP = 10;
	protected static const float RADIUS_MAX = 500;
	protected static const float RADIUS_MIN_WHILE_DRAWING = 10;
	protected static const float RADIUS_MIN = 0;

	protected static const int BRUSH_COLOUR_DEFAULT	= 0xFF00FF00; // green
	protected static const int BRUSH_COLOUR_ACTIVE	= 0xFFFF8800; // orange

	protected static const float CURSOR_RATIO = 0.25;

	protected static const int ARC_STEP = 30;
	protected static const int ARC_SAFETY = 0; // .125;

	//------------------------------------------------------------------------------------------------
	protected void DrawBrush()
	{
		if (m_bIsDrawing)
			m_iBrushShapeColor = BRUSH_COLOUR_ACTIVE;
		else
			m_iBrushShapeColor = BRUSH_COLOUR_DEFAULT;

		if (m_fRadius > 0)
		{
			m_BrushShape = CreateCircle(m_vLastMousePosition, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);
			return;
		}

		float size = vector.Distance(m_vLastCameraPosition, m_vLastMousePosition) * CURSOR_RATIO;
		m_BrushShape = Shape.CreateArrow(m_vLastMousePosition + vector.Up * size, m_vLastMousePosition, size * 0.25, m_iBrushShapeColor, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected array<vector> GetShapeBorder()
	{
		int shapePointsCount = m_aShapePoints.Count();
		if (shapePointsCount < 1)
			return {};

		array<vector> result = {};
		if (shapePointsCount == 1)
		{
			for (int i; i < 360; i += ARC_STEP)
			{
				result.Insert(GetPointAtAngle(m_aShapePoints[0], i, m_aShapeRadii[0], true));
			}

			return result;
		}

		if (m_fRadius == 0) // line only
			return SCR_ArrayHelperT<vector>.GetCopy(m_aShapePoints);

		array<vector> rightPoints = {};
		array<vector> rejectedPoints = {};
		result.Reserve(shapePointsCount);
		vector point;
		vector prevShapePoint;
		foreach (int shapePointIndex, vector currShapePoint : m_aShapePoints)
		{
			if (shapePointIndex == 0)							// first drawn point
			{
				float angleDeg = vector.Direction(currShapePoint, m_aShapePoints[1]).ToYaw();
				for (int additionalAngleDeg = 90; additionalAngleDeg <= 270; additionalAngleDeg += ARC_STEP)
				{
					point = GetPointAtAngle(currShapePoint, angleDeg + additionalAngleDeg, m_aShapeRadii[shapePointIndex] + ARC_SAFETY, false);
					if (IsValidPoint(point, shapePointIndex))
						result.Insert(point);
					else
						rejectedPoints.Insert(point);
				}
			}
			else if (shapePointIndex == shapePointsCount - 1)	// last drawn point
			{
				float angleDeg = vector.Direction(m_aShapePoints[shapePointIndex - 1], currShapePoint).ToYaw();

				for (int additionalAngleDeg = -90; additionalAngleDeg <= 90; additionalAngleDeg += ARC_STEP)
				{
					point = GetPointAtAngle(currShapePoint, angleDeg + additionalAngleDeg, m_aShapeRadii[shapePointIndex] + ARC_SAFETY, false);
					if (IsValidPoint(point, shapePointIndex))
						result.Insert(point);
					else
						rejectedPoints.Insert(point);
				}
			}
			else // middle point
			{
				float angleDeg = vector.Direction(prevShapePoint, m_aShapePoints[shapePointIndex + 1]).ToYaw();

				point = GetPointAtAngle(currShapePoint, angleDeg - 90, m_aShapeRadii[shapePointIndex], false); // left
				if (IsValidPoint(point, shapePointIndex))
					result.Insert(point);
				else
					rejectedPoints.Insert(point);

				point = GetPointAtAngle(currShapePoint, angleDeg + 90, m_aShapeRadii[shapePointIndex], false); // right
				if (IsValidPoint(point, shapePointIndex))
					rightPoints.Insert(point);
				else
					rejectedPoints.Insert(point);
			}

			prevShapePoint = currShapePoint;
		}

		SCR_ArrayHelperT<vector>.Reverse(rightPoints);
		result.InsertAll(rightPoints);

		// set terrain Y here (and not for all including rejected points)
		foreach (int i, vector resultPoint : result)
		{
			resultPoint[1] = m_API.GetTerrainSurfaceY(resultPoint[0], resultPoint[2]);
			result[i] = resultPoint;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] origin the centre
	//! \param[in] angleDeg absolute angle in degrees
	//! \param[in] distance distance in metres
	//! \param[in] setTerrainY whether or not snap it to terrain
	//! \return the wanted point
	protected vector GetPointAtAngle(vector origin, float angleDeg, float distance, bool setTerrainY)
	{
		if (distance == 0)
			return origin;

		float angleRad = -(angleDeg - 90) * Math.DEG2RAD;
		if (setTerrainY)
		{
			vector point = origin + { Math.Cos(angleRad) * distance, 0, Math.Sin(angleRad) * distance };
			point[1] = m_API.GetTerrainSurfaceY(point[0], point[2]);
			return point;
		}
		else
		{
			return origin + { Math.Cos(angleRad) * distance, 0, Math.Sin(angleRad) * distance };
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] point
	//! \param[in] shapePointIndex
	//! \return
	bool IsValidPoint(vector point, int shapePointIndex)
	{
		foreach (int otherPointIndex, vector otherShapePoint : m_aShapePoints)
		{
			if (shapePointIndex == otherPointIndex)
				continue;

			if (vector.DistanceXZ(point, otherShapePoint) < m_aShapeRadii[otherPointIndex])
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawShape()
	{
		m_DebugShapeManager.Clear();

		// draw axis
		foreach (int i, vector shapePoint : m_aShapePoints)
		{
			m_DebugShapeManager.AddLine(shapePoint + 3 * vector.Up, shapePoint);
			if (i > 0)
				m_DebugShapeManager.AddLine(m_aShapePoints[i - 1], shapePoint, 0x88FFDD55);
		}

		// draw border
		array<vector> borderPoints = GetShapeBorder();
		foreach (int i, vector borderPoint : borderPoints)
		{
			m_DebugShapeManager.AddLine(borderPoint + 3 * vector.Up, borderPoint, 0xFF55FF55);
			if (i > 0)
				m_DebugShapeManager.AddLine(borderPoints[i - 1], borderPoint, 0xFF00FF00);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource CreateShapeEntity(notnull array<vector> shapePoints)
	{
		if (shapePoints.IsEmpty())
		{
			PrintFormat("Empty points array provided to create shape", level: LogLevel.ERROR);
			return null;
		}

		m_API.BeginEntityAction();

		string shapeClassname;
		if (m_iShapeType == 0)
			shapeClassname = "PolylineShapeEntity";
		else
			shapeClassname = "SplineShapeEntity";

		IEntitySource shapeEntitySource = m_API.CreateEntity(shapeClassname, string.Empty, m_API.GetCurrentEntityLayerId(), null, shapePoints[0], vector.Zero);
		if (!shapeEntitySource)
		{
			m_API.EndEntityAction();

			PrintFormat("Cannot create %1 shape", shapeClassname, level: LogLevel.ERROR);
			Workbench.Dialog("Error", "Cannot create " + shapeClassname + " shape");

			return null;
		}

		vector origin;
		foreach (int i, vector point : shapePoints)
		{
			if (!m_API.CreateObjectArrayVariableMember(shapeEntitySource, null, "Points", "ShapePoint", i))
			{
				Print("Cannot create point #" + i, LogLevel.ERROR);
				break;
			}

			if (i == 0) // first point = "0 0 0"
			{
				origin = point;
				continue;
			}

			point -= origin;
			m_API.SetVariableValue(shapeEntitySource, { new ContainerIdPathEntry("Points", i) }, "Position", string.Format("%1 %2 %3", point[0], point[1], point[2]));
		}

		if (m_bIsShapeClosed)
			m_API.SetVariableValue(shapeEntitySource, null, "IsClosed", "1");

		m_API.EndEntityAction();

		return shapeEntitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! Helps getting proper new value for a property
	//! \param[in] delta the scrollwheel value (obtained in OnWheelEvent) that is a multiple of 120
	//! \param[in] currentValue the value from which to start
	//! \param[in] min the min value
	//! \param[in] max the max value
	//! \param[in] step the step by which delta's converted value will be multiplied
	//! \return the min-max clamped new value
	protected float AdjustValueUsingScrollwheel(float delta, float currentValue, float min, float max, float step)
	{
		// delta returns multiples of 120 - converting it into a more useable value of multiples of 1
		float value = currentValue + (delta / 120) * step;

		if (value < min)
			return min;

		if (value > max)
			return max;

		return value;
	}

	//------------------------------------------------------------------------------------------------
	override void OnLeaveEvent()
	{
		if (!m_bIsDrawing)
			m_BrushShape = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMouseMoveEvent(float x, float y)
	{
		vector traceStart, traceEnd, traceDir;
		if (!m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
			return;

		if (m_bIsDrawing)
		{
			if (vector.DistanceSqXZ(traceEnd, m_aShapePoints[m_aShapePoints.Count() - 1]) >= m_fMinStep * m_fMinStep)
			{
				if (m_bIsDrawing)
				{
					m_aShapePoints.Insert(m_vLastMousePosition);
					m_aShapeRadii.Insert(m_fRadius);

					DrawShape();
				}
			}
		}

		m_vLastCameraPosition = traceStart;
		m_vLastMousePosition = traceEnd;

		DrawBrush();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		m_bIsDrawing = true;

		vector traceStart, traceEnd, traceDir;
		m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir);

		m_vLastCameraPosition = traceStart;
		m_vLastMousePosition = traceEnd;

		m_aShapePoints.Clear();
		m_aShapeRadii.Clear();

		m_aShapePoints.Insert(m_vLastMousePosition);
		m_aShapeRadii.Insert(m_fRadius);

		DrawShape();

		DrawBrush();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons)
	{
		if (buttons != WETMouseButtonFlag.LEFT)
			return;

		vector traceStart, traceEnd, traceDir;
		if (!m_API.TraceWorldPos(x, y, TraceFlags.WORLD, traceStart, traceEnd, traceDir))
			return;

		m_bIsDrawing = false;
		if (m_aShapePoints.Count() > 1 || traceEnd != m_aShapePoints[0])
		{
			m_aShapePoints.Insert(m_vLastMousePosition);
			m_aShapeRadii.Insert(m_fRadius);
		}

		array<vector> borderPoints = GetShapeBorder();

		IEntitySource shapeEntitySource = CreateShapeEntity(borderPoints);

		m_aShapePoints.Clear();
		m_aShapeRadii.Clear();
		m_DebugShapeManager.Clear();

		if (shapeEntitySource)
			m_API.SetEntitySelection(shapeEntitySource);
		else
			Print("Cannot create Shape", LogLevel.ERROR);

		DrawBrush();
	}

	//------------------------------------------------------------------------------------------------
	override void OnWheelEvent(int delta)
	{
		// adjusts m_fRadius value using a CTRL + Scrollwheel keybind
		if (GetModifierKeyState(ModifierKey.CONTROL))
		{
			if (m_bIsDrawing)
			{
				if (m_fRadius == 0)
					return;

				m_fRadius = AdjustValueUsingScrollwheel(delta, m_fRadius, RADIUS_MIN_WHILE_DRAWING, RADIUS_MAX, RADIUS_STEP);	// cannot reach 0 if drawing
			}
			else
			{
				m_fRadius = AdjustValueUsingScrollwheel(delta, m_fRadius, RADIUS_MIN, RADIUS_MAX, RADIUS_STEP);					// allow reaching 0
			}

			m_BrushShape = CreateCircle(m_vLastMousePosition, vector.Up, m_fRadius, m_iBrushShapeColor, 50, ShapeFlags.NOZBUFFER);
			UpdatePropertyPanel();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_aShapePoints.Clear();
		m_aShapeRadii.Clear();
		m_bIsDrawing = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDeActivate()
	{
		m_BrushShape = null;
	}
}
#endif
