[EntityEditorProps(category: "GameScripted/Generators", description: "Road Generator - use this instead of RoadGeneratorEntity", visible: false, dynamicBox: true)]
class SCR_RoadGeneratorEntityClass : RoadGeneratorEntityClass
{
}

class SCR_RoadGeneratorEntity : RoadGeneratorEntity
{
	/*
		Debug
	*/

	[Attribute(defvalue: "10", category: "Debug", desc: "(metres) 2D distance between checks", uiwidget: UIWidgets.Slider, params: "1 100 1")]
	protected float m_fMeasurementStep;

	[Attribute(defvalue: MAX_POSSIBLE_ANGLE.ToString(), category: "Debug", desc: "(degrees) Maximum acceptable absolute slope for the road.\nPercent to degree formula: Math.Atan2(7 / 100, 1) * Math.RAD2DEG == 4.00", uiwidget: UIWidgets.Slider, params: "0 90 0.01")]
	protected float m_fMaxSlopeAngle;

	[Attribute(defvalue: MAX_POSSIBLE_ANGLE.ToString(), category: "Debug", desc: "(degrees) Maximum acceptable absolute slope for the roadside compared to its middle (checks both sides)", uiwidget: UIWidgets.Slider, params: "0 90 0.01")]
	protected float m_fMaxRollAngle;

	[Attribute(defvalue: "1", category: "Debug", desc: "Check road roll angle per side (middle to left and middle to right), otherwise from left to right")]
	protected bool m_bRollCheckPerSide;

	protected ref SCR_DebugShapeManager m_DebugShapeManager = new SCR_DebugShapeManager();

	protected static const float MAX_POSSIBLE_ANGLE = 90;

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	protected void CheckSlopes(bool printDebugLink)
	{
		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return;

		m_DebugShapeManager.Clear();

		if (m_fMaxSlopeAngle >= MAX_POSSIBLE_ANGLE && m_fMaxRollAngle >= MAX_POSSIBLE_ANGLE)
			return;

		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return;

		IEntitySource thisSource = worldEditorAPI.EntityToSource(this);
		if (!thisSource)
			return;

		IEntitySource parentSource = thisSource.GetParent();
		if (!parentSource)
			return;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(parentSource));

		SCR_ShapeNextPointHelper shapeNextPointHelper = SCR_ShapeNextPointHelper.CreateFromShape(shapeEntity);
		if (!shapeNextPointHelper)
		{
			Print("Cannot check road generator slope with invalid shape at " + Debug.GetEntityLinkString(shapeEntity), LogLevel.WARNING);
			return;
		}

		float shapeLength = shapeNextPointHelper.GetShapeLength();
		if (shapeLength <= 0.02) // boo
			return;

		float measurementStep;
		if (shapeLength <= m_fMeasurementStep)
			measurementStep = shapeLength - 0.01;
		else
			measurementStep = m_fMeasurementStep;

		bool checkSlope = m_fMaxSlopeAngle < MAX_POSSIBLE_ANGLE;

		float roadWidth;
		bool checkSides = thisSource.Get("RoadWidth", roadWidth) && roadWidth > 0;
		if (!checkSides && !checkSlope) // neither checkSides or checkSlope, checkNothing is not an option
			return;

		float halfRoadWidth = 0.5 * roadWidth;

		bool hasIssues;
		vector currPoint;
		vector prevPoint = shapeEntity.CoordToParent(shapeNextPointHelper.GetAnchorPoints()[0]);
		prevPoint[1] = world.GetSurfaceY(prevPoint[0], prevPoint[2]);

		while (shapeNextPointHelper.GetNextPoint(measurementStep, currPoint, xzMode: true))
		{
			currPoint = shapeEntity.CoordToParent(currPoint);
			currPoint[1] = world.GetSurfaceY(currPoint[0], currPoint[2]);

			if (checkSlope)
			{
				float angle = Math.Atan2(currPoint[1] - prevPoint[1], measurementStep) * Math.RAD2DEG;

				if (angle < 0)
					angle = -angle;

				if (angle > m_fMaxSlopeAngle)
				{
					float delta01;
					if (m_fMaxSlopeAngle == 0)
						delta01 = 1;
					else
						delta01 = Math.InverseLerp(m_fMaxSlopeAngle, m_fMaxSlopeAngle * 2, angle);

					m_DebugShapeManager.AddLine(prevPoint, currPoint, GetGradientColour(delta01));
					hasIssues = true;
				}
			}

			if (checkSides)
			{
				vector currDir = shapeNextPointHelper.GetCurrentDirection();
				currDir[1] = 0; // to 2D
				currDir.Normalize();
				vector rightVector = (vector.Up * shapeEntity.VectorToParent(currDir)) * halfRoadWidth;
				vector leftPoint = currPoint - rightVector;
				vector rightPoint = currPoint + rightVector;

				vector leftRightPoints[2] = { leftPoint, rightPoint };

				if (m_bRollCheckPerSide)
				{
					foreach (vector sidePoint : leftRightPoints)
					{
						sidePoint[1] = world.GetSurfaceY(sidePoint[0], sidePoint[2]);
						float angle = Math.Atan2(currPoint[1] - sidePoint[1], halfRoadWidth) * Math.RAD2DEG;

						if (angle < 0)
							angle = -angle;

						if (angle > m_fMaxRollAngle)
						{
							float delta01;
							if (m_fMaxRollAngle == 0)
								delta01 = 1;
							else
								delta01 = Math.InverseLerp(m_fMaxRollAngle, m_fMaxRollAngle * 2, angle);

							m_DebugShapeManager.AddLine(currPoint, sidePoint, GetGradientColour(delta01));
							hasIssues = true;
						}
					}
				}
				else // full road width
				{
					foreach (int i, vector sidePoint : leftRightPoints)
					{
						leftRightPoints[i][1] = world.GetSurfaceY(sidePoint[0], sidePoint[2]);
					}

					float angle = Math.Atan2(leftRightPoints[0][1] - leftRightPoints[1][1], roadWidth) * Math.RAD2DEG;

					if (angle < 0)
						angle = -angle;

					if (angle > m_fMaxRollAngle)
					{
						float delta01;
						if (m_fMaxRollAngle == 0)
							delta01 = 1;
						else
							delta01 = Math.InverseLerp(m_fMaxRollAngle, m_fMaxRollAngle * 2, angle);

						m_DebugShapeManager.AddLine(leftRightPoints[0], leftRightPoints[1], GetGradientColour(delta01));
						hasIssues = true;
					}
				}
			}

			prevPoint = currPoint;
		}

		if (printDebugLink && hasIssues)
			Print("[SCR_RoadGeneratorEntity.CheckSlope] shape has slope issue " + Debug.GetEntityLinkString(shapeEntity), LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetGradientColour(float value01)
	{
		if (value01 < 0)
			return 0xFF00FF00;

		if (value01 > 1)
			return 0xFFFF0000;

		int r, g; // b is always 0, unused
		if (value01 <= 0.5)
		{
			r = 255 * (2 * value01);
			g = 255;
		}
		else
		{
			r = 255;
			g = 255 * (1 - 2 * (value01 - 0.5));
		}

		return (0xFF << 24) | (r << 16) | (g << 8); // | (0 << 0)
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		CheckSlopes(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		bool result = super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		CheckSlopes(false);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override event void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		CheckSlopes(true);
	}

#endif // WORKBENCH
}
