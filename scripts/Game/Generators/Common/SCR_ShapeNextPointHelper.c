//! This helper provides the next point in shape's relative position
//! - anchor points = points that define the shape (the ones draggable in Workbench)
//! - tesselated points = all points of the shape (equals to anchor points for PolylineShapeEntity)
//! - section = anchor to anchor - 1 tesselated points; [anchorA, tessPoint, tessPoint], [anchorB, tessPoint, tessPoint], etc
//! - segment = straight line between two tesselated points
//! - section point index = pointJ <= position < pointK = pointJ

//	TODO:
//		- in shape's length (straightLine parameter)

class SCR_ShapeNextPointHelper
{
	protected ref array<vector> m_aAnchorPoints;				//!< format [anchor0, anchor1, anchor2]
	protected ref array<vector> m_aTesselatedPoints;			//!< format [anchor0, pointA0, pointB0, anchor1, pointA1, pointB1, anchor2]
	protected ref array<int> m_aAnchorPointIndices;				//!< anchor indices in tesselated points
	protected ref array<ref array<vector>> m_aSegments;			//!< format [[segment0startPos, segment0endPos], [segment1startPos, segment1endPos]]
	protected ref array<ref array<vector>> m_aSections;			//!< format [[anchor0, tessPoint, tessPoint, anchor1], [anchor1, tessPoint, tessPoint, anchor2]]
//	protected ref array<ref array<float>> m_aSectionLengths;	//!< format [anchor0to1shapeLength, anchor1to2shapeLength] - shape length = length following the spline
	protected int m_iSegmentsCount;								//!< count of m_aSegments
	protected int m_iAnchorPointsCount;							//!< count of m_aAnchorPoints
//	protected int m_iTesselatedPointsCount;						//!< count of m_aTesselatedPoints

	protected int m_iCurrentSectionIndex;
	protected int m_iCurrentSectionPointIndex;
	protected int m_iCurrentSegmentIndex;
	protected vector m_vCurrentPosition;
	protected vector m_vCurrentDirection;

	protected float m_fDoneLength;
	protected float m_fTotalLength;

	//------------------------------------------------------------------------------------------------
	//! \return anchor points in shape's relative coordinates
	array<vector> GetAnchorPoints()
	{
		return SCR_ArrayHelperT<vector>.GetCopy(m_aAnchorPoints);
	}

	//------------------------------------------------------------------------------------------------
	//! \return anchor point tesselated indices
	array<int> GetAnchorPointIndices()
	{
		return SCR_ArrayHelperT<int>.GetCopy(m_aAnchorPointIndices);
	}

