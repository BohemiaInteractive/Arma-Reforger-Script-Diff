class SCR_SortedArray<Class TValue>
{
	protected ref array<int> m_aOrders = {};
	protected ref array<TValue> m_aValues = {};

	//------------------------------------------------------------------------------------------------
	//! \param[in] n index
	//! \return value at index n
	TValue Get(int n)
	{
		return m_aValues.Get(n);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets value of element at index [n]
	//! \param[in] n index
	//! \param[in] value new value
	void Set(int n, TValue value)
	{
		m_aValues.Set(n, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Insert new value with given order.
	//! Values are ordered from lowest to highest.
	//! \param[in] order order in array
	//! \param[in] value value to be inserted
	void Insert(int order, TValue value)
	{
		int index = Count();
		for (int i = 0; i < index; i++)
		{
			if (order < m_aOrders[i])
			{
				index = i;
				break;
			}
		}

		m_aOrders.InsertAt(order, index);
		m_aValues.InsertAt(value, index);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove value on given index.
	//! \param[in] i
	void Remove(int i)
	{
		m_aOrders.RemoveOrdered(i);
		m_aValues.RemoveOrdered(i);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all entries with given order number.
	//! \param[in] order order number to be removed
	void RemoveOrders(int order)
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (m_aOrders[i] == order)
			{
				m_aOrders.RemoveOrdered(i);
				m_aValues.RemoveOrdered(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all entries with given value.
	//! \param[in] value value to be removed
	void RemoveValues(TValue value)
	{
		for (int i = Count() - 1; i >= 0; i--)
		{
			if (m_aValues[i] == value)
			{
				m_aOrders.RemoveOrdered(i);
				m_aValues.RemoveOrdered(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] i entry index
	//! \return order number on given index
	int GetOrder(int i)
	{
		return m_aOrders[i];
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] i index
	//! \return value on given index
	TValue GetValue(int i)
	{
		return m_aValues[i];
	}

	//------------------------------------------------------------------------------------------------
	//! \return number of elements in the array
	int Count()
	{
		return m_aOrders.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the array size is 0, false otherwise
	bool IsEmpty()
	{
		return m_aOrders.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if the array contains given value
	bool Contains(TValue value)
	{
		return m_aValues.Contains(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Try to find given value in the array.
	//! \param[in] value Searched value
	//! \return index of first occurence, -1 when not found
	int Find(TValue value)
	{
		return m_aValues.Find(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Copy data from another sorted array.
	//! \param[in] from source array
	//! \return number of items in the array
	int CopyFrom(notnull SCR_SortedArray<TValue> from)
	{
		Clear();
		int count = from.Count();
		for (int i; i < count; i++)
		{
			m_aOrders.Insert(from.m_aOrders[i]);
			m_aValues.Insert(from.m_aValues[i]);
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Destroys all elements of the array and sets the Count to 0.
	void Clear()
	{
		m_aOrders.Clear();
		m_aValues.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Fills normal array with items in sorted order.
	//! \param[out] outArray target array
	//! \return number of items
	int ToArray(out notnull array<TValue> outArray)
	{
		return outArray.Copy(m_aValues);
	}

	//------------------------------------------------------------------------------------------------
	//! Print array values to log.
	void Debug()
	{
		PrintFormat("SCR_SortedArray count: %1", Count(), level: LogLevel.NORMAL);
		for (int i, count = Count(); i < count; i++)
		{
			PrintFormat("[%1] => %2: %3", i, m_aOrders[i], m_aValues[i], level: LogLevel.NORMAL);
		}
	}
}
