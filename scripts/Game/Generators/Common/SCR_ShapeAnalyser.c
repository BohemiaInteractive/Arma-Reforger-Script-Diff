//! The Shape Analyser provides "points" (Rays/Transforms) with absolute position and vectorDir (vectorUp is not filled)
//! if the shape is closed, the first point is added as the last point FOR THE POLYLINE ONLY (the engine already does it for the spline)
class SCR_ShapeAnalyser
{
	protected bool m_bIsSpline;
	protected bool m_bIsClosed;

	protected ref array<ref SCR_Ray> m_aAbsoluteAnchorRays;				//!< points as shown by the Vector tool
	protected ref array<ref SCR_Ray> m_aAbsoluteMidPointRays;			//!< tesselated points between two "normal" points, regardless of distance (by num of tesselated points)
	protected ref array<ref SCR_Ray> m_aAbsoluteTesselatedPointRays;	//!< intermediate points
	protected ref array<vector> m_aRelativeAnchorPoints;
	protected ref array<vector> m_aAbsoluteAnchorPoints;
	protected ref array<vector> m_aRelativeTesselatedPoints;
	protected ref array<vector> m_aAbsoluteTesselatedPoints;

	// stats
	protected float m_fLength2D;
	protected float m_fLength3D;
	protected float m_fSurface;
	protected float m_fMinAltitude, m_fMaxAltitude;
	protected float m_fMinSlope, m_fMaxSlope; //!< in radians

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsClosed()
	{
		return m_bIsClosed;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsSpline()
	{
		return m_bIsSpline;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_Ray> GetAbsoluteAnchorPointRays()
	{
		array<ref SCR_Ray> result = {};
		foreach (SCR_Ray point : m_aAbsoluteAnchorRays)
		{
			result.Insert(point);
		}
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return absolute anchor points positions
	array<vector> GetAbsoluteAnchorPoints()
	{
		array<vector> result = {};
		result.Copy(m_aAbsoluteAnchorPoints);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return relative anchor points positions
	array<vector> GetRelativeAnchorPoints()
	{
		array<vector> result = {};
		result.Copy(m_aRelativeAnchorPoints);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return absolute tesselated points positions
	array<vector> GetAbsoluteTesselatedPoints()
	{
		array<vector> result = {};
		result.Copy(m_aAbsoluteTesselatedPoints);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return relative tesselated points positions
	array<vector> GetRelativeTesselatedPoints()
	{
		array<vector> result = {};
		result.Copy(m_aRelativeTesselatedPoints);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_Ray> GetAbsoluteMiddlePointRays()
	{
		return SCR_ArrayHelperRefT<SCR_Ray>.GetCopy(m_aAbsoluteMidPointRays);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<ref SCR_Ray> GetAbsoluteTesselatedPointRays()
	{
		return SCR_ArrayHelperRefT<SCR_Ray>.GetCopy(m_aAbsoluteTesselatedPointRays);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return length2D, length3D, surface (-1 if shape is not closed), minAltitude, maxAltitude, minSlope, maxSlope
	array<float> GetStats()
	{
		return { m_fLength2D, m_fLength3D, m_fSurface, m_fMinAltitude, m_fMaxAltitude, m_fMinSlope, m_fMaxSlope };
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] shapeEntity
	static SCR_ShapeAnalyser CreateFromShape(notnull ShapeEntity shapeEntity)
	{
		SCR_ShapeAnalyser result = new SCR_ShapeAnalyser();
		result.m_bIsSpline = SplineShapeEntity.Cast(shapeEntity) != null;
		result.m_bIsClosed = shapeEntity.IsClosed();

		result.m_aRelativeAnchorPoints = {};
		shapeEntity.GetPointsPositions(result.m_aRelativeAnchorPoints);

		int pointsCount = result.m_aRelativeAnchorPoints.Count();
		if (pointsCount < 2)
			return null;

		result.m_aAbsoluteAnchorPoints = {};
		foreach (vector relativeAnchorPoint : result.m_aRelativeAnchorPoints)
		{
			result.m_aAbsoluteAnchorPoints.Insert(shapeEntity.CoordToParent(relativeAnchorPoint));
		}
		result.m_aRelativeTesselatedPoints = {};
		shapeEntity.GenerateTesselatedShape(result.m_aRelativeTesselatedPoints);

		result.m_aAbsoluteTesselatedPoints = {};
		result.m_aAbsoluteTesselatedPoints.Reserve(result.m_aRelativeTesselatedPoints.Count());
		foreach (int i, vector relPoint : result.m_aRelativeTesselatedPoints)
		{
			result.m_aAbsoluteTesselatedPoints.Insert(shapeEntity.CoordToParent(relPoint));
		}

		result.m_aAbsoluteAnchorRays = {};
		result.m_aAbsoluteTesselatedPointRays = {};

		vector diff;
		vector prevTessPoint;
		int anchorIndex;
		vector currAnchorPoint = result.m_aAbsoluteAnchorPoints[anchorIndex];

		array<int> pointTesselatedIndices = {};

		// stats
		array<float> polygon2D = {};
		result.m_fMinAltitude = currAnchorPoint[1];
		result.m_fMaxAltitude = result.m_fMinAltitude;
		result.m_fMinSlope = float.INFINITY;
		result.m_fMaxSlope = -float.INFINITY;

		int tessCount = result.m_aAbsoluteTesselatedPoints.Count();
		SCR_Ray ray;
		foreach (int i, vector currTessPoint : result.m_aAbsoluteTesselatedPoints)
		{
			if (i == 0)
				diff = vector.Direction(currTessPoint, result.m_aAbsoluteTesselatedPoints[1]);
			else
				diff = vector.Direction(prevTessPoint, currTessPoint);

			ray = new SCR_Ray();
			ray.m_vPosition = currTessPoint; // absolute (world) position

			result.m_aAbsoluteAnchorPoints.Insert(ray.m_vPosition);

			float slopeRad = Math.Atan2(diff[1], vector.DistanceXZ(diff, vector.Zero));

			if (i == 0)						// first
				ray.m_vDirection = vector.Direction(currTessPoint, result.m_aAbsoluteTesselatedPoints[i + 1]).Normalized();
			else if (i < tessCount - 1)		// mid-curve - averages previous and next vector
				ray.m_vDirection = vector.Direction(
					vector.Direction(currTessPoint, prevTessPoint).Normalized(),
					vector.Direction(currTessPoint, result.m_aAbsoluteTesselatedPoints[i + 1]).Normalized()
				).Normalized();
			else							// last
				ray.m_vDirection = vector.Direction(prevTessPoint, currTessPoint).Normalized();

			result.m_aAbsoluteTesselatedPointRays.Insert(ray);

			if (currTessPoint == currAnchorPoint)
			{
				pointTesselatedIndices.Insert(i);
				result.m_aAbsoluteAnchorRays.Insert(ray);

				anchorIndex++;
				if (anchorIndex >= pointsCount)
					anchorIndex = 0;

				currAnchorPoint = result.m_aAbsoluteAnchorPoints[anchorIndex];
			}

			// stats
			if (prevTessPoint)
			{
				result.m_fLength2D += vector.DistanceXZ(prevTessPoint, currTessPoint);
				result.m_fLength3D += vector.Distance(prevTessPoint, currTessPoint);
			}

			if (ray.m_vPosition[1] < result.m_fMinAltitude)
				result.m_fMinAltitude = ray.m_vPosition[1];

			if (ray.m_vPosition[1] > result.m_fMaxAltitude)
				result.m_fMaxAltitude = ray.m_vPosition[1];

			if (slopeRad < result.m_fMinSlope)
				result.m_fMinSlope = slopeRad;

			if (slopeRad > result.m_fMaxSlope)
				result.m_fMaxSlope = slopeRad;

			polygon2D.Insert(ray.m_vPosition[0]);
			polygon2D.Insert(ray.m_vPosition[2]);

			// loop
			prevTessPoint = currTessPoint;
		}

		result.m_aAbsoluteMidPointRays = {};
		if (result.m_bIsSpline) // take the middle point
		{
			int prevIndex = pointTesselatedIndices[0];
			int nextIndex;
			for (int i = 1, count = pointTesselatedIndices.Count(); i < count; i++)
			{
				nextIndex = pointTesselatedIndices[i];
				result.m_aAbsoluteMidPointRays.Insert(result.m_aAbsoluteTesselatedPointRays[prevIndex + 0.5 * (nextIndex - prevIndex)]);
				prevIndex = nextIndex;
			}
		}
		else
		{
			SCR_Ray currRay = result.m_aAbsoluteAnchorRays[0];
			SCR_Ray nextRay;
			for (int i = 1, count = result.m_aAbsoluteAnchorRays.Count(); i < count; i++) // start from 1
			{
				nextRay = result.m_aAbsoluteAnchorRays[i];
				result.m_aAbsoluteMidPointRays.Insert(SCR_Ray.Lerp(currRay, nextRay, 0.5));
				currRay = nextRay;
			}

			if (result.m_bIsClosed)
				result.m_aAbsoluteMidPointRays.Insert(SCR_Ray.Lerp(currRay, result.m_aAbsoluteAnchorRays[0], 0.5));
		}

		result.m_fSurface = SCR_Math2D.GetPolygonArea(polygon2D); // closed or not

		return result;
	}

	//---
	// constructor
	protected void SCR_ShapeAnalyser();
}
