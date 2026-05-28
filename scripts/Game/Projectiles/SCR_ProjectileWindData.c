[BaseContainerProps(configRoot: true)]
class SCR_ProjectileWindData
{
	[Attribute("1", desc: "Init Speed Coef", params: "0 inf 0.01")]
	protected float m_fInitSpeedCoef;

	[Attribute("10", desc: "Speed of the wind")]
	protected float m_fWindSpeed;

	// two seprate arrays will keep the overall file size smaller than using something like ref array<ref Wrapper<TFloatArray>>, while stil being possible to be viewed in workbench

	//! [0] elevation angle
	//! [1] distance to point of impact
	//! [2] peak altitude
	[Attribute(defvalue: "", desc: "Firing solution\n[X] elevation angle in radians\n[Y] horizontal distance to impact position\n[Z] peak altitude")]
	protected ref TVectorArray m_aFiringSolution;

	//! [0] azimuth correction due to the perpendicular wind
	//! [1] distance difference due to the parallel wind
	//! [2] angle of impact
	[Attribute(defvalue: "", desc: "Data.\n[X] azimuth correction in milliradians for perpendicular wind\n[Y] difference between point of impact with parallel wind\n[Z] agle at which projectile has impacted the surface")]
	protected ref TVectorArray m_aValues;

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetInitSpeedCoef()
	{
		return m_fInitSpeedCoef;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetWindSpeed()
	{
		return m_fWindSpeed;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] angle in radians
	//! \return an array with data coresponding to the given angle, or null
	//! [0] angle in radians
	//! [1] distance in meters
	//! [2] peak altitude
	//! [3] azimuth correction in milliradians for the crosswind
	//! [4] distance correction in meters for the parallel wind
	//! [5] impcat angle
	array<float> GetDataByAngle(float angle)
	{
		if (angle <= 0 || angle > 90)
			return null;

		int lowId;
		if (angle < m_aFiringSolution[lowId][0])
			return null;

		int highId = m_aFiringSolution.Count() - 1;
		if (angle > m_aFiringSolution[highId][0])
			return null;

		int midId;
		vector first;
		vector second;
		while (lowId < highId)
		{
			midId = (lowId + highId) * 0.5;

			first = m_aFiringSolution[midId - 1];
			second = m_aFiringSolution[midId];

			if (first[0] > angle)
				highId = --midId;
			else if (second[0] <= angle)
				lowId = ++midId;
			else
				break;
		}

		float t = Math.InverseLerp(first[0], second[0], angle);
		vector fireSolutionOut = vector.Lerp(first, second, t);
		vector valuesOut = vector.Lerp(m_aValues[midId-1], m_aValues[midId], t);
		array<float> output = {
			fireSolutionOut[0],	// angle
			fireSolutionOut[1],	// distance
			fireSolutionOut[2],	// peak distance
			valuesOut[0],		// azimuth correction for perpendicular wind
			valuesOut[1],		// distance difference due to parallel wind
			valuesOut[2]		// peak altitude
		};

		return output;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] distance
	//! \param[in] directFire
	//! \return an array with data coresponding to the given angle, or null
	//! [0] angle in radians
	//! [1] distance in meters
	//! [2] peak altitude
	//! [3] azimuth correction in milliradians for the crosswind
	//! [4] distance correction in meters for the parallel wind
	//! [5] impcat angle
	array<float> GetDataByDisance(float distance, bool directFire)
	{
		if (distance < 0)
			return null;

		if (!m_aValues || m_aValues.IsEmpty())
			return null;

		int highId = m_aValues.Count() - 1;
		if (highId != 89) // we expect this to have 90 entries
			return null;

		int lowId;
		if (directFire)
		{
			highId = 44;
			if (distance < m_aFiringSolution[lowId][1])
				return null;

			if (distance > m_aFiringSolution[highId][1])
				return null;
		}
		else
		{
			lowId = 44;
			if (distance > m_aFiringSolution[lowId][1])
				return null;

			if (distance < m_aFiringSolution[highId][1])
				return null;
		}

		int midId;
		vector first;
		vector second;

		while (lowId < highId)
		{
			midId = (lowId + highId) * 0.5;

			first = m_aFiringSolution[midId - 1];
			second = m_aFiringSolution[midId];

			if (directFire)
			{
				if (first[1] > distance)
					highId = --midId;
				else if (second[1] <= distance)
					lowId = ++midId;
				else
					break;
			}
			else
			{
				if (first[1] <= distance)
					highId = --midId;
				else if (second[1] > distance)
					lowId = ++midId;
				else
					break;
			}
		}

		float t = Math.InverseLerp(first[1], second[1], distance);
		vector fireSolutionOut = vector.Lerp(first, second, t);
		vector valuesOut = vector.Lerp(m_aValues[midId-1], m_aValues[midId], t);
		array<float> output = {
			fireSolutionOut[0],	// angle
			fireSolutionOut[1],	// distance
			fireSolutionOut[2],	// peak distance
			valuesOut[0],		// azimuth correction for perpendicular wind
			valuesOut[1],		// distance difference due to parallel wind
			valuesOut[2]		// peak altitude
		};

		return output;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] initSpeedCoef
	//! \param[in] windSpeed
	//! \param[in] values
	//! \param[in] firingSolutions
	void SCR_ProjectileWindData(float initSpeedCoef, float windSpeed, notnull TVectorArray values, notnull TVectorArray firingSolutions)
	{
		if (!values || !firingSolutions || values.Count() != firingSolutions.Count())
			return;

		m_fInitSpeedCoef = initSpeedCoef;
		m_fWindSpeed = windSpeed;
		m_aValues = values;
		m_aFiringSolution = firingSolutions;
	}
}
