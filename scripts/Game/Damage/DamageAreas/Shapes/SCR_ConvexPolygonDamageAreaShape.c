class SCR_ConvexPolygonDamageAreaShape : SCR_BaseDamageAreaShape
{
	[Attribute(desc: "List of poin infos defining the convex boundry area")]
	private ref array<ref SCR_BoundsPosition> m_aBounds;

	[Attribute(defvalue: "1", desc: "Height of the boundries", params: "0.1 inf 0.01")]
	protected float m_fHeight;

	protected ref array<vector> m_aSimpleBoundsLS;
	protected float m_fDiameter;

#ifdef ENABLE_DIAG
	static bool s_bSupressChangedPropertyWarnings;
	const string PROPERTY_OFFSET = "Offset";
	const string PROPERTY_BOUNDS = "m_aBounds";
	const string PROPERTY_HEIGHT = "m_fHeight";

	//------------------------------------------------------------------------------------------------
	override void DrawDebug(IEntity owner, inout SCR_DebugShapeManager shapeMgr, bool runtime = false)
	{
		if (!m_aSimpleBoundsLS)
			return;

		array<vector> positions = {};
		foreach (vector point : m_aSimpleBoundsLS)
		{
			positions.Insert(owner.CoordToParent(point));
		}

		if (positions.Count() < 3)
			return;

		ShapeFlags flag;
		int color = Color.RED;
		if (runtime)
			flag = ShapeFlags.ONCE; // once as in runtime it is called every frame and we dont have a nice point to clear them, thus they are disposable
		else
			color = Color.WHITE;

		vector mat[4];
		owner.GetTransform(mat);

		shapeMgr.AddBounds(positions, m_fHeight, mat[1], color, additionalFlags: flag);
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (s_bSupressChangedPropertyWarnings)
			return false;

		if (key.IsEmpty())
			return false;

		if (key != PROPERTY_OFFSET && key != PROPERTY_BOUNDS && key != PROPERTY_HEIGHT)
			return false;

		Print("SCR_ConvexPolygonDamageAreaShape -> NOTICE! After you are finished adjusting the bounds, use World Editor -> Plugins -> Optimize Damage Area Shapes", LogLevel.WARNING);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OptimizeShape(notnull IEntitySource ownerSource, inout notnull WorldEditorAPI api, notnull array<ref ContainerIdPathEntry> shapePath, out vector offset)
	{
		if (!m_aSimpleBoundsLS)
			return false;

		const int numberOfPoints = m_aSimpleBoundsLS.Count();
		if (numberOfPoints < 1)
			return false; // nothing to recenter

		// Find lowest and highest point
		float low = float.MAX;
		float high = int.MIN;
		foreach (vector point : m_aSimpleBoundsLS)
		{
			if (point[1] < low)
				low = point[1];

			if (point[1] > high)
				high = point[1];
		}

		// Recalculate the max height based on the lowest point
		m_fHeight = high - low + m_fHeight; // total height

		// Readjust their positions and find their center
		foreach (int i, vector point : m_aSimpleBoundsLS)
		{
			point[1] = low; // set all of them to the same Y position
			m_aSimpleBoundsLS[i] = point;
			offset += point;
			offset += point + vector.Up * m_fHeight;
		}

		if (offset == vector.Zero ||
			(float.AlmostEqual(offset[0], 0) && float.AlmostEqual(offset[1], 0) && float.AlmostEqual(offset[2], 0))) // if its off by float point precision then we dont care
			return false; // its already centered

		// Actual center in local space
		offset /= numberOfPoints * 2;

		array<ref ContainerIdPathEntry> finalPath = {};
		foreach (ContainerIdPathEntry pathEntry : shapePath)
		{
			finalPath.Insert(ContainerIdPathEntry.Cast(pathEntry.Clone()));
		}

		// Disable printing that something has changed in this prefab to not spam
		SCR_ConvexPolygonDamageAreaShape.s_bSupressChangedPropertyWarnings = true;

		// Update entity properties for this shape
		api.SetVariableValue(ownerSource, finalPath, PROPERTY_HEIGHT, m_fHeight.ToString());

		finalPath.Insert(new ContainerIdPathEntry(PROPERTY_BOUNDS));
		int lastElementId = finalPath.Count() - 1;
		foreach (int entryId, vector oldPos : m_aSimpleBoundsLS)
		{
			finalPath[lastElementId].Index = entryId;

			vector newPos = oldPos - offset;
			api.SetVariableValue(ownerSource, finalPath, PROPERTY_OFFSET, newPos.ToString(false));
		}

		SCR_ConvexPolygonDamageAreaShape.s_bSupressChangedPropertyWarnings = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Diag_DrawArea(int color)
	{
		if (!GetGame().InPlayMode())
			return;

		IEntity owner = GetDamageArea().GetParent();
		SCR_DamageAreaComponent dmgAreaComp = SCR_DamageAreaComponent.Cast(owner.FindComponent(SCR_DamageAreaComponent));
		SCR_DebugShapeManager shapeMgr = dmgAreaComp.GetDebugShapeMgr();
		DrawDebug(owner, shapeMgr, true);
	}
#endif

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity owner)
	{
		if (!m_aBounds)
			return;

		vector pos;

		float maxDistanceSq, currentDistSq;
		m_aSimpleBoundsLS = {};
		foreach (SCR_BoundsPosition point : m_aBounds)
		{
			if (!point)
				continue;

			// transfer from point infos to simple vectors for smaller memory footprint
			pos = point.Offset;
			m_aSimpleBoundsLS.Insert(pos);

			currentDistSq = pos.LengthSq();
			if (currentDistSq > maxDistanceSq)
				maxDistanceSq = currentDistSq;

			vector upperCorner = pos + vector.Up * m_fHeight;
			currentDistSq = upperCorner.LengthSq();
			if (currentDistSq > maxDistanceSq)
				maxDistanceSq = currentDistSq;
		}

		m_fDiameter = Math.Sqrt(maxDistanceSq) * 2;
		m_aBounds = null;
	}

	//------------------------------------------------------------------------------------------------
	override float GetDiameter()
	{
		return m_fDiameter;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldAffectEntity(IEntity entityToAffect)
	{
		vector worldPos;
		ChimeraCharacter character = ChimeraCharacter.Cast(entityToAffect);
		if (character)
		{
			worldPos = character.AimingPosition();
		}
		else
		{
			Physics phys = entityToAffect.GetPhysics();
			if (phys)
				worldPos = phys.GetCenterOfMass();
			else
				worldPos = entityToAffect.GetOrigin();
		}

		return AffectsPosition(worldPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if this position is contained inside of the shape
	//! This method is primarily called by the debug to validate if the point is within the shape
	override bool AffectsPosition(vector position)
	{
		const IEntity owner = GetDamageArea().GetParent();
		// translate to local space for easier evaluation with vectors from m_aSimpleBoundsLS
		const vector positionCopy = owner.CoordToLocal(position);

		// all shapes should be optimized with the puling, which will make the entity to be in the center of the shape
		// thus half height determines if Y component is outside of the area of influence
		const float halfHeight = m_fHeight * 0.5;
		if (positionCopy[1] > halfHeight || positionCopy[1] < -halfHeight)
			return false;

		const vector upOffset = {0, m_fHeight, 0};

		vector previousCorner, nextCorner, previousPlaneDir, nextPlaneDir, previousNormal, nextNormal, directionToPoint;
		float previousDot, nextDot;
		int lastPointId = m_aSimpleBoundsLS.Count() - 1;

		// check if point is on the right side of the previous plane, and on the left side of the next plane
		// to ensure that point is within the wedge
		foreach (int i, vector corner : m_aSimpleBoundsLS)
		{
			if (i > 0)
				previousCorner = m_aSimpleBoundsLS[i - 1];
			else
				previousCorner = m_aSimpleBoundsLS[lastPointId];

			if (i < lastPointId)
				nextCorner = m_aSimpleBoundsLS[i + 1];
			else
				nextCorner = m_aSimpleBoundsLS[0];

			previousPlaneDir = previousCorner - corner;
			nextPlaneDir = nextCorner - corner;
			previousNormal = SCR_Math3D.Cross(upOffset, previousPlaneDir, true);
			nextNormal = SCR_Math3D.Cross(upOffset, nextPlaneDir, true);
			directionToPoint = positionCopy - corner;
			previousDot = vector.Dot(previousNormal, directionToPoint);
			nextDot = vector.Dot(nextNormal, directionToPoint);

			// if both dots have the same sign, then the point is on the wrong side for one of them
			if (previousDot * nextDot > 0)
				return false; // its outside of the wedge
		}

		return true; // its somewhere inside of the shape
	}
}
