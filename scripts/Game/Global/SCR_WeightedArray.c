class SCR_WeightedArray<Class TValue>
{
	protected ref array<TValue> m_aValues = {};
	protected ref array<float> m_aWeights = {};
	protected float m_fTotalWeight;

	//------------------------------------------------------------------------------------------------
	//! \param[out] outValue
	//! \return
	int GetRandomValue(out TValue outValue)
	{
		return GetWeightedValue(outValue, Math.RandomFloat(0, m_fTotalWeight));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outValue
	//! \param[in] randomGenerator
	//! \return
	int GetRandomValue(out TValue outValue, notnull RandomGenerator randomGenerator)
	{
		return GetWeightedValue(outValue, randomGenerator.RandFloatXY(0, m_fTotalWeight));
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outValue
	//! \param[in] weightedValue
	//! \return
	int GetWeightedValue(out TValue outValue, float weightedValue)
	{
		float weight;
		for (int i, count = m_aValues.Count(); i < count; i++)
		{
			weight += m_aWeights[i];
			if (weightedValue < weight)
			{
				outValue = m_aValues[i];
				return i;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetTotalWeight()
	{
		return m_fTotalWeight;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] n
	//! \return
	TValue Get(int n)
	{
		return m_aValues.Get(n);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] n
	//! \param[in] value
	void Set(int n, TValue value)
	{
		m_aValues.Set(n, value);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \param[in] weight
	//! \return
	int Insert(TValue value, float weight)
	{
		m_aWeights.Insert(weight);
		m_fTotalWeight += weight;
		return m_aValues.Insert(value);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] i
	void Remove(int i)
	{
		m_fTotalWeight -= m_aWeights[i];
		m_aValues.Remove(i);
		m_aWeights.Remove(i);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] i
	void RemoveOrdered(int i)
	{
		m_fTotalWeight -= m_aWeights[i];
		m_aValues.RemoveOrdered(i);
		m_aWeights.RemoveOrdered(i);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] i
	//! \return
	float GetWeight(int i)
	{
		return m_aWeights[i];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] i
	//! \return
	TValue GetValue(int i)
	{
		return m_aValues[i];
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int Count()
	{
		return m_aValues.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsEmpty()
	{
		return m_aValues.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \return
	bool Contains(TValue value)
	{
		return m_aValues.Contains(value);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \return
	int Find(TValue value)
	{
		return m_aValues.Find(value);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] from
	//! \return
	int CopyFrom(notnull SCR_WeightedArray<TValue> from)
	{
		Clear();
		int count = from.Count();
		for (int i; i < count; i++)
		{
			m_aWeights.Insert(from.m_aWeights[i]);
			m_aValues.Insert(from.m_aValues[i]);
		}

		m_fTotalWeight = from.m_fTotalWeight;

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Clear()
	{
		m_aWeights.Clear();
		m_aValues.Clear();
		m_fTotalWeight = 0;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[out] outArray
	//! \return
	int ToArray(out notnull array<TValue> outArray)
	{
		return outArray.Copy(m_aValues);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void Debug()
	{
		PrintFormat("SCR_WeightedArray<%1> count: %2, total weight: %3", TValue, Count(), GetTotalWeight(), level: LogLevel.NORMAL);
		foreach (int i, TValue value : m_aValues)
		{
			PrintFormat("[%1] => %2: %3", i, m_aWeights[i], value, level: LogLevel.NORMAL);
		}
	}
}
