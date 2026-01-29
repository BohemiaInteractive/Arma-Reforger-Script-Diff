//! SCR_Math2D Class
//!
//! Contains various scripted 2D math functions
class SCR_Math2D
{
	//------------------------------------------------------------------------------------------------
	//! Get 2D points array from vector array, format { x1,y1, x2,y2, x3,y3 }
	//! \param[in] points3D
	//! \param[out] points2D
	static void Get2DPolygon(notnull array<vector> points3D, out notnull array<float> points2D)
	{
		points2D.Clear();
		if (points3D.IsEmpty())
			return;

		foreach (vector point3D : points3D)
		{
			points2D.Insert(point3D[0]);
			points2D.Insert(point3D[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get an array of vectors from 2D array with 0 for Y value
	//! \param[in] points2D
	//! \param[out] points3D
	//! \return array of vector where Y is set to zero, e.g { x0, y0, x1, y1 } = { { x0, 0, y0 }, { x1, 0, y1 } }
	// unused
	static void Get3DPolygon(notnull array<float> points2D, out notnull array<vector> points3D)
	{
		int count = points2D.Count();
		if (count & 1)
			return;

		points3D.Clear();

		for (int i; i < count; i += 2)
		{
			points3D.Insert({ points2D[i], 0, points2D[i + 1] });
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] polygon
	//! \param[out] minX
	//! \param[out] maxX
	//! \param[out] minY
	//! \param[out] maxY
	//! \return true on success, otherwise false (e.g invalid polygon)
	static bool GetMinMaxPolygon(notnull array<float> polygon, out float minX, out float maxX, out float minY, out float maxY)
	{
		if (!IsPolygonValid(polygon))
			return false;

		minX = polygon[0];
		maxX = polygon[0];
		minY = polygon[1];
		maxY = polygon[1];
		float x, y;

		for (int i = 2, count = polygon.Count(); i < count; i += 2)
		{
			x = polygon[i];
			y = polygon[i + 1];

			if (x < minX)
				minX = x;
			else if (x > maxX)
				maxX = x;

			if (y < minY)
				minY = y;
			else if (y > maxY)
				maxY = y;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get provided polygon's area
	//! \param[in] polygon
	//! \return -1 on invalid polygon, otherwise the polygon's surface
	static float GetPolygonArea(notnull array<float> polygon)
	{
		if (!IsPolygonValid(polygon))
			return -1;

		float result;

		int j;
		for (int i = 0, count = polygon.Count(); i < count; i += 2) // step 2
		{
			j = (i + 2) % count;
			result += 0.5 * (polygon[i] * polygon[j + 1] - polygon[j] * polygon[i + 1]);
		}

		if (result < 0)
			result = -result;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// TODO: better
	// use SCR_Math.GetMathRandomGenerator().GenerateRandomPoint()?
	//! Get a random point within the provided 2D polygon
	//! \param[in] polygon
	//! \param[out] x found point's x coordinates
	//! \param[out] y found point's x coordinates
	//! \return true on success, false otherwise
	// unused
	static bool GetRandomPointInPolygon(notnull array<float> polygon, out float x, out float y)
	{
		float minX, minY, maxX, maxY;
		if (!GetMinMaxPolygon(polygon, minX, maxX, minY, maxY))
			return false;

		GetRandomPointInRectangle(minX, maxX, minY, maxY, x, y);
		while (!Math2D.IsPointInPolygon(polygon, x, y)) // ugh
		{
			GetRandomPointInRectangle(minX, maxX, minY, maxY, x, y);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a random point in the provided rectangle
	//! \param[in] minX
	//! \param[in] maxX
	//! \param[in] minY
	//! \param[in] maxY
	//! \param[out] x
	//! \param[out] y
	//! \return
	// only used by GetRandomPointInPolygon
	static bool GetRandomPointInRectangle(float minX, float maxX, float minY, float maxY, out float x, out float y)
	{
		x = Math.RandomFloat(minX, maxX);
		y = Math.RandomFloat(minY, maxY);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a random point in the provided circular sector - uniform distribution
	//! \param[in] originX
	//! \param[in] originY
	//! \param[in] angleFrom angle in radians from which we calculate random angle
	//! \param[in] angleTo angle in radians to which we calculate random angle
	//! \param[in] radius
	//! \param[out] x
	//! \param[out] y
	//! \return
	static bool GetRandomPointInSector(float originX, float originY, float angleFrom, float angleTo, float radius, out float x, out float y)
	{
		float distance = radius * Math.Sqrt(Math.RandomFloat01()); // to have it uniformly distributed
		float angle = Math.RandomFloat(angleFrom, angleTo);
		x = originX + distance * Math.Cos(angle);
		y = originY + distance * Math.Sin(angle);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks the amount of points for a 2D polygon (needs 3 points to make a polygon and an even amount of values to have valid points)
	//! \param[in] polygon format x0, y0, x1, y1 etc
	//! \return true if the polygon has at least three valid points, false otherwise (two points only, or odd number of values)
	static bool IsPolygonValid(notnull array<float> polygon)
	{
		int count = polygon.Count();

		if (count < 6) // less than 3 points? not a polygon
			return false;

		if (count & 1) // odd number = one missing/extra point
			return false;

		return true;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Calculates squared distance of `point` to a line segment given by points `v0` and `v1`.
//	// unused
//	static float GetPointLineSegmentDistanceSqr(float pX, float pY, float x0, float y0, float x1, float y1)
//	{
//		return Math3D.PointLineSegmentDistanceSqr({ pX, 0, pY }, { x0, 0, y0 }, {x1, 0, y1 });
//	}

//	//------------------------------------------------------------------------------------------------
//	//! Calculates distance of `point` to a line segment given by points `v0` and `v1`.
//	static float GetPointLineSegmentDistance(float pX, float pY, float x0, float y0, float x1, float y1)
//	{
//		return Math3D.PointLineSegmentDistance({ pX, 0, pY }, { x0, 0, y0 }, { x1, 0, y1 });
//		// return Math.Sqrt(PointLineSegmentDistance(pX, pY, x0, y0, x1, y1));
//	}

	//------------------------------------------------------------------------------------------------
	//! Translates cartesian coordinates (x,y) into polar (angle and radius)
	//! \param[in] x
	//! \param[in] y
	//! \param[out] angle in radian
	//! \param[out] radius
	//! \return true
	protected static bool CartesianToPolar(float x, float y, out float angle, out float radius)
	{
		angle = Math.Atan2(y, x);
		radius = Math.Sqrt(x + y);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Translates polar coordinates (angle and radius) into cartesian (x,y)
	//! \param[in] angle in radians
	//! \param[in] radius
	//! \param[out] x
	//! \param[out] y
	//! \return true
	static bool PolarToCartesian(float angle, float radius, out float x, out float y)
	{
		x = Math.Cos(angle) * radius;
		y = Math.Sin(angle) * radius;
		return true;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Get a clockwise degree angle value from counter-clockwise radians
//	//! \param[in] value angle in counter-clockwise radians
//	//! \return [0..360[ clockwise value
//	protected static float TrigoRadianToDegree(float value)
//	{
//		value = 90 - value * Math.RAD2DEG;
//
//		if (value < 0 || value > 360)
//			value = Math.Repeat(value, 360);
//
//		if (value < 0)
//			value += 360;
//
//		if (float.AlmostEqual(value, 360))
//			value = 0;
//
//		return value;
//	}

	//------------------------------------------------------------------------------------------------
	//! Get a counter-clockwise radian angle value from clockwise angles
	//! \param[in] value angle in clockwise degrees
	//! \return [0..Math.PI2[ counter-clockwise value
	static float DegreeToTrigoRadian(float value)
	{
		value = Math.PI_HALF - value * Math.DEG2RAD;

		if (value < 0 || value > Math.PI2)
			value = Math.Repeat(value, Math.PI2);

		if (value < 0)
			value += Math.PI2;

		if (float.AlmostEqual(value, Math.PI2))
			value = 0;

		return value;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Get the difference between two degree angles
//	//! \code
//	//! GetDegreeAngleDifference(0, 45)		// returns 45
//	//! GetDegreeAngleDifference(0, 315)	// returns -45
//	//! \code
//	//! \param[in] angleA
//	//! \param[in] angleB
//	//! \return angle difference in range ]-180..+180]
//	protected static float GetDegreeAngleDifference(float angleA, float angleB)
//	{
//		if (angleA <= -180 || angleA > 180)
//			angleA = Math.Repeat(angleA, 360);
//
//		if (angleB <= -180 || angleB > 180)
//			angleB = Math.Repeat(angleB, 360);
//
//		angleA = angleB - angleA; // variable reuse
//
//		if (angleA <= -180)
//			angleA += 360;
//		else if (angleA > 180)
//			angleA -= 360;
//
//		return angleA;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! Get the difference between two radian angles
//	//! \code
//	//! GetRadianAngleDifference(0, Math.PI)		// returns Math.PI
//	//! GetRadianAngleDifference(0, Math.PI * 1.5)	// returns -Math.PI_HALF
//	//! \code
//	//! \param[in] angleA
//	//! \param[in] angleB
//	//! \return angle difference in range ]-Math.PI..+Math.PI]
//	protected static float GetRadianAngleDifference(float angleA, float angleB)
//	{
//		if (angleA <= -Math.PI || angleA > Math.PI)
//			angleA = Math.Repeat(angleA, Math.PI2);
//
//		if (angleB <= -Math.PI || angleB > Math.PI)
//			angleB = Math.Repeat(angleB, Math.PI2);
//
//		angleA = angleB - angleA; // variable reuse
//
//		if (angleA <= -Math.PI)
//			angleA += Math.PI2;
//		else if (angleA > Math.PI)
//			angleA -= Math.PI2;
//
//		return angleA;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! in range [0..Math.PI2[
//	//! \param[in] from
//	//! \param[in] to
//	//! \return radian angle
//	protected static float GetRadianAngle(vector from, vector to)
//	{
//		to = to - from; // variable reuse
//		return Math.Atan2(to[2], to[0]);
//	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] x0
	//! \param[in] y0
	//! \param[in] angleRad0 counter-clockwise radians, range does not matter (no need for [0..Math.PI2[)
	//! \param[in] x1
	//! \param[in] y1
	//! \param[in] angleRad1 counter-clockwise radians, range does not matter (no need for [0..Math.PI2[)
	//! \param[out] x
	//! \param[out] y
	//! \return true on intersection, false otherwise
	static bool GetLinesIntersectionXZ(float x0, float y0, float angleRad0, float x1, float y1, float angleRad1, out float x, out float y)
	{
		if (angleRad0 < 0 || angleRad0 > Math.PI2)
			angleRad0 = Math.Repeat(angleRad0, Math.PI2);

		if (angleRad1 < 0 || angleRad1 > Math.PI2)
			angleRad1 = Math.Repeat(angleRad1, Math.PI2);

		// below is from https://www.geeksforgeeks.org/program-for-point-of-intersection-of-two-lines/
		float a1 = Math.Sin(angleRad0);
		float b1 = -Math.Cos(angleRad0);
		float c1 = a1 * x0 + b1 * y0;

		// Line CD represented as a2x + b2y = c2
		float a2 = Math.Sin(angleRad1);
		float b2 = -Math.Cos(angleRad1);
		float c2 = a2 * x1 + b2 * y1;

		float determinant = a1 * b2 - a2 * b1;

		// lines are parallel
		if (float.AlmostEqual(determinant, 0))
			return false;

		x = (b2 * c1 - b1 * c2) / determinant;
		y = (a1 * c2 - a2 * c1) / determinant;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates random point in given polygon
	//! \param[in] polygon Consecutive floats give polygon in 2D (2 floats = Vector2)
	//! \param[in] bbMin Bounding box minimum corner
	//! \param[in] bbMax Bounding box maximum corner
	//! \return Vector3 point in polygon
	// unused
	static vector GenerateRandomPoint(array<float> polygon, vector bbMin, vector bbMax)
	{
		return SCR_Math.GetMathRandomGenerator().GenerateRandomPoint(polygon, bbMin, bbMax);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates a random point around `center` in range min/max radius
	//! \param[in] minRadius All generated points will be at least this far from center
	//! \param[in] maxRadius All generated points will be at most this far from center
	//! \param[in] center Position around which to generate. Vector2 XZ
	//! \param[in] uniform If false, has a small bias towards the center which may be desirable in some situations
	//! \return Vector2 XZ set, Y = 0
	// unused
	static vector GenerateRandomPointInRadius(float minRadius, float maxRadius, vector center, bool uniform = true)
	{
		return SCR_Math.GetMathRandomGenerator().GenerateRandomPointInRadius(minRadius, maxRadius, center, uniform);
	}
}
