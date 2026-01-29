class SCR_LineGeneratorBaseEntityClass : SCR_GeneratorBaseEntityClass
{
}

//! SCR_LineGeneratorBaseEntity responsibilities:
//! - allow and draw a shape offset from the parent shape ("parallel/virtual" shape)
//! - hold a "next point helper" to get the next point on the parent/offset shape according to the exposed attributes
class SCR_LineGeneratorBaseEntity : SCR_GeneratorBaseEntity
{
	/*
		Shape Usage
	*/

	[Attribute(defvalue: "0 0 0", desc: "-/+ meanings:\n- X: Left/Right (POINT relative)\n- Y: Down/Up (SHAPE relative)\n- Z: Backward/Forward (POINT relative)\n", params: "-100 100", category: "Shape Usage")]
	protected vector m_vShapeOffset;

	[Attribute(defvalue: "1", desc: "Use Y offset in shape space (perpendicular to shape's relative Y), otherwise use in point space (perpendicular to point's next point direction)", category: "Shape Usage")]
	protected bool m_bYOffsetInShapeSpace;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility (since 2024-10-01)
	protected float m_fShapeOffset;

	[Attribute(defvalue: "0.95", desc: "Safety distance from the original spline in which no points can exist\nDistance is measured in 2D or 3D based on Next Point Measurement XZ below and in percentage of Shape Offset above\n- does not apply to Polyline\n- be aware that using Z offset may remove many points", uiwidget: UIWidgets.Slider, params: "0 1 0.01", precision: 2, category: "Shape Usage")]
	protected float m_fSplineSafetyDistanceRatio;

	[Attribute(defvalue: "1", desc: "Offset shape is snapped to the ground", category: "Shape Usage")]
	protected bool m_bSnapOffsetShapeToTheGround;

	[Attribute(defvalue: "1", desc: "The \"next point\" distance is measured in 2D (XZ plane) instead of 3D", category: "Shape Usage")]
	protected bool m_bNextPointMeasurementXZ;

#ifdef WORKBENCH
	protected ref SCR_ShapeNextPointHelper m_ShapeNextPointHelper;
	protected ref SCR_DebugShapeManager m_DbgShpMgr;
	protected ref array<Shape> m_aOffsetShapeShapes = {};
	protected bool m_bIsSelectedAlone;
	protected bool m_bForceOffsetShapeShapesRefresh;

	//------------------------------------------------------------------------------------------------
	protected void ResetShapeNextPointHelper()
	{
		m_ShapeNextPointHelper = null;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		IEntitySource newGeneratorSource = worldEditorAPI.EntityToSource(this);
		if (!newGeneratorSource)
			return; // huh?

		if (!m_ParentShapeSource)
			return;

		ShapeEntity shapeEntity = ShapeEntity.Cast(worldEditorAPI.SourceToEntity(m_ParentShapeSource));
		if (!shapeEntity)
			return;

		array<vector> offsetAnchorPoints = {};
		array<vector> offsetTesselatedPoints = {};
		array<int> anchorIndices = {};
		if (!SCR_ParallelShapeHelper.GetAnchorsAndTesselatedPointsFromShape(shapeEntity, m_vShapeOffset, m_bYOffsetInShapeSpace, offsetAnchorPoints, offsetTesselatedPoints, anchorIndices))
		{
			PrintFormat("[SCR_LineGeneratorBaseEntity.ResetShapeNextPointHelper] error getting shape points from shape with %1 points at position %2", shapeEntity.GetPointCount(), shapeEntity.CoordToParent(shapeEntity.GetOrigin()), level: LogLevel.WARNING);
			return;
		}

		if (m_bSnapOffsetShapeToTheGround)
		{
			BaseWorld world = shapeEntity.GetWorld();
			if (world)
			{
				vector tempVector;
				foreach (int i, vector point : offsetAnchorPoints)
				{
					tempVector = shapeEntity.CoordToParent(point);
					tempVector[1] = world.GetSurfaceY(tempVector[0], tempVector[2]) + m_vShapeOffset[1];
					tempVector = shapeEntity.CoordToLocal(tempVector);

					if (point[0] == tempVector[0] && point[2] == tempVector[2])
						point[1] = tempVector[1];	// avoids changing point's x/z
					else
						point = tempVector;			// unless shape is not vector.Up aligned

					offsetAnchorPoints[i] = point;
				}

				foreach (int i, vector point : offsetTesselatedPoints)
				{
					tempVector = shapeEntity.CoordToParent(point);
					tempVector[1] = world.GetSurfaceY(tempVector[0], tempVector[2]) + m_vShapeOffset[1];
					tempVector = shapeEntity.CoordToLocal(tempVector);

					if (point[0] == tempVector[0] && point[2] == tempVector[2])
						point[1] = tempVector[1];	// avoids changing point's x/z
					else
						point = tempVector;			// unless shape is not vector.Up aligned

					offsetTesselatedPoints[i] = point;
				}
			}
		}

		if (m_vShapeOffset != vector.Zero)
			m_DbgShpMgr = new SCR_DebugShapeManager();
		else
			m_DbgShpMgr = null;

		if (m_fSplineSafetyDistanceRatio != 0 && (m_vShapeOffset[0] != 0 || m_vShapeOffset[2] != 0) && SplineShapeEntity.Cast(shapeEntity) != null) // isSpline
		{
			SCR_ShapeDistanceRuler ruler = SCR_ShapeDistanceRuler.CreateFromShape(shapeEntity);
			float safetyDistance;
			safetyDistance = float.MAX;
			foreach (vector offsetAnchorPoint : offsetAnchorPoints)
			{
				float distance;
				if (m_bNextPointMeasurementXZ)
					distance = ruler.GetDistanceXZ(offsetAnchorPoint);
				else
					distance = ruler.GetDistance(offsetAnchorPoint);

				if (distance < safetyDistance)
					safetyDistance = distance;
			}

			safetyDistance *= m_fSplineSafetyDistanceRatio;

			bool anchorPointRefusal;
			for (int i = offsetTesselatedPoints.Count() - 2; i >= 1; --i) // avoid first and last point
			{
				bool removePoint;
				if (m_bNextPointMeasurementXZ)
					removePoint = ruler.IsWithinDistanceXZ(offsetTesselatedPoints[i], safetyDistance, false);
				else
					removePoint = ruler.IsWithinDistance(offsetTesselatedPoints[i], safetyDistance, false);

				if (removePoint)
				{
					if (anchorIndices.Contains(i))
					{
						anchorPointRefusal = true;
						continue;
					}

					if (m_DbgShpMgr)
						m_DbgShpMgr.AddArrow(shapeEntity.CoordToParent(offsetTesselatedPoints[i] + 2 * vector.Up), shapeEntity.CoordToParent(offsetTesselatedPoints[i]));

					offsetTesselatedPoints.RemoveOrdered(i);
				}
			}

			if (anchorPointRefusal)
			{
				PrintFormat("Cannot remove offset anchor points near %1 shape; only intermediate spline points can be removed", shapeEntity.GetOrigin(), level: LogLevel.NORMAL);
				Print("Solution: make this curve with an anchor point on each side instead of one at the curve's apex", LogLevel.NORMAL);
			}
		}

		m_ShapeNextPointHelper = SCR_ShapeNextPointHelper.CreateFromPoints(offsetAnchorPoints, offsetTesselatedPoints);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		super.OnShapeChangedInternal(shapeEntitySrc, shapeEntity, mins, maxes);

		ResetShapeNextPointHelper();
	}

