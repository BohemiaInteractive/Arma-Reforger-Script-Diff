[EntityEditorProps(category: "GameScripted/Generators/Helpers", description: "", dynamicBox: true, visible: false)]
class SCR_ShapeAnalyserEntityClass : SCR_GeneratorBaseEntityClass
{
}

class SCR_ShapeAnalyserEntity : SCR_GeneratorBaseEntity
{
	[Attribute(defvalue: "1", category: "Shape", desc: "")]
	protected bool m_bDrawErrors;

	[Attribute(defvalue: "30", category: "Shape", desc: "Maximum 2D angle (in degree) for a 10m line", params: "0 90 1", uiwidget: UIWidgets.Slider)]
	protected float m_fMaxAngle;

	[Attribute(defvalue: "10", category: "Shape", desc: "Maximum slope angle (in degree)", params: "0 90 1", uiwidget: UIWidgets.Slider)]
	protected float m_fMaxSlope;

	[Attribute(defvalue: "0", category: "Debug", desc: "")]
	protected bool m_bDrawDebugShapes;

	[Attribute(defvalue: "0", category: "Debug", desc: "")]
	protected bool m_bPrintDebugInfo;

#ifdef WORKBENCH

	protected static ref array<ref Shape> s_aErrorShapes;
	protected static ref array<ref Shape> s_aDebugShapes;

	protected static const int ERROR_SHAPE_ANGLE_ARRAY_LENGTH = 50;
	protected static const float ERROR_SHAPE_ANGLE_CHECK_LENGTH = 10.0; //!< see m_fMaxAngle description

	protected static float SLOPE_ERROR_SHAPE_LENGTH = 30.0;
	protected static float ANGLE_ERROR_LINE_LENGTH = 5.0;