	//------------------------------------------------------------------------------------------------
	//! \return tesselated points in shape's relative coordinates
	array<vector> GetTesselatedPoints()
	{
		return SCR_ArrayHelperT<vector>.GetCopy(m_aTesselatedPoints);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	vector GetCurrentPosition()
	{
		return m_vCurrentPosition;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the normalised direction of the current position
	vector GetCurrentDirection()
	{
		return m_vCurrentDirection;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetShapeLength()
	{
		return m_fTotalLength;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDoneShapeLength()
	{
		return m_fDoneLength;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetRemainingShapeLength()
	{
		return m_fTotalLength - m_fDoneLength;
	}

	//------------------------------------------------------------------------------------------------
	// bruteforce for now, optimisation later
	protected float CalculateCurrentSegmentIndex()
	{
		float currentSegmentIndex;
		for (int i; i < m_iCurrentSectionIndex; ++i)
		{
			currentSegmentIndex += m_aSections[i].Count() - 1;
		}

		return currentSegmentIndex + m_iCurrentSectionPointIndex;
	}

	//------------------------------------------------------------------------------------------------
	// bruteforce for now, optimisation later
	protected float CalculateDoneLength()
	{
		float doneLength;
		foreach (int i, array<vector> segment : m_aSegments)
		{
			if (i >= m_iCurrentSegmentIndex)
			{
				doneLength += vector.Distance(segment[0], m_vCurrentPosition);
				return doneLength;
			}

			doneLength += vector.Distance(segment[0], segment[1]);
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the next point from the current position, but without going over the provided anchor index
	//! \param[in] distance
	//! \param[out] result the requested point
	//! \param[in] anchorLimit 0-based anchor index before which the point must be found; -1 for no limit
	//! \param[in] straightLine [NOT IMPLEMENTED, straight line only] if true, find in straight line distance; if false, finds in shape length (following the shape)
	//! \param[in] doNotMove if true, get the next point without internally moving (next Get will start from the previous position)
	//! \param[in] xzMode look for a 2D distance from the existing point
	//! \return true on success, false on failure
	bool GetNextPoint(float distance, out vector result, int anchorLimit = -1, bool straightLine = true, bool doNotMove = false, bool xzMode = false)
	{
		if (distance <= 0)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPoint's distance parameter cannot be zero or negative!", LogLevel.ERROR);
			return false; // wrong distance
		}

		if (anchorLimit == -1)
			anchorLimit = m_iAnchorPointsCount - 1;

		if (anchorLimit < 1 || anchorLimit >= m_iAnchorPointsCount)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPoint's anchorLimit parameter is invalid (must be in range 1.." + m_iAnchorPointsCount + ", was " + anchorLimit + ")", LogLevel.ERROR);
			return false;
		}

		if (anchorLimit <= m_iCurrentSectionIndex)
		{
			Print("SCR_ShapeNextPointHelper.GetNextPoint's anchorLimit parameter is already passed (" + anchorLimit + " requested, " + m_iCurrentSectionIndex + " current)", LogLevel.WARNING);
			return false;
		}

		float distanceSq = distance * distance;

		int currentSectionIndex = m_iCurrentSectionIndex;
		int currentSectionPoint = m_iCurrentSectionPointIndex + 1;
//		float addedLength;

		if (!m_aSections[currentSectionIndex].IsIndexValid(currentSectionPoint))
		{
			currentSectionIndex++;
			currentSectionPoint = 1; // because end point of previous section = first point of next section
		}

		vector prevPoint = m_aSections[m_iCurrentSectionIndex][m_iCurrentSectionPointIndex];

		array<vector> sectionPoints;
//		array<float> sectionSegmentsLength;
		for (; currentSectionIndex < anchorLimit; currentSectionIndex++)
		{
			sectionPoints = m_aSections[currentSectionIndex];
//			sectionSegmentsLength = m_aSectionLengths[currentSectionIndex];

			for (int pointIndex = currentSectionPoint, sectionPointsCount = sectionPoints.Count(); pointIndex < sectionPointsCount; pointIndex++)
			{
				vector nextPoint = sectionPoints[pointIndex];
//				if (straightLine)
//				{
					if (
						(xzMode && vector.DistanceSqXZ(m_vCurrentPosition, nextPoint) > distanceSq) ||	// 2D mode
						(!xzMode && vector.DistanceSq(m_vCurrentPosition, nextPoint) > distanceSq))		// 3D mode
					{
						vector endToStartDir = vector.Direction(nextPoint, prevPoint);
						if (xzMode)	// 2D mode - there must exist a better formula, todo later
						{
							vector endToStartDirXZ = endToStartDir;
							endToStartDirXZ[1] = 0;
							vector sameLevelCentre = m_vCurrentPosition;
							sameLevelCentre[1] = nextPoint[1];
							float removedRatio;
							if (vector.DistanceXZ(nextPoint, prevPoint) != 0)
								removedRatio =
									Math3D.IntersectionRaySphere(nextPoint, endToStartDirXZ.Normalized(), sameLevelCentre, distance)
									/ vector.DistanceXZ(nextPoint, prevPoint);

							result = nextPoint + removedRatio * endToStartDirXZ;
							result[1] = prevPoint[1] + (1 - removedRatio) * (nextPoint[1] - prevPoint[1]);
						}
						else		// 3D mode
						{
							endToStartDir.Normalize(); // "variable reuse"
							result = nextPoint + endToStartDir * Math3D.IntersectionRaySphere(nextPoint, endToStartDir, m_vCurrentPosition, distance);
						}

						if (!doNotMove)
						{
							m_vCurrentPosition = result;
							m_vCurrentDirection = vector.Direction(prevPoint, nextPoint).Normalized(); // TODO: on vertices
							m_iCurrentSectionIndex = currentSectionIndex;
							m_iCurrentSectionPointIndex = pointIndex - 1;
//							m_fDoneLength += addedLength + vector.Distance(prevPoint, result);

							m_iCurrentSegmentIndex = CalculateCurrentSegmentIndex();
							m_fDoneLength = CalculateDoneLength();
						}

						return true;
					}
//				}
//				else // in shape's length
//				{
//					if (addedLength + sectionSegmentsLength[pointIndex] > distance)
//					{
//						float remainder = distance - addedLength;
//						vector prevToNextDirNormalised = vector.Direction(prevPoint, nextPoint).Normalized();
//						result = prevPoint + prevToNextDirNormalised * remainder;
//
//						if (!doNotMove)
//						{
//							m_vCurrentPosition = result;
//							m_iCurrentSectionIndex = currentSectionIndex;
//							m_iCurrentSectionPointIndex = pointIndex - 1;
//							m_fDoneLength += distance;
//						}
//
//						return true;
//					}
//				}

//				addedLength += sectionSegmentsLength[pointIndex];
				prevPoint = nextPoint;
			}

			currentSectionPoint = 1; // because end point of previous section = first point of next section
		}

		return false; // shape end
	}

	//------------------------------------------------------------------------------------------------
	//! \return shape distance (following curve) from the current point to the next anchor
	protected float GetShapeDistanceToNextAnchor()
	{
		float result = vector.Distance(m_vCurrentPosition, m_aSegments[m_iCurrentSegmentIndex][1]);
		if (result == 0)
			return 0;

		array<vector> segment;
		for (int i = m_iCurrentSegmentIndex + 1; i < m_iSegmentsCount; i++)
		{
			segment = m_aSegments[i];
			result += vector.Distance(segment[0], segment[1]);
			if (m_aAnchorPoints.Contains(segment[1]))
				return result;
		}
//TODO
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return straight distance (not following curve) from the current point to the next anchor
	float GetStraightDistanceToNextAnchor()
	{
		if (m_iCurrentSectionIndex + 1 >= m_iAnchorPointsCount)
			return vector.Distance(m_vCurrentPosition, m_aAnchorPoints[m_iAnchorPointsCount - 1]);
		else
			return vector.Distance(m_vCurrentPosition, m_aAnchorPoints[m_iCurrentSectionIndex + 1]);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets to the provided anchor's position
	//! \param[in] anchorIndex the index of the anchor - while useless, it is possible to set it to the last point
	//! \return true on success, false on failure
	bool SetOnAnchor(int anchorIndex)
	{
		if (anchorIndex < 0 || anchorIndex >= m_iAnchorPointsCount)
			return false;

		m_vCurrentPosition = m_aAnchorPoints[anchorIndex];

		vector prevPoint;
		vector nextPoint;
		if (anchorIndex == 0)									// start
		{
			m_vCurrentDirection = vector.Direction(m_aSegments[0][0], m_aSegments[0][1]).Normalized();
		}
		else if (anchorIndex == m_iAnchorPointsCount - 1)		// end
		{
			m_vCurrentDirection = vector.Direction(m_aSegments[m_iSegmentsCount - 1][0], m_aSegments[m_iSegmentsCount - 1][1]).Normalized();
		}
		else													// middle
		{
			// just a direction of prevNorm to nextNorm, not accurate - TODO: fix
			m_vCurrentDirection = vector.Direction(
				vector.Direction(m_vCurrentPosition, m_aAnchorPoints[anchorIndex - 1]).Normalized(),
				vector.Direction(m_vCurrentPosition, m_aAnchorPoints[anchorIndex + 1]).Normalized()
			).Normalized();
		}

		m_iCurrentSectionIndex = anchorIndex;
		m_iCurrentSectionPointIndex = 0;

		m_iCurrentSegmentIndex = CalculateCurrentSegmentIndex();
		m_fDoneLength = CalculateDoneLength();

		return true;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Sets to the provided position IF on the shape
//	//! \param[in] position the wanted (relative) position
//	//! \return true on success, false on failure
//	bool SetPos(vector position)
//	{
//		foreach (int segmentIndex, array<vector> segment : m_aSegments)
//		{
//			if (
//				(segmentIndex == m_iSegmentsCount - 1 || position != segment[1]) &&				// on start point, not on end
//				Math3D.PointLineSegmentDistanceSqr(position, segment[0], segment[1]) < 0.001)	// on segment
//			{
//				m_vCurrentPosition = position;
//				m_iCurrentSegmentIndex = segmentIndex;
//				return true;
//			}
//		}
//
//		return false;
//	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the provided shape is valid (more than one point and with some length), false otherwise
	bool IsValid()
	{
		return m_iAnchorPointsCount > 1 && m_fTotalLength > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset to the beginning
	void Reset()
	{
		if (m_iAnchorPointsCount > 0)
			SetOnAnchor(0);
	}

	//--- constructors

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] anchorPoints shape's anchor points
	//! \param[in] tesselatedPoints shape's tesselated points
	//! \return a new instance or null on error (not enough points, anchor points not present in tesselated points, etc)
	static SCR_ShapeNextPointHelper CreateFromPoints(notnull array<vector> anchorPoints, notnull array<vector> tesselatedPoints)
	{
		int anchorPointsCount = anchorPoints.Count();
		if (anchorPointsCount < 2)
			return null;

		int tesselatedPointsCount = tesselatedPoints.Count();
		if (tesselatedPointsCount < anchorPointsCount)
			return null;

		// must have same first and last points
		if (anchorPoints[0] != tesselatedPoints[0])
			return null;

//		if (anchorPoints[anchorPointsCount - 1] != tesselatedPoints[tesselatedPointsCount - 1])
//			return null;

		int anchorIndex;
		vector anchorPos = anchorPoints[anchorIndex];
		foreach (vector tesselatedPoint : tesselatedPoints)
		{
			if (anchorIndex >= anchorPointsCount) // found too many matches
				break;	// wrong detection

			if (tesselatedPoint == anchorPoints[anchorIndex])
			{
				++anchorIndex; // must break after the first point
				if (anchorIndex < anchorPointsCount)
					anchorPos = anchorPoints[anchorIndex];
			}
		}

		if (anchorIndex != anchorPointsCount) // did not find all points
			return null;

		// everything OK? let's go

		SCR_ShapeNextPointHelper result = new SCR_ShapeNextPointHelper();

		result.m_aAnchorPoints = SCR_ArrayHelperT<vector>.GetCopy(anchorPoints);
		result.m_aAnchorPointIndices = {};
		result.m_aTesselatedPoints = SCR_ArrayHelperT<vector>.GetCopy(tesselatedPoints);
		result.m_aSections = {};
//		m_aSectionLengths = {};
		array<vector> anchorSection;
		array<float> sectionSegmentLengths;
		result.m_aSegments = {};
		anchorIndex = 0;
		vector nextAnchor = result.m_aAnchorPoints[anchorIndex];

		if (anchorPointsCount > 0)
			result.m_vCurrentPosition = result.m_aAnchorPoints[0];

		vector prevPoint;
		foreach (int i, vector currPoint : result.m_aTesselatedPoints)
		{
			float segmentLength = vector.Distance(currPoint, prevPoint);

			if (i > 0)										// normal point or anchor point
			{
				anchorSection.Insert(currPoint);
				sectionSegmentLengths.Insert(segmentLength);
				result.m_aSegments.Insert({ prevPoint, currPoint });
			}

			if (currPoint == nextAnchor)					// anchor point (including first and last ones)
			{
				result.m_aAnchorPointIndices.Insert(i);
				if (i > 0)									// everything but the first point
				{
					result.m_aSections.Insert(anchorSection);
					sectionSegmentLengths.Insert(segmentLength);
//					m_aSectionLengths.Insert(sectionSegmentLengths);
				}

				anchorSection = { currPoint };
				sectionSegmentLengths = {};

				++anchorIndex; // if last point, foreach ends and m_aAnchorPoints[maxIndex + 1] never occurs
				if (anchorIndex >= anchorPointsCount)
					anchorIndex = 0;

				nextAnchor = result.m_aAnchorPoints[anchorIndex];
			}

			result.m_fTotalLength += segmentLength;

			prevPoint = currPoint;
		}

		result.m_iAnchorPointsCount = anchorPointsCount;
//		result.m_iTesselatedPointsCount = tesselatedPointsCount;
		result.m_iSegmentsCount = tesselatedPointsCount - 1;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] shapeEntity
	//! \param[in] reverse invert shape direction (from end to start instead of from start to end)
	//! \return
	static SCR_ShapeNextPointHelper CreateFromShape(notnull ShapeEntity shapeEntity, bool reverse = false)
	{
		array<vector> anchorPoints = {};
		array<vector> tesselatedPoints = {};

		shapeEntity.GetPointsPositions(anchorPoints);
		shapeEntity.GenerateTesselatedShape(tesselatedPoints);

		bool isShapeClosed = shapeEntity.IsClosed();

#ifdef WORKBENCH
		WorldEditorAPI worldEditorAPI = ((WorldEditor)Workbench.GetModule(WorldEditor)).GetApi();
		if (worldEditorAPI)
		{
			IEntitySource shapeEntitySource = worldEditorAPI.EntityToSource(shapeEntity);
			if (shapeEntitySource)
				shapeEntitySource.Get("IsClosed", isShapeClosed);
		}
#endif // WORKBENCH

		if (isShapeClosed)
		{
			anchorPoints.Insert(anchorPoints[0]);
			if (!SplineShapeEntity.Cast(shapeEntity)) // spline already has the first tesselated point as last point
				tesselatedPoints.Insert(tesselatedPoints[0]);
		}

		if (reverse)
		{
			SCR_ArrayHelperT<vector>.Reverse(anchorPoints);
			SCR_ArrayHelperT<vector>.Reverse(tesselatedPoints);
		}

		return CreateFromPoints(anchorPoints, tesselatedPoints);
	}

	//------------------------------------------------------------------------------------------------
	//! protected constructor - use CreateFrom* methods
	protected void SCR_ShapeNextPointHelper();
}