	//------------------------------------------------------------------------------------------------
	protected override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (!super._WB_OnKeyChanged(src, key, ownerContainers, parent))
			return false;

		m_bForceOffsetShapeShapesRefresh = true;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI || worldEditorAPI.UndoOrRedoIsRestoring())
			return false;

		if (key == "m_fSplineSafetyDistance")
		{
			if (m_vShapeOffset == vector.Zero)
				return false;

			if (m_ParentShapeSource && m_ParentShapeSource.GetClassName().ToType() && m_ParentShapeSource.GetClassName().ToType().IsInherited(PolylineShapeEntity))
				return false;
		}

		BaseContainerTools.WriteToInstance(this, _WB_GetEditorAPI().EntityToSource(this));
		ResetShapeNextPointHelper();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc)
	{
		super._WB_OnParentChange(src, prevParentSrc);

		ResetShapeNextPointHelper();
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_OnInit(inout vector mat[4], IEntitySource src)
	{
		super._WB_OnInit(mat, src);

		ResetShapeNextPointHelper();
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(float timeSlice)
	{
		super._WB_AfterWorldUpdate(timeSlice);

		if (!m_Source || !m_DbgShpMgr)
			return;

		WorldEditorAPI worldEditorAPI = _WB_GetEditorAPI();
		if (!worldEditorAPI)
			return;

		bool isSelectedAlone = worldEditorAPI.IsEntitySelected(m_Source) && worldEditorAPI.GetSelectedEntitiesCount() == 1;
		if (m_bForceOffsetShapeShapesRefresh)
			m_bForceOffsetShapeShapesRefresh = false;
		else if (m_bIsSelectedAlone == isSelectedAlone)
			return;

		m_bIsSelectedAlone = isSelectedAlone;

		foreach (Shape offsetShapeShape : m_aOffsetShapeShapes)
		{
			if (offsetShapeShape)
				m_DbgShpMgr.Remove(offsetShapeShape);
		}

		if (!m_bIsSelectedAlone || m_vShapeOffset == vector.Zero || !m_ShapeNextPointHelper || !m_ParentShapeSource)
			return;

		ShapeEntity shapeEntity = ShapeEntity.Cast(_WB_GetEditorAPI().SourceToEntity(m_ParentShapeSource));
		if (!shapeEntity)
			return;

		// draw offset shape
		array<vector> offsetTesselatedPoints = m_ShapeNextPointHelper.GetTesselatedPoints();
		vector prevPoint;
		foreach (int i, vector point : offsetTesselatedPoints)
		{
			if (i != 0)
				m_aOffsetShapeShapes.Insert(
					m_DbgShpMgr.AddLine(
						shapeEntity.CoordToParent(prevPoint),
						shapeEntity.CoordToParent(point),
						Color.ORANGE));

			prevPoint = point;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		super._WB_GetAfterWorldUpdateSpecs(src);
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

#endif // WORKBENCH
}
