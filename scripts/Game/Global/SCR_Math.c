class SCR_Math
{
	//~ Random generator used in scripted Math functions
	protected const static ref RandomGenerator RANDOM_GENERATOR = new RandomGenerator();

	static const float MILS_NATO2DEG	= 0.05625;
	static const float MILS_WP2DEG		= 0.06;
	static const float MILS_STRECK2DEG	= 0.05714;

	static const int MAX_RANDOM = 0x7FFF; // 32767

	//------------------------------------------------------------------------------------------------
	//! Returns a random `float` number between `min` [inclusive] and `max` [exclusive].
	//! Will run checks to make sure the values are valid for Math.RandomFloat, and fix them if not correct
	//! \param[in] min Minimum Value
	//! \param[in] max Maximum Value
	//! \return Random float value
	static float RandomFloat(float min, float max)
	{
		if (min == max)
			return min;
		else if (min < max)
			return Math.RandomFloat(min, max);
		else 
			return Math.RandomFloat(max, min);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a random `float` number between `min` [inclusive] and `max` [inclusive].
	//! Will run checks to make sure the values are valid for Math.RandomFloatInclusive, and fix them if not correct
	//! \param[in] min Minimum Value
	//! \param[in] max Maximum Value
	//! \return Random float value
	static float RandomFloatInclusive(float min, float max)
	{
		if (min == max)
			return min;
		else if (min < max)
			return Math.RandomFloatInclusive(min, max);
		else 
			return Math.RandomFloatInclusive(max, min);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns random number with Gauss/Normal distribution (https://en.wikipedia.org/wiki/Normal_distribution)
	//! \param[in] min included
	//! \param[in] mid can be outside min..max range
	//! \param[in] max included
	//! \return a float in min..max range
	static float RandomGaussFloat(float min, float mid, float max)
	{
		if (min == max)
			return min;

		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}

		float result = Math.RandomGaussFloat((max - min) / 6.0, mid); // ~99.73% cases covered

		if (result < min)
			return min;

		if (result > max)
			return max;

		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a random `int` number between `min` [inclusive] and `max` [exclusive].
	//! Will run checks to make sure the values are valid for Math.RandomInt, and fix them if not correct
	//! \param[in] min Minimum Value
	//! \param[in] max Maximum Value
	//! \return Random int value
	static int RandomInt(int min, int max)
	{
		if (min == max)
			return min;
		else if (min < max)
			return Math.RandomInt(min, max);
		else 
			return Math.RandomInt(max, min);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns a random `int` number between `min` [inclusive] and `max` [inclusive].
	//! Will run checks to make sure the values are valid for Math.RandomIntInclusive, and fix them if not correct
	//! \param[in] min Minimum Value
	//! \param[in] max Maximum Value
	//! \return Random int value
	static int RandomIntInclusive(int min, int max)
	{
		if (min == max)
			return min;
		else if (min < max)
			return Math.RandomIntInclusive(min, max);
		else 
			return Math.RandomIntInclusive(max, min);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes two floats and returns the remainder after division
	//! \param[in] dividend
	//! \param[in] divisor
	//! \return an always-positive modulo (unlike Math.Mod)
	// TODO: [Obsolete("Use Math.Mod instead")] ?
	//		beware, negative numbers return different results:
	//		Math.Mod(-500, 360)      = -140
	//		SCR_Math.fmod(-500, 360) =  220
	static float fmod(float dividend, float divisor)
	{
		if (divisor == 0)
			return 0;

		return dividend - Math.Floor(dividend / divisor) * divisor;
	}

	//------------------------------------------------------------------------------------------------
	//! Shortest linear interpolation between two angles.
	//! \param[in] a Start angle in degrees
	//! \param[in] b Target angle in degrees
	//! \param[in] time Progress in range [0,1]
	//! \return Interpolated angle
	static float LerpAngle(float a, float b, float t)
	{
		float dt = fmod(b - a, 360);
		if (dt > 180)
			dt -= 360;

		return fmod(Math.Lerp(a, a + dt, t), 360);
	}

	//------------------------------------------------------------------------------------------------
	//! Get shortest angle between two angles.
	//! \param[in] a Start angle in degrees
	//! \param[in] b Target angle in degrees
	//! \return Difference in degrees
	static float DeltaAngle(float a, float b)
	{
		return 180 - Math.AbsFloat(fmod(Math.AbsFloat(b - a), 360) - 180);
	}

	//------------------------------------------------------------------------------------------------
	//! Minimum mask that can cover a provided number
	//! \param[in] x Positive integer value
	//! \return Minimum mask
	static int IntegerMask(int x)
	{
		x |= (x >> 1);
		x |= (x >> 2);
		x |= (x >> 4);
		x |= (x >> 8);
		x |= (x >> 16);
		return x;
	}

	//------------------------------------------------------------------------------------------------
	//! Get distance to stop with given deceleration
	//! \param[in] speed
	//! \param[in] deceleration
	//! \return stop distance in metres
	static float GetDistanceToStop(float speed, float deceleration)
	{
		float distance = 0;
		if (deceleration > 0)
			distance = 0.5 * speed * speed / deceleration;
		else if (speed > 0)
			distance = float.INFINITY;

		return distance;
	}

	//------------------------------------------------------------------------------------------------
	//! Get speed necessary to reach given distance with specified deceleration.
	//! \param[in] distance
	//! \param[in] deceleration
	//! \return
	static float GetSpeedToReachDistance(float distance, float deceleration)
	{
		if (distance <= 0 || deceleration <= 0)
			return 0;

		return Math.Sqrt(2 * distance * deceleration);
	}

	//------------------------------------------------------------------------------------------------
	//! Get speed necessary to reach given distance with specified deceleration in specified time.
	//! \param[in] distance
	//! \param[in] deceleration
	//! \param[in] time
	//! \return
	static float GetSpeedToReachDistanceInTime(float distance, float deceleration, float time)
	{
		if (time <= 0)
			return float.INFINITY;

		// Get the decelerated part
		float speed = GetSpeedToReachDistance(distance, deceleration);
		float averageSpeed = distance / time;
		float additionalSpeed = averageSpeed - speed * 0.5;
		return speed + additionalSpeed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] min (included)
	//! \param[in] mid Gaussian curve's middle point
	//! \param[in] max (included)
	//! \return the random value
	static float GetGaussianDistributionRandom(float min, float mid, float max)
	{
		if (min == max)
			return min;

		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}

		float result = Math.RandomGaussFloat((max - min) / 6.0, mid); // ~99.73% cases covered

		if (result < min)
			return min;

		if (result > max)
			return max;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! This is Arma 3's random (syntax 2) formula
	//! \param[in] min (included)
	//! \param[in] mid Bates curve's middle point
	//! \param[in] max (included)
	//! \return the random value
	static float GetBatesDistributionRandom(float min, float mid, float max)
	{
		if (min == max)
			return min;

		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}

		const float bates = (
			RANDOM_GENERATOR.RandFloat01()
			+ RANDOM_GENERATOR.RandFloat01()
			+ RANDOM_GENERATOR.RandFloat01()
			+ RANDOM_GENERATOR.RandFloat01()
		) * 0.25;

		if (bates < 0.5)
			return Math.Lerp(min, mid, bates * 2);
		else
			return Math.Lerp(mid, max, bates * 2 - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the math random Generator
	//! \return the one SCR_Math's random generator
	static RandomGenerator GetMathRandomGenerator()
	{
		return RANDOM_GENERATOR;
	}

	//------------------------------------------------------------------------------------------------
	//! Ensures the angle is in range <-units; +units>
	//! \param[in] angle angle value
	//! \param[in] units Math.PI or 180
	//! \return float angle in range <-units; +units>
	static float FixAngle(float angle, float units = Math.PI)
	{
		return Math.Repeat(units + angle, units * 2) - units;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts angle to provided unit type from radians
	//! \param[in] radianAngleFrom
	//! \param[in] toUnitType
	//! \return
	static float ConvertFromRadians(float radianAngleFrom, SCR_EOpticsAngleUnits toUnitType)
	{
		switch (toUnitType)
		{
			case SCR_EOpticsAngleUnits.DEGREES:
				radianAngleFrom = radianAngleFrom * Math.RAD2DEG;
				break;

			case SCR_EOpticsAngleUnits.MILS_WP:
				radianAngleFrom = radianAngleFrom * Math.RAD2DEG / MILS_WP2DEG;
				break;

			case SCR_EOpticsAngleUnits.MILS_NATO:
				radianAngleFrom = radianAngleFrom * Math.RAD2DEG / MILS_NATO2DEG;
				break;

			case SCR_EOpticsAngleUnits.MILS_STRECK:
				radianAngleFrom = radianAngleFrom * Math.RAD2DEG / MILS_STRECK2DEG;
				break;

			default:
				radianAngleFrom = radianAngleFrom * 1000;	//from radians to milliradians
				break;
		}

		return radianAngleFrom;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts angle from provided unit type to radians
	//! \param[in] angleFrom
	//! \param[in] fromUnitType
	//! \return
	static float ConvertToRadians(float angleFrom, SCR_EOpticsAngleUnits fromUnitType)
	{
		switch (fromUnitType)
		{
			case SCR_EOpticsAngleUnits.DEGREES:
			{
				angleFrom = angleFrom * Math.DEG2RAD;
				break;
			}

			case SCR_EOpticsAngleUnits.MILS_WP:
			{
				angleFrom = angleFrom * MILS_WP2DEG * Math.DEG2RAD;
				break;
			}

			case SCR_EOpticsAngleUnits.MILS_NATO:
			{
				angleFrom = angleFrom * MILS_NATO2DEG * Math.DEG2RAD;
				break;
			}

			case SCR_EOpticsAngleUnits.MILS_STRECK:
			{
				angleFrom = angleFrom * MILS_STRECK2DEG * Math.DEG2RAD;
				break;
			}

			default:
			{
				angleFrom = angleFrom * 0.001;	//from milliradians to radians
				break;
			}
		}

		return angleFrom;
	}
}
