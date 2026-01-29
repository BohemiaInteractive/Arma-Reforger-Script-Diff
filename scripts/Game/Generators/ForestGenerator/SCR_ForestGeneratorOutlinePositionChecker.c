class SCR_ForestGeneratorOutlinePositionChecker
{
	protected ref array<ref array<vector>> m_aOutlineSegments = {};	//<! points are 2D, format { X, 0, Z }
//	protected ref array<vector> m_aOutlineCentres = {};
//	protected ref array<float> m_aOutlineRadii = {}; // are you radii?
	protected float m_fSetDistanceSq;

	//------------------------------------------------------------------------------------------------
	//! \param[in] pos the position to check
	//! \param[in] distance the checked distance
	//! \return true if provided pos is <= distance from diff outline, false otherwise
	bool IsPosDistanceEqualOrCloserThan(vector pos, float distance)
	{
		pos[1] = 0; // 2D conversion

		distance *= distance;
		foreach (int i, array<vector> segment : m_aOutlineSegments)
		{
			if (Math3D.PointLineSegmentDistanceSqr(pos, segment[0], segment[1]) <= distance)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pos the position to check
	//! \return true if provided pos is <= distance from diff outline, false otherwise
	bool IsPosWithinSetDistance(vector pos)
	{
		pos[1] = 0; // 2D conversion

		foreach (int i, array<vector> segment : m_aOutlineSegments)
		{
			if (Math3D.PointLineSegmentDistanceSqr(pos, segment[0], segment[1]) <= m_fSetDistanceSq)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return two arrays of outline difference "lines" (four floats, x0y0-x1y1) around which the area must be cleared - old and new shape
	protected static array<float> GetOutlineDiffLines(notnull array<float> oldPoints, notnull array<float> newPoints)
	{
		// points can only be added one by one
		// points can be deleted in group and non-sequentially (e.g delete point 1-5-6 out of 10)
		// points can be moved in group and non-sequentially

		int oldCount = oldPoints.Count();
		int newCount = newPoints.Count();

		if (newCount > oldCount)
			return GetOutlineDiffLines_PointAdded(oldPoints, newPoints);

		if (newCount < oldCount)
			return GetOutlineDiffLines_PointsRemoved(oldPoints, newPoints);

		return GetOutlineDiffLines_PointsMoved(oldPoints, newPoints);
	}

	//------------------------------------------------------------------------------------------------
	// wanted result:
	// e.g points @ index 0, 2 and 3 are removed
	// old: { x0, y0, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, x6, y6 }
	// new: { x1, y1, x4, y4, x5, y5, x6, y6 }
	// returns:
	// result[0] { m1-p0-p1, m1-p2-p1, m1-p3-p1 }
	// result[1] { m1-p1-p1, m1-p4-p1, m1-p6-p1 (because p0 removed) }
	protected static array<float> GetOutlineDiffLines_PointsRemoved(notnull array<float> oldPoints, notnull array<float> newPoints)
	{
		int oldCount = oldPoints.Count();
		int newCount = newPoints.Count();

		if (oldCount < 3)
			return {};

		array<int> removedIndices = {};
		array<int> relatedIndices = {};

		// first, get removed indices and related new indices
		int indexOld;
		for (int i; i < newCount; i += 2) // step 2
		{
			bool isRelated = false;

			for (; indexOld < oldCount; indexOld += 2)
			{
				if (newPoints[i] != oldPoints[indexOld] || newPoints[i + 1] != oldPoints[indexOld + 1])
				{
					removedIndices.Insert(indexOld * 0.5);
					isRelated = true;
				}
				else
				{
					indexOld += 2;
					break;
				}
			}

			if (isRelated)
				relatedIndices.Insert(i * 0.5);
		}

		if (indexOld < oldCount) // if there was more, add last and first point (if not already added)
		{
			if (!relatedIndices.Contains(0))
				relatedIndices.Insert(0);

			if (!relatedIndices.Contains(newCount * 0.5 - 1))
				relatedIndices.Insert(newCount * 0.5 - 1);
		}

		array<float> tmp; // for ease of writing, otherwise linecount++++++

		array<float> result = {};
		foreach (int removedIndex : removedIndices)
		{
			int floatIndex = removedIndex * 2;
			if (floatIndex == 0)
			{
				tmp = {
					oldPoints[oldCount - 2], oldPoints[oldCount - 1], oldPoints[0], oldPoints[1],	// old-1 to oldPoint
					oldPoints[0], oldPoints[1], oldPoints[2], oldPoints[3],							// oldPoint to old+1
				};
			}
			else if (floatIndex == oldCount - 2) // -2
			{
				tmp = {
					oldPoints[oldCount - 4], oldPoints[oldCount - 3], oldPoints[oldCount - 2], oldPoints[oldCount - 1],	// old-1 to oldPoint
					oldPoints[oldCount - 2], oldPoints[oldCount - 1], oldPoints[0], oldPoints[1],						// oldPoint to old+1
				};
			}
			else
			{
				tmp = {
					oldPoints[floatIndex - 2], oldPoints[floatIndex - 1], oldPoints[floatIndex], oldPoints[floatIndex + 1],	// old-1 to oldPoint
					oldPoints[floatIndex], oldPoints[floatIndex + 1], oldPoints[floatIndex + 2], oldPoints[floatIndex + 3],	// oldPoint to old+1
				};
			}

			foreach (float value : tmp)
			{
				result.Insert(value);
			}
		}

		if (newCount > 2)
		{
			foreach (int relatedIndex : relatedIndices)
			{
				int floatIndex = relatedIndex * 2;
				if (floatIndex == 0)
				{
					tmp = {
						newPoints[newCount - 2], newPoints[newCount - 1], newPoints[0], newPoints[1],	// old-1 to oldPoint
						newPoints[0], newPoints[1], newPoints[2], newPoints[3],							// oldPoint to old+1
					};
				}
				else if (floatIndex == newCount - 2) // -2
				{
					tmp = {
						newPoints[newCount - 4], newPoints[newCount - 3], newPoints[newCount - 2], newPoints[newCount - 1],	// old-1 to oldPoint
						newPoints[newCount - 2], newPoints[newCount - 1], newPoints[0], newPoints[1],						// oldPoint to old+1
					};
				}
				else
				{
					tmp = {
						newPoints[floatIndex - 2], newPoints[floatIndex - 1], newPoints[floatIndex], newPoints[floatIndex + 1],	// old-1 to oldPoint
						newPoints[floatIndex], newPoints[floatIndex + 1], newPoints[floatIndex + 2], newPoints[floatIndex + 3],	// oldPoint to old+1
					};
				}

				foreach (float value : tmp)
				{
					result.Insert(value);
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return all p-1→vertice / vertice→p+1 (multiple of 4) for old and new points; cannot return null
	protected static array<float> GetOutlineDiffLines_PointsMoved(notnull array<float> oldPoints, notnull array<float> newPoints)
	{
		// count is identical here
		int count = oldPoints.Count();
		if (count < 3)
			return {};

		int lastIndex = count - 2;

		float prevOldPoint[2];
		float currOldPoint[2];
		float nextOldPoint[2];

		float prevNewPoint[2];
		float currNewPoint[2];
		float nextNewPoint[2];

		array<float> tmp; // for ease of writing, otherwise linecount++++++
		array<float> result = {};
		for (int i; i < count; i += 2) // step 2
		{
			if (oldPoints[i] != newPoints[i] || newPoints[i + 1] != newPoints[i + 1])
			{
				currOldPoint = { oldPoints[i], oldPoints[i + 1] };
				currNewPoint = { newPoints[i], newPoints[i + 1] };

				if (i == 0)				// first point
				{
					prevOldPoint = { oldPoints[lastIndex], oldPoints[lastIndex + 1] };
					nextOldPoint = { oldPoints[i + 2], oldPoints[i + 3] };

					prevNewPoint = { newPoints[lastIndex], newPoints[lastIndex + 1] };
					nextNewPoint = { newPoints[i + 2], newPoints[i + 3] };
				}
				else if (i == lastIndex)	// last point
				{
					prevOldPoint = { oldPoints[i - 2], oldPoints[i - 1] };
					nextOldPoint = { oldPoints[0], oldPoints[1] };

					prevNewPoint = { newPoints[i - 2], oldPoints[i - 1] };
					nextNewPoint = { newPoints[0], oldPoints[1] };
				}
				else						// normal point
				{
					prevOldPoint = { oldPoints[i - 2], oldPoints[i - 1] };
					nextOldPoint = { oldPoints[i + 2], oldPoints[i + 3] };

					prevNewPoint = { newPoints[i - 2], oldPoints[i - 1] };
					nextNewPoint = { newPoints[i + 2], newPoints[i + 3] };
				}

				tmp = {
					prevOldPoint[0], prevOldPoint[1], currOldPoint[0], currOldPoint[1], // old-1 to oldPoint
					nextOldPoint[0], nextOldPoint[1], currOldPoint[0], currOldPoint[1], // old+1 to oldPoint

					prevNewPoint[0], prevNewPoint[1], currNewPoint[0], currNewPoint[1], // new-1 to newPoint
					nextNewPoint[0], nextNewPoint[1], currNewPoint[0], currNewPoint[1], // new+1 to newPoint
				};

				foreach (float value : tmp)
				{
					result.Insert(value);
				}
/*
				Print("moved {" +
					currOldPoint[0] + ", " + currOldPoint[1] +
					"} to {" +
					currNewPoint[0] + ", " + currNewPoint[1] +
					"}");
*/
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return eight floats representing FOUR points (old-1 to old, old+1 to old, old-1 to new, old+1 to new
	protected static array<float> GetOutlineDiffLines_PointAdded(notnull array<float> oldPoints, notnull array<float> newPoints)
	{
		int newCount = newPoints.Count();
		if (newCount < 3)
			return {};

		int oldCount = oldPoints.Count();
		int index = -1;
		for (int i; i < oldCount; i += 2)	// step 2
		{
			if (oldPoints[i] != newPoints[i] || oldPoints[i + 1] != newPoints[i + 1])
			{
				index = i;
				break;
			}
		}

		bool isLastPoint = false;
		if (index == -1) // no difference found, it is then the last point
		{
			isLastPoint = true;
			index = oldCount + 2;
		}

		float newPoint[2] = { newPoints[index], newPoints[index + 1] };

		float prevPoint[2];
		float nextPoint[2];

		if (index == 0)			// first point - cannot happen in World Editor AFAIK
		{
			prevPoint = { oldPoints[oldCount - 2], oldPoints[oldCount - 1] };
			nextPoint = { oldPoints[index + 2], oldPoints[index + 3] };
		}
		else if (isLastPoint)		// last point
		{
			prevPoint = { newPoints[index - 2], oldPoints[index - 1] };
			nextPoint = { oldPoints[0], oldPoints[1] };
		}
		else						// normal point
		{
			prevPoint = { oldPoints[index - 2], oldPoints[index - 1] };
			nextPoint = { oldPoints[index + 2], oldPoints[index + 3] };
		}

		float oldPoint[2] = { // virtual middle of old segment
			nextPoint[0] + 0.5 * (nextPoint[0] - prevPoint[0]),
			nextPoint[1] + 0.5 * (nextPoint[1] - prevPoint[1]),
		};

		return {
			prevPoint[0], prevPoint[1], oldPoint[0], oldPoint[1],	// p-1 to oldPoint
			nextPoint[0], nextPoint[1], oldPoint[0], oldPoint[1],	// p+1 to oldPoint

			prevPoint[0], prevPoint[1], newPoint[0], newPoint[1],	// p-1 to newPoint
			nextPoint[0], nextPoint[1], newPoint[0], newPoint[1],	// p+1 to newPoint
		};
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] oldPoints
	//! \param[in] newPoints
	//! \param[in] setDistance
	void SCR_ForestGeneratorOutlinePositionChecker(notnull array<float> oldPoints, notnull array<float> newPoints, float setDistance)
	{
		array<float> allOutlinePoints = GetOutlineDiffLines(oldPoints, newPoints);

		array<vector> segment; // cannot use vector segment[2] due to { xxx[i] } below
		for (int i, count = allOutlinePoints.Count(); i < count; i += 4) // step 4
		{
			segment = {
				{ allOutlinePoints[i], 0, allOutlinePoints[i + 1] },
				{ allOutlinePoints[i + 2], 0, allOutlinePoints[i + 3] },
			};

			m_aOutlineSegments.Insert(segment);
//			m_aOutlineCentres.Insert(vector.Lerp(segment[0], segment[1], 0.5));
//			m_aOutlineRadii.Insert(vector.DistanceXZ(segment[0], segment[1]) * 0.5);
		}

		m_fSetDistanceSq = setDistance * setDistance;
	}
}