	protected static vector NORMAL_POINT_LINE_VECTOR = "0 20 0";
	protected static float NORMAL_POINT_LINE_HAT_SIZE = 1.0;
	protected static vector TESSELATED_POINT_LINE_VECTOR = "0 10 0";
	protected static vector MIDDLE_POINT_LINE_VECTOR = "0 15 0";
	protected static int ERROR_SHAPE_COLOUR = Color.RED;
	protected static int DEBUG_SHAPE_COLOUR = Color.MAGENTA;
	protected static int DEBUG_SHAPE_COLOUR_IMPORTANT = Color.DARK_RED;
	protected static ShapeFlags DEBUG_SHAPE_FLAGS = ShapeFlags.NOZBUFFER;
	protected static ShapeFlags DEBUG_SHAPE_FLAGS_SPHERE = ShapeFlags.NOOUTLINE | ShapeFlags.TRANSP;
	protected static int DEBUG_SHAPE_COLOUR_ALPHA = 0x66000000;


	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		if (!m_ParentShapeSource)
			return false;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		ShapeEntity parentShape = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!parentShape)
			return false;

		src = worldEditorAPI.EntityToSource(this);
		BaseContainerTools.WriteToInstance(this, src); // refresh attributes

		Process(m_ParentShapeSource, parentShape);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (worldEditorAPI)
			return;

		shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));

		Process(shapeEntitySrc, shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void Process(notnull IEntitySource shapeEntitySrc, notnull ShapeEntity shapeEntity)
	{
		SCR_ShapeAnalyser shapeAnalyser = SCR_ShapeAnalyser.CreateFromShape(shapeEntity);

		s_aDebugShapes = null;
		s_aErrorShapes = null;

		if (m_bDrawDebugShapes)
			DrawDebugShapes(shapeAnalyser);

		if (m_bDrawErrors)
			DrawErrorShapes(shapeAnalyser);

		if (m_bPrintDebugInfo)
			PrintDebugInfo(shapeAnalyser);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShapes(notnull SCR_ShapeAnalyser shapeAnalyser)
	{
		s_aDebugShapes = {};

		array<ref SCR_Ray> pointRays = shapeAnalyser.GetAbsoluteAnchorPointRays();
		array<ref SCR_Ray> middlePointRays = shapeAnalyser.GetAbsoluteMiddlePointRays();
		array<ref SCR_Ray> tesselatedPointRays = shapeAnalyser.GetAbsoluteTesselatedPointRays();

		int currentPointIndex = 0;
		int currentMiddlePointIndex = 0;
		int lastPointIndex = pointRays.Count() - 1;
		int lastMiddlePointIndex = pointRays.Count() - 1;
		SCR_Ray currentPoint = pointRays[0];
		SCR_Ray currentMiddlePoint = middlePointRays[0];

		foreach (SCR_Ray pointRay : tesselatedPointRays)
		{
			if (pointRay == currentPoint)
			{
				AddDebugLine(pointRay.m_vPosition, pointRay.m_vPosition + NORMAL_POINT_LINE_VECTOR, true);
				AddDebugLine(pointRay.m_vPosition - pointRay.m_vDirection * NORMAL_POINT_LINE_VECTOR[1], pointRay.m_vPosition + pointRay.m_vDirection * NORMAL_POINT_LINE_VECTOR[1], true);
				currentPointIndex++;
				if (currentPointIndex < lastPointIndex)
					currentPoint = pointRays[currentPointIndex];
			}
			else if (pointRay == currentMiddlePoint)
			{
				AddDebugLine(pointRay.m_vPosition, pointRay.m_vPosition + MIDDLE_POINT_LINE_VECTOR, true);
				AddDebugLine(pointRay.m_vPosition - pointRay.m_vDirection * MIDDLE_POINT_LINE_VECTOR[1], pointRay.m_vPosition + pointRay.m_vDirection * MIDDLE_POINT_LINE_VECTOR[1], true);
				currentMiddlePointIndex++;
				if (currentMiddlePointIndex < lastMiddlePointIndex)
					currentMiddlePoint = middlePointRays[currentMiddlePointIndex];
			}
			else // normal
			{
				AddDebugLine(pointRay.m_vPosition, pointRay.m_vPosition + TESSELATED_POINT_LINE_VECTOR);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddDebugLine(vector pointA, vector pointB, bool isImportant = false)
	{
		vector pointRays[2] = { pointA, pointB };
		if (isImportant)
			s_aDebugShapes.Insert(Shape.CreateLines(DEBUG_SHAPE_COLOUR_IMPORTANT, DEBUG_SHAPE_FLAGS, pointRays, 2));
		else
			s_aDebugShapes.Insert(Shape.CreateLines(DEBUG_SHAPE_COLOUR, DEBUG_SHAPE_FLAGS, pointRays, 2));
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawErrorShapes(notnull SCR_ShapeAnalyser shapeAnalyser)
	{
		array<ref SCR_Ray> pointRays = shapeAnalyser.GetAbsoluteAnchorPointRays();
		array<ref SCR_Ray> tesselatedPointRays = shapeAnalyser.GetAbsoluteTesselatedPointRays();

		// error display
		s_aErrorShapes = {};
		vector segment[2];

		float firstAngleDeg = Math.Atan2(pointRays[0].m_vDirection[0], pointRays[0].m_vDirection[2]) * Math.RAD2DEG;
		float lastAnglesDeg[ERROR_SHAPE_ANGLE_ARRAY_LENGTH];
		float lastAnglesDist[ERROR_SHAPE_ANGLE_ARRAY_LENGTH];
		for (int i; i < ERROR_SHAPE_ANGLE_ARRAY_LENGTH; i++)
		{
			lastAnglesDeg[i] = firstAngleDeg;
		}

		int lastTesselatedIndex = tesselatedPointRays.Count() - 1;

		foreach (int i, SCR_Ray tesselatedPoint : tesselatedPointRays)
		{
			// too slopy!
			float slopeDeg = Math.Atan2(tesselatedPoint.m_vDirection[1], vector.DistanceXZ(tesselatedPoint.m_vDirection, vector.Zero)) * Math.RAD2DEG;
			if (slopeDeg > m_fMaxSlope || slopeDeg < -m_fMaxSlope)
			{
				segment[0] = tesselatedPoint.m_vPosition;
				segment[1] = segment[0] + vector.Up * SLOPE_ERROR_SHAPE_LENGTH;
				s_aErrorShapes.Insert(Shape.CreateLines(Color.RED, ShapeFlags.NOZBUFFER, segment, 2));
			}

			// too tight turn!
			float angleDeg = Math.Atan2(tesselatedPoint.m_vDirection[0], tesselatedPoint.m_vDirection[2]) * Math.RAD2DEG;
			float angleRad = Math.Atan2(tesselatedPoint.m_vDirection[2], tesselatedPoint.m_vDirection[0]);

			float totalDist;
			for (int j = ERROR_SHAPE_ANGLE_ARRAY_LENGTH - 1; j >= 0; j--) // last to first!
			{
				if (lastAnglesDist[j] == 0)
					break;

				float angleDiffDeg = lastAnglesDeg[j] - angleDeg;

				if (angleDiffDeg <= -180 || angleDiffDeg > 180)
					angleDiffDeg = Math.Repeat(180 + angleDiffDeg, 360) - 180;

				if (angleDiffDeg < 0)
					angleDiffDeg *= -1;

				if (angleDiffDeg > m_fMaxAngle)
				{
					segment[0] = tesselatedPoint.m_vPosition + { Math.Cos(angleRad - Math.PI_HALF), 0, Math.Sin(angleRad - Math.PI_HALF) } * (ANGLE_ERROR_LINE_LENGTH * 0.5);
					segment[1] = tesselatedPoint.m_vPosition + { Math.Cos(angleRad + Math.PI_HALF), 0, Math.Sin(angleRad + Math.PI_HALF) } * (ANGLE_ERROR_LINE_LENGTH * 0.5);
					s_aErrorShapes.Insert(Shape.CreateLines(Color.RED, ShapeFlags.NOZBUFFER, segment, 2));
					break;
				}

				totalDist += lastAnglesDist[j];
				if (totalDist > ERROR_SHAPE_ANGLE_CHECK_LENGTH)
					break;
			}

			// shift everything back by one
			for (int j, jMax = ERROR_SHAPE_ANGLE_ARRAY_LENGTH - 1; j < jMax; j++)
			{
				lastAnglesDeg[j] = lastAnglesDeg[j + 1];
				lastAnglesDist[j] = lastAnglesDist[j + 1];
			}

			lastAnglesDeg[ERROR_SHAPE_ANGLE_ARRAY_LENGTH - 1] = angleDeg;
			if (i != lastTesselatedIndex)
				lastAnglesDist[ERROR_SHAPE_ANGLE_ARRAY_LENGTH - 1] = vector.DistanceXZ(tesselatedPoint.m_vPosition, tesselatedPointRays[i + 1].m_vPosition);
		}

	}

	//------------------------------------------------------------------------------------------------
	protected void PrintDebugInfo(notnull SCR_ShapeAnalyser shapeAnalyser)
	{
		array<ref SCR_Ray> pointRays = shapeAnalyser.GetAbsoluteAnchorPointRays();
		array<ref SCR_Ray> tesselatedPointRays = shapeAnalyser.GetAbsoluteTesselatedPointRays();
		array<float> polygon2D = {};

		array<float> stats = shapeAnalyser.GetStats();

		int statIndex;
		float length2D = stats[statIndex++];
		float length3D = stats[statIndex++];
		float surface	= stats[statIndex++];
		float minAlt	= stats[statIndex++], maxAlt	= stats[statIndex++];
		float minSlope	= stats[statIndex++], maxSlope	= stats[statIndex++];

		float minAltATL = float.INFINITY, maxAltATL = -float.INFINITY;

		SCR_Ray prevPointRay;
		foreach (int i, SCR_Ray pointRay : tesselatedPointRays)
		{
			if (prevPointRay)
			{
				length2D += vector.DistanceXZ(prevPointRay.m_vPosition, pointRay.m_vPosition);
				length3D += vector.Distance(prevPointRay.m_vPosition, pointRay.m_vPosition);
			}

			float tmp = pointRay.m_vPosition[1] - GetWorld().GetSurfaceY(pointRay.m_vPosition[0], pointRay.m_vPosition[2]);

			if (tmp < minAltATL)
				minAltATL = tmp;

			if (tmp > maxAltATL)
				maxAltATL = tmp;

			prevPointRay = pointRay;
		}

		Print(string.Format("%1 pointRays, %2 tesselated pointRays", pointRays.Count(), tesselatedPointRays.Count()), LogLevel.NORMAL);
		Print("Length 2D/3D     : " + length2D.ToString(lenDec: 2) + "m / " + length3D.ToString(lenDec: 2) + "m", LogLevel.NORMAL);

		if (shapeAnalyser.IsClosed())
			Print("Surface area     : " + SCR_Math2D.GetPolygonArea(polygon2D).ToString(lenDec: 2) + "m sq.", LogLevel.NORMAL);
		else
			Print("Surface area     : not an area", LogLevel.NORMAL);

		Print("Min/Max slope    : [" + (minSlope * Math.RAD2DEG).ToString(lenDec: 2) + "deg / " + (maxSlope * Math.RAD2DEG).ToString(lenDec: 2) + "deg]", LogLevel.NORMAL);
		Print("Min/Max altitude : [" + minAlt.ToString(lenDec: 2) + "m  / " + maxAlt.ToString(lenDec: 2) + "m]", LogLevel.NORMAL);
		Print("Min/Max alt ATL  : [" + minAltATL.ToString(lenDec: 2) + "m  / " + maxAltATL.ToString(lenDec: 2) + "m]", LogLevel.NORMAL);
	}

#endif // WORKBENCH

}
