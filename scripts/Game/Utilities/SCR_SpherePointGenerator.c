class SCR_SpherePointGenerator
{
	const float GOLDEN_RATIO = 1.618033988749895; // limited to 16 places, rounded away from zero
	const float INV_GOLDEN_RADIO_RAD = 3.883222077450933; // 2 * Pi / GOLDEN_RATIO limited to 16 places, rounded away from zero
	const float PI4 = 12.56637061435917; // 4 * Pi limited to 16 places, rounded away from zero
	const float POINT_DIST_SPHERE_EXP = 2.231; // Significantly improves accuracy of spacing predictions. This number was manually goal-searched by optimising for Root Mean Square Error on GetPointOnUnitSphere's results and predictions that use this exponent.
	const float INV_POINT_DIST_SPHERE_EXP = 0.4482294935006723; // 1 / 2.231 limited to 16 places, rounded away from zero

	//------------------------------------------------------------------------------------------------
	//! Generates almost uniformly spaced points on a unit sphere using a standard Fibonacci lattice.
	//! Odd and Even numbers alternate between above and bellow the equator.
	//! \param[in] amount Amount represents the total number of points on the unit sphere.
	//! \param[in] index Index is used to fetch a specific point on the sphere, it must be smaller than amount.
	//! \return a point on unit sphere from equator based on given amount and index, with centered index calculation. Vector.Zero is returned for invalid parameters.
	static vector GetPointOnUnitSphereFromEquator(int amount, int index)
	{
		int isIndexOdd = 1 & index;
		int centerOffset = index >> 1;
		if (isIndexOdd)
			centerOffset = -(centerOffset + 1);
		int centeredIndex = ((int)(0.5 * amount)) + centerOffset;
		return GetPointOnUnitSphere(amount, centeredIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates almost uniformly spaced points on a unit sphere using a standard Fibonacci lattice.
	//! \param[in] amount Amount represents the total number of points on the unit sphere.
	//! \param[in] index Index is used to fetch a specific point on the sphere, it must be smaller than amount.
	//! \return a point on unit sphere based on given index and amount. Vector.Zero is returned for invalid parameters.
	static vector GetPointOnUnitSphere(int amount, int index)
	{
		if (amount < 1)
			return vector.Zero;

		float theta = index * INV_GOLDEN_RADIO_RAD;
		float phi = Math.Acos(1 - 2 * index / amount); // Math.Acos already limits the domain to [-1, 1], which is acceptable. No additional bounds checking is needed.
		return { Math.Cos(theta) * Math.Sin(phi), Math.Cos(phi), Math.Sin(theta) * Math.Sin(phi) };
	}

	//------------------------------------------------------------------------------------------------
	//! Estimates points on unit sphere from straight-line spacing.
	//! \param[in] spacing Spacing represents the desired distance between points on a unit sphere.
	//! \return Estimates the number of points required on a unit sphere to achieve a given spacing between points. Returns float.INFINITY for zero spacing.
	static int EstimatePointsFromSpacingOnUnitSphere(float spacing)
	{
		if (float.AlmostEqual(spacing, 0))
			return float.INFINITY;

		float areaPerPoint = Math.Pow(spacing, POINT_DIST_SPHERE_EXP);
		float pointsEst = PI4 / areaPerPoint;
		return Math.Ceil(pointsEst);
	}

	//------------------------------------------------------------------------------------------------
	//! Estimates straight-line spacing between points on a unit sphere.
	//! \param[in] amount Amount represents the number of points on a unit sphere.
	//! \return Represents the estimated straight-line spacing between points on a unit sphere. Returns -1 for undefined case when amount is 0 or negative.
	static float EstimateSpacingFromPointsOnUnitSphere(int amount)
	{
		if (amount < 1)
			return -1;

		float areaPerPoint = PI4 / amount;
		return Math.Pow(areaPerPoint, INV_POINT_DIST_SPHERE_EXP);
	}
}
