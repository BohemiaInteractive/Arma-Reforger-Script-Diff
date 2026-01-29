class SCR_ParallelShapeHelper // should be named SCR_OffsetShapeHelper
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntity
	//! \param[in] offset
	//! \param[out] offsetAnchorPoints
	//! \param[out] offsetTesselatedPoints
	//! \param[out] offsetAnchorIndices not created/filled if not provided
	//! \return true on success, false on failure (e.g not enough points, etc)
	[Obsolete("This method taking a float offset is replaced with the vector one")] // obsolete since 2024-09-30
	static bool GetAnchorsAndTesselatedPointsFromShape(notnull ShapeEntity shapeEntity, float offset, out notnull array<vector> offsetAnchorPoints, out notnull array<vector> offsetTesselatedPoints, out array<int> offsetAnchorIndices = null)
	{
		return GetAnchorsAndTesselatedPointsFromShape(shapeEntity, (vector){ offset, 0, 0 }, true, offsetAnchorPoints, offsetTesselatedPoints, offsetAnchorIndices);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntity
	//! \param[in] offset
	//! \param[in] yOffsetInShapeSpace offset Y in shape's up vector, otherwise in point's up vector
	//! \param[out] offsetAnchorPoints
	//! \param[out] offsetTesselatedPoints
	//! \param[out] offsetAnchorIndices not created/filled if not provided
	//! \return true on success, false on failure (e.g not enough points, no out arrays provided, etc)
	static bool GetAnchorsAndTesselatedPointsFromShape(notnull ShapeEntity shapeEntity, vector offset, bool yOffsetInShapeSpace, out notnull array<vector> offsetAnchorPoints, out notnull array<vector> offsetTesselatedPoints, out array<int> offsetAnchorIndices = null)
	{
		if (!offsetAnchorPoints && !offsetTesselatedPoints && !offsetAnchorIndices)
			return false;

		array<vector> anchorPoints = {};
		shapeEntity.GetPointsPositions(anchorPoints);
		int anchorPointsCount = anchorPoints.Count();
		if (anchorPointsCount < 2)
			return false;

		// spline
		array<vector> tesselatedPoints = {};
		shapeEntity.GenerateTesselatedShape(tesselatedPoints);
		int tesselatedPointsCount = tesselatedPoints.Count();
		if (tesselatedPointsCount < anchorPointsCount)
			return false;

		if (anchorPoints[0] != tesselatedPoints[0])
			return false;

		bool isShapeClosed = shapeEntity.IsClosed();

#ifdef WORKBENCH
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (worldEditorAPI)
		{
			IEntitySource shapeEntitySource = worldEditorAPI.EntityToSource(shapeEntity);
			if (shapeEntitySource)
			{
				shapeEntitySource.Get("IsClosed", isShapeClosed);
				if (SplineShapeEntity.Cast(shapeEntity) && isShapeClosed != shapeEntity.IsClosed()) // discrepancyyy
					Print("Inverted \"Is Closed\" Spline tesselated points due to only having the shape's previous state!", LogLevel.WARNING);
			}
		}
#endif // WORKBENCH

		if (isShapeClosed)
		{
			anchorPoints.Insert(anchorPoints[0]);
			++anchorPointsCount;

			if (!SplineShapeEntity.Cast(shapeEntity)) // spline already has the first tesselated point as last point
			{
				tesselatedPoints.Insert(tesselatedPoints[0]);
				++tesselatedPointsCount;
			}
		}

		int anchorIndex;
		vector anchorPos = anchorPoints[anchorIndex];
		array<int> anchorIndices = {};
		foreach (int i, vector tesselatedPoint : tesselatedPoints)
		{
			if (tesselatedPoint == anchorPos)
			{
				anchorIndices.Insert(i);
				++anchorIndex;
				if (anchorIndex < anchorPointsCount)
					anchorPos = anchorPoints[anchorIndex];
			}
		}

		offsetTesselatedPoints.Copy(GetOffsetPointsFromPoints(tesselatedPoints, offset, yOffsetInShapeSpace, isShapeClosed));

		offsetAnchorPoints.Clear();
		foreach (int index : anchorIndices)
		{
			offsetAnchorPoints.Insert(offsetTesselatedPoints[index]);
		}

		if (offsetAnchorIndices)
			offsetAnchorIndices.Copy(anchorIndices);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \param[in] offset
	//! \param[in] isShapeClosed
	//! \return relative offset points
	static array<vector> GetRelativeOffsetPointsFromShape(notnull ShapeEntity shapeEntity, float offset, bool isShapeClosed)
	{
		array<vector> result = {};
		shapeEntity.GenerateTesselatedShape(result);

		if (offset != 0)
			result = GetOffsetPointsFromPoints(result, offset, isShapeClosed);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \param[in] deletionRadius 3D deletion radius - must be greater than zero
	//! \param[in,out] points points relative to the shape
	//! \return true on success, false on failure (wrong points, etc)
	static bool RemovePointsCloseToShape(notnull ShapeEntity shapeEntity, float deletionRadius, inout notnull array<vector> points)
	{
		if (deletionRadius <= 0)
			return false;

		SCR_ShapeDistanceRuler shapeRuler = SCR_ShapeDistanceRuler.CreateFromShape(shapeEntity);
		for (int i = points.Count() - 1; i >= 0; --i)
		{
			if (shapeRuler.IsWithinDistance(points[i], deletionRadius, false))
				points.RemoveOrdered(i);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \param[in] deletionRadius 2D deletion radius - must be greater than zero
	//! \param[in,out] points
	//! \return true on success, false on failure (wrong points, deletionRadius <= 0 etc)
	static bool RemovePointsCloseToShapeXZ(notnull ShapeEntity shapeEntity, float deletionRadius, inout notnull array<vector> points)
	{
		if (deletionRadius <= 0)
			return false;

		int count = points.Count();
		if (count < 1)
			return false;

		SCR_ShapeDistanceRuler shapeRuler = SCR_ShapeDistanceRuler.CreateFromShape(shapeEntity);
		for (int i = count - 1; i >= 0; --i)
		{
			if (shapeRuler.IsWithinDistance(points[i], deletionRadius, false))
				points.RemoveOrdered(i);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \param[in,out] points
	//! \return
	static bool SnapRelativePointsToGround(notnull ShapeEntity shapeEntity, inout notnull array<vector> points)
	{
		vector worldPos;
		BaseWorld world = shapeEntity.GetWorld();
		foreach (int i, vector point : points)
		{
			worldPos = shapeEntity.CoordToParent(point);
			worldPos[1] = world.GetSurfaceY(worldPos[0], worldPos[2]);
			points[i] = shapeEntity.CoordToLocal(worldPos);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] minDistanceBetweenPoints 3D distance between consecutive points
	//! \param[in,out] points min 3 points
	//! \return true on success, false on wrong input (less than 3 points)
	static bool ReducePointsDensity(float minDistanceBetweenPoints, inout notnull array<vector> points)
	{
		int pointsCount = points.Count();
		if (pointsCount < 3)
			return false;

		if (minDistanceBetweenPoints == 0)
			return true;

		float minDistanceBetweenPointsSq = minDistanceBetweenPoints * minDistanceBetweenPoints;

		for (int i = pointsCount - 2; i >= 1; --i) // from count -1 to 1, avoiding first and last elements
		{
			if (vector.DistanceSq(points[i + 1], points[i]) < minDistanceBetweenPointsSq)
				points.RemoveOrdered(i); // TODO: maybe a better filter like point average etc
			else
			if (i == 1 && vector.DistanceSq(points[0], points[1]) < minDistanceBetweenPointsSq)
				points.RemoveOrdered(i);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] minDistanceBetweenPoints 2D distance between consecutive points
	//! \param[in,out] points
	//! \return true on success, false on wrong input (less than 3 points)
	static bool ReducePointsDensityXZ(float minDistanceBetweenPoints, inout notnull array<vector> points)
	{
		int pointsCount = points.Count();
		if (pointsCount < 3)
			return false;

		float minDistanceBetweenPointsSq = minDistanceBetweenPoints * minDistanceBetweenPoints;

		for (int i = pointsCount - 2; i >= 1; --i) // from count -1 to 1, avoiding first and last elements
		{
			if (vector.DistanceSqXZ(points[i + 1], points[i]) < minDistanceBetweenPointsSq)
				points.RemoveOrdered(i); // TODO: maybe a better filter like point average etc
			else
			if (i == 1 && vector.DistanceSqXZ(points[0], points[1]) < minDistanceBetweenPointsSq)
				points.RemoveOrdered(i);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A helpful wrapper for users to get offset points easily
	//! \param[in] points
	//! \param[in] offset
	//! \param[in] isShapeClosed
	//! \return
	static array<vector> GetOffsetPointsFromPoints(notnull array<vector> points, float offset, bool isShapeClosed)
	{
		return GetOffsetPointsFromPoints(points, (vector){ offset, 0, 0 }, true, isShapeClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! A helpful wrapper for users to get offset points easily
	//! \param[in] points
	//! \param[in] offset
	//! \param[in] yOffsetInShapeSpace offset Y in shape's up vector, otherwise in point's up vector
	//! \param[in] isShapeClosed
	//! \return
	static array<vector> GetOffsetPointsFromPoints(notnull array<vector> points, vector offset, bool yOffsetInShapeSpace, bool isShapeClosed)
	{
		int count = points.Count();
		if (count < 2)
			return null;

		if (offset == vector.Zero)
			return SCR_ArrayHelperT<vector>.GetCopy(points);

		return GetOffsetPointsFromRays(GetRaysFromPoints(points, isShapeClosed), offset, yOffsetInShapeSpace);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] points a minimum of two points is required
	//! \param[in] isShapeClosed
	//! \return array of point rays or null on less than two points
	static array<ref SCR_Ray> GetRaysFromPoints(notnull array<vector> points, bool isShapeClosed)
	{
		int countMinus1 = points.Count() - 1;
		if (countMinus1 < 1) // less than two points
			return null;

		array<ref SCR_Ray> result = {};
		result.Reserve(countMinus1 + 1);

		vector prevPoint;
		SCR_Ray ray;
		foreach (int i, vector currPoint : points)
		{
			ray = new SCR_Ray();
			ray.m_vPosition = currPoint;

			if (i == 0)					// first point
			{
				if (isShapeClosed)
					ray.m_vDirection = vector.Direction(points[countMinus1 - 1], points[1]);
				else
					ray.m_vDirection = vector.Direction(currPoint, points[1]);
			}
			else if (i < countMinus1)	// middle point
			{
				ray.m_vDirection = vector.Direction(
					vector.Direction(currPoint, prevPoint).Normalized(),
					vector.Direction(currPoint, points[i + 1]).Normalized()
				);
			}
			else						// last point
			{
				if (isShapeClosed)
					ray.m_vDirection = vector.Direction(prevPoint, points[1]);
				else
					ray.m_vDirection = vector.Direction(prevPoint, currPoint);
			}

			ray.m_vDirection.Normalize();
			result.Insert(ray);

			prevPoint = currPoint;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Modifies the provided array and flattens rays to 2D direction (XZ plane)
	//! \param[in,out] rays
	static void FlattenRays(inout notnull array<ref SCR_Ray> rays)
	{
		foreach (SCR_Ray ray : rays)
		{
			ray.m_vDirection[1] = 0;
			ray.m_vDirection.Normalize();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rays can be of any size
	//! \param[in] offset
	//! \return
	static array<vector> GetOffsetPointsFromRays(notnull array<ref SCR_Ray> rays, float offset)
	{
		return GetOffsetPointsFromRays(rays, (vector){ offset, 0, 0 }, true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rays can be of any size
	//! \param[in] offset
	//! \param[in] yOffsetInShapeSpace offset Y in shape's up vector, otherwise in point's up vector
	//! \return
	static array<vector> GetOffsetPointsFromRays(notnull array<ref SCR_Ray> rays, vector offset, bool yOffsetInShapeSpace)
	{
		int countMinus1 = rays.Count() - 1;
		if (countMinus1 < 0) // empty
			return {};

		if (countMinus1 < 1) // 1 element
			return { GetOffsetPoint(rays[0].m_vPosition, rays[0].m_vDirection, offset, yOffsetInShapeSpace) };

		vector prevNewPoint, currNewPoint;
		SCR_Ray prevRay, nextRay;
		array<vector> result = {};
		result.Reserve(countMinus1 + 1);

		foreach (int i, SCR_Ray currRay : rays)
		{
			if (i == 0 || i == countMinus1)	// first or last
			{
				currNewPoint = GetOffsetPoint(currRay.m_vPosition, currRay.m_vDirection, offset, yOffsetInShapeSpace);
			}
			else							// middle point
			{
				nextRay = rays[i + 1];

				vector currRayOffsetPos = GetOffsetPoint(currRay.m_vPosition, currRay.m_vDirection, offset, yOffsetInShapeSpace);
				vector nextRayOffsetPos = GetOffsetPoint(nextRay.m_vPosition, nextRay.m_vDirection, offset, yOffsetInShapeSpace);

				float angleRad0 = SCR_Math2D.DegreeToTrigoRadian(vector.Direction(prevNewPoint, currRayOffsetPos).Normalized().ToYaw());
				float angleRad1 = SCR_Math2D.DegreeToTrigoRadian(vector.Direction(nextRayOffsetPos, currRayOffsetPos).Normalized().ToYaw());

				float x, z;
				bool intersects = SCR_Math2D.GetLinesIntersectionXZ(
					prevNewPoint[0], prevNewPoint[2], angleRad0,
					nextRayOffsetPos[0], nextRayOffsetPos[2], angleRad1,
					x, z);

				if (intersects)
				{
					currNewPoint = { x, currRay.m_vPosition[1] + offset[1], z };
				}
				else // ALAAARM https://www.youtube.com/watch?v=hl8e9i6YiA8&t=52s - this should NEVER happen but hey, it does.
				{
					currNewPoint = vector.Lerp(prevNewPoint, nextRayOffsetPos, 0.5);
					PrintFormat(
						"[SCR_ParallelShapeHelper.GetOffsetPointsFromRays] does not intersect at point %1/%2 (most likely a straight line), using middle point %3 (%4 L%5)",
						i, countMinus1 + 1, currNewPoint, __FILE__, __LINE__,
						level: LogLevel.DEBUG);

					PrintFormat(
						"[SCR_ParallelShapeHelper.GetOffsetPointsFromRays] prev %1 at %2rad → %3 at %4rad next",
						prevNewPoint, angleRad0.ToString(-1, 2),
						nextRayOffsetPos, angleRad1.ToString(-1, 2),
						level: LogLevel.DEBUG);
				}
			}

			result.Insert(currNewPoint);

			prevRay = currRay;
			prevNewPoint = currNewPoint;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] position
	//! \param[in] direction
	//! \param[in] offset
	//! \param[in] yOffsetInShapeSpace offset Y in absolute vector up, otherwise in direction's up
	//! \return
	protected static vector GetOffsetPoint(vector position, vector direction, vector offset, bool yOffsetInShapeSpace)
	{
		vector mat[4];
		if (yOffsetInShapeSpace)
		{
			direction[1] = 0;
			direction.Normalize();
		}

		Math3D.DirectionAndUpMatrix(direction, vector.Up, mat);
		return position + offset.Multiply4(mat);
	}

	//------------------------------------------------------------------------------------------------
	// constructor - not available for creation
	protected void SCR_ParallelShapeHelper();
}
