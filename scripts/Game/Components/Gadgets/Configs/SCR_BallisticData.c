class SCR_BallisticData : ScriptAndConfig
{

	static ref array<ref SCR_BallisticData> s_aBallistics;

	//! expected structure
	//! [0] range,
	//! [1] angle,
	//! [2] time of flight
	//! [3] drop per 100m of elevation change,
	//! [4] time of flight change per 100m of elevation change,
	protected ref array<ref array<float>> m_aBallisticValues;
	protected ResourceName m_sProjectilePrefabName;
	protected bool m_bDirectFireMode;
	protected int m_iRangeStep;
	protected float m_fProjectileInitSpeedCoef;
	protected ref map<int, int> m_mDistances;

	//------------------------------------------------------------------------------------------------
	int GetNumberOfEntries()
	{
		if (!m_aBallisticValues)
			return 0;

		return m_aBallisticValues.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	//! \param[out] values 
	//! \return number of values in returned array
	int GetValues(int id, out array<float> values)
	{
		if (!m_aBallisticValues.IsIndexValid(id))
			return 0;

		values = m_aBallisticValues[id];
		return values.Count();
	}


	//------------------------------------------------------------------------------------------------
	//! Method for changing the content of existing entry
	//! \param[in] id
	//! \param[in] values
	void SetValues(int id, notnull array<float> newValues)
	{
		if (!m_aBallisticValues.IsIndexValid(id))
			return;

		m_aBallisticValues[id] = newValues;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds values for first closest range 
	//! \param[in] range searched range
	//! \param[out] id position in m_aBallisticValues which contains returned value (-1 no value found)
	//! \return 
	array<float> GetBallisticValuesForClosestRange(int range, out int id = -1)
	{
		if (m_iRangeStep == 0 || !m_aBallisticValues || m_aBallisticValues.IsEmpty())
			return null;

		range = Math.Round(range / m_iRangeStep) * m_iRangeStep;
		if (m_mDistances.Contains(range))
			return GetBallisticValuesForRange(range, id);

		if (range <= m_aBallisticValues[0][0])
		{
			id = 0;
			return m_aBallisticValues[0];
		}

		id = m_aBallisticValues.Count() - 1;
		return m_aBallisticValues[id];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] angle searched angle
	//! \param[out] id position in m_aBallisticValues which contains returned value (-1 no value found)
	//! \param[in] epsilon this value will be increased by epsilonStep with each recursive exectution to widen the range
	//! \param[in] epsilonStep
	//! \param[in] maxEpsilon if epsilon reaches this value then search will be stop and null will be returned
	//! \return
	array<float> GetBallisticValuesForClosestAngle(float angle, out int id = -1, float epsilon = 0.1, float epsilonStep = 2, float maxEpsilon = 1000)
	{
		if (epsilon > maxEpsilon)
		{
			angle = -1;
			id = -1;
			return null;
		}

		foreach(int i, array<float> entry : m_aBallisticValues)
		{
			if (float.AlmostEqual(entry[1], angle, epsilon))
			{
				angle = entry[0];
				id = i;
				return entry;
			}
		}

		return GetBallisticValuesForClosestAngle(angle, id, epsilon * epsilonStep, epsilonStep, maxEpsilon);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] range
	//! \param[out] id
	//! \return
	array<float> GetBallisticValuesForRange(int range, out int id = -1)
	{
		if (!m_mDistances.Contains(range))
			return null;

		id = m_mDistances.Get(range);
		return m_aBallisticValues[id];
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetProjectileName()
	{
		return m_sProjectilePrefabName;
	}

	//------------------------------------------------------------------------------------------------
	int GetRangeStep()
	{
		return m_iRangeStep;
	}

	//------------------------------------------------------------------------------------------------
	float GetProjectileInitSpeedCoef()
	{
		return m_fProjectileInitSpeedCoef;
	}

	//------------------------------------------------------------------------------------------------
	bool IsForDirectFireMode()
	{
		return m_bDirectFireMode;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] ballisticValues
	//! \param[in] projectilePrefab
	//! \param[in] directFireMode
	//! \param[in] rangeStep
	void SCR_BallisticData(notnull array<ref array<float>> ballisticValues, string projectilePrefab, bool directFireMode, int rangeStep, float speedCoef)
	{
		m_aBallisticValues = ballisticValues;
		if (!m_aBallisticValues && m_aBallisticValues.IsEmpty())
			return;

		m_mDistances = new map<int, int>();
		foreach (int i, array<float> ballistics : m_aBallisticValues)
		{
			m_mDistances.Insert(ballistics[0], i);
		}
		m_sProjectilePrefabName = projectilePrefab;
		m_bDirectFireMode = directFireMode;
		m_iRangeStep = rangeStep;
		m_fProjectileInitSpeedCoef = speedCoef;
	}
}
