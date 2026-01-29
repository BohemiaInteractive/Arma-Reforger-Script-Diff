class SCR_ShapeDistanceRuler
{
	vector m_aShapeMatrix[4];
	protected ref array<ref array<vector>> m_aRelativeSegments;

	//------------------------------------------------------------------------------------------------
	//! \param[in] relativePos
	//! \return the 3D distance from shape, -1 on error (e.g no shape points)
	float GetDistance(vector relativePos)
	{
		float resultSq = float.INFINITY;
		foreach (int i, array<vector> segment : m_aRelativeSegments)
		{
			float distanceSq = Math3D.PointLineSegmentDistanceSqr(relativePos, segment[0], segment[1]);
			if (distanceSq < resultSq)
				resultSq = distanceSq;
		}

		if (resultSq == float.INFINITY)
			return -1;

		return Math.Sqrt(resultSq);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] relativePos
	//! \return the 2D distance from shape, -1 on error (e.g no shape points)
	float GetDistanceXZ(vector relativePos)
	{
		relativePos[1] = 0;

		float resultSq = float.INFINITY;
		foreach (int i, array<vector> segment : m_aRelativeSegments)
		{
			vector start = segment[0];
			vector end = segment[1];
			start[1] = 0;
			end[1] = 0;

			float distanceSq = Math3D.PointLineSegmentDistanceSqr(relativePos, start, end);
			if (distanceSq < resultSq)
				resultSq = distanceSq;
		}

		if (resultSq == float.INFINITY)
			return -1;

		return Math.Sqrt(resultSq);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] relativePos the world position to check
	//! \param[in] distance the checked distance
	//! \param[in] equalCounts makes the less than < check into a less than or equal <= check
	//! \return true if provided relativePos is within shape distance, false otherwise
	bool IsWithinDistance(vector relativePos, float distance, bool equalCounts = true)
	{
		distance *= distance; // square it
		foreach (int i, array<vector> segment : m_aRelativeSegments)
		{
			if (equalCounts)
			{
				if (Math3D.PointLineSegmentDistanceSqr(relativePos, segment[0], segment[1]) <= distance)
					return true;
			}
			else
			{
				if (Math3D.PointLineSegmentDistanceSqr(relativePos, segment[0], segment[1]) < distance)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] relativePos the relative position to check
	//! \param[in] distance the checked distance
	//! \param[in] equalCounts makes the less than < check into a less than or equal <= check
	//! \return true if provided relativePos is within shape distance, false otherwise
	bool IsWithinDistanceXZ(vector relativePos, float distance, bool equalCounts = true)
	{
		relativePos[1] = 0;

		distance *= distance; // square it
		foreach (int i, array<vector> segment : m_aRelativeSegments)
		{
			vector start = segment[0];
			vector end = segment[1];
			start[1] = 0;
			end[1] = 0;

			if (equalCounts)
			{
				if (Math3D.PointLineSegmentDistanceSqr(relativePos, start, end) <= distance)
					return true;
			}
			else
			{
				if (Math3D.PointLineSegmentDistanceSqr(relativePos, start, end) < distance)
					return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldPos
	//! \return
	vector CoordToLocal(vector worldPos)
	{
		return worldPos.InvMultiply4(m_aShapeMatrix);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \return
	static SCR_ShapeDistanceRuler CreateFromShape(notnull ShapeEntity shapeEntity)
	{
		array<vector> tesselatedRelPoints = {};
		shapeEntity.GenerateTesselatedShape(tesselatedRelPoints);

		int countMinus1 = tesselatedRelPoints.Count() - 1;
		if (countMinus1 < 0)
		{
			Print("[SCR_ShapeDistanceRuler] empty shape provided", LogLevel.WARNING);
			return null;
		}

		SCR_ShapeDistanceRuler result = new SCR_ShapeDistanceRuler();
		
		shapeEntity.GetTransform(result.m_aShapeMatrix);
		result.m_aRelativeSegments = {};
		result.m_aRelativeSegments.Reserve(countMinus1);

		vector prevPoint;
		foreach (int i, vector currPoint : tesselatedRelPoints)
		{
			if (i != 0)
				result.m_aRelativeSegments.Insert({ prevPoint, currPoint });

			prevPoint = currPoint;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// constructor - not available for creation
	protected void SCR_ShapeDistanceRuler();
}
