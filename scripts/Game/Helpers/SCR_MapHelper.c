//! T being the key type and U being the value type
//! In the case of references, simply use SCR_MapHelperT<ClassA, ClassB>, not SCR_MapHelperT<ref ClassA, ref ClassB>
//! Some methods may seem quadruplicated but they are actually variants for <T, U>, <T, ref U>, <ref T, U> and <ref T, ref U>
class SCR_MapHelperT<Class T, Class U>
{
	//------------------------------------------------------------------------------------------------
	//! Check if two maps are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] map1
	//! \param[in] map2
	//! \return true if equal, false if not
	// NATIVE-NATIVE version
	static bool AreEqual(notnull map<T, U> map1, notnull map<T, U> map2)
	{
		if (map1.Count() != map2.Count())
			return false;

		U tmp;
		foreach (T index, U value : map1)
		{
			if (!map2.Find(index, tmp) || tmp != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if two maps are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] map1
	//! \param[in] map2
	//! \return true if equal, false if not
	// NATIVE-REFERENCE version
	static bool AreEqual(notnull map<T, ref U> map1, notnull map<T, ref U> map2)
	{
		if (map1.Count() != map2.Count())
			return false;

		U tmp;
		foreach (T index, U value : map1)
		{
			if (!map2.Find(index, tmp) || tmp != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if two maps are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] map1
	//! \param[in] map2
	//! \return true if equal, false if not
	// REFERENCE-NATIVE version
	static bool AreEqual(notnull map<ref T, U> map1, notnull map<ref T, U> map2)
	{
		if (map1.Count() != map2.Count())
			return false;

		U tmp;
		foreach (T index, U value : map1)
		{
			if (!map2.Find(index, tmp) || tmp != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if two maps are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] map1
	//! \param[in] map2
	//! \return true if equal, false if not
	// REFERENCE-REFERENCE version
	static bool AreEqual(notnull map<ref T, ref U> map1, notnull map<ref T, ref U> map2)
	{
		if (map1.Count() != map2.Count())
			return false;

		U tmp;
		foreach (T index, U value : map1)
		{
			if (!map2.Find(index, tmp) || tmp != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a map with indices[x] -> values[x] map. The result will be of the size of the smallest array.
	//! \param[in] indices
	//! \param[in] values
	//! \return
	// NATIVE-NATIVE version
	static map<T, U> ArraysToMap(notnull array<T> indices, notnull array<U> values)
	{
		map<T, U> result = new map<T, U>();
		for (int i, count = Math.Min(indices.Count(), values.Count()); i < count; i++)
		{
			result.Insert(indices[i], values[i]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a map with indices[x] -> values[x] map. The result will be of the size of the smallest array.
	//! \param[in] indices
	//! \param[in] values
	//! \return
	// NATIVE-REFERENCE version
	static map<T, ref U> ArraysToMap(notnull array<T> indices, notnull array<ref U> values)
	{
		map<T, ref U> result = new map<T, ref U>();
		for (int i, count = Math.Min(indices.Count(), values.Count()); i < count; i++)
		{
			result.Insert(indices[i], values[i]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a map with indices[x] -> values[x] map. The result will be of the size of the smallest array.
	//! \param[in] indices
	//! \param[in] values
	//! \return
	// REFERENCE-NATIVE version
	static map<ref T, U> ArraysToMap(notnull array<ref T> indices, notnull array<U> values)
	{
		map<ref T, U> result = new map<ref T, U>();
		for (int i, count = Math.Min(indices.Count(), values.Count()); i < count; i++)
		{
			result.Insert(indices[i], values[i]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a map with indices[x] -> values[x] map. The result will be of the size of the smallest array.
	//! \param[in] indices
	//! \param[in] values
	//! \return
	// REFERENCE-REFERENCE version
	static map<ref T, ref U> ArraysToMap(notnull array<ref T> indices, notnull array<ref U> values)
	{
		map<ref T, ref U> result = new map<ref T, ref U>();
		for (int i, count = Math.Min(indices.Count(), values.Count()); i < count; i++)
		{
			result.Insert(indices[i], values[i]);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// NATIVE-NATIVE version
	static array<T> GetKeys(notnull map<T, U> input)
	{
		array<T> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(key);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// NATIVE-REFERENCE version
	static array<T> GetKeys(notnull map<T, ref U> input)
	{
		array<T> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(key);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// REFERENCE-NATIVE version
	static array<ref T> GetKeys(notnull map<ref T, U> input)
	{
		array<ref T> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(key);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// REFERENCE-REFERENCE version
	static array<ref T> GetKeys(notnull map<ref T, ref U> input)
	{
		array<ref T> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(key);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// NATIVE-NATIVE version
	static array<U> GetElements(notnull map<T, U> input)
	{
		array<U> result = {};
		for (int i = 0, count = input.Count(); i < count; i++)
		{
			result.Insert(input.GetElement(i));
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// NATIVE-REFERENCE version
	static array<ref U> GetElements(notnull map<T, ref U> input)
	{
		array<ref U> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// REFERENCE-NATIVE version
	static array<U> GetElements(notnull map<ref T, U> input)
	{
		array<U> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \return
	// REFERENCE-REFERENCE version
	static array<ref U> GetElements(notnull map<ref T, ref U> input)
	{
		array<ref U> result = {};
		foreach (T key, U value : input)
		{
			result.Insert(value);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] receiver
	//! \param[in] sender
	//! \param[in] allowOverride
	// NATIVE-NATIVE version
	static void InsertAll(notnull map<T, U> receiver, notnull map<T, U> sender, bool allowOverride = true)
	{
		if (allowOverride) // done so for performance
		{
			foreach (T key, U value : sender)
			{
				receiver.Set(key, value);
			}
		}
		else
		{
			foreach (T key, U value : sender)
			{
				receiver.Insert(key, value);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] receiver
	//! \param[in] sender
	//! \param[in] allowOverride
	// NATIVE-REFERENCE version
	static void InsertAll(notnull map<T, ref U> receiver, notnull map<T, ref U> sender, bool allowOverride = true)
	{
		if (allowOverride) // done so for performance
		{
			foreach (T key, U value : sender)
			{
				receiver.Insert(key, value);
			}
		}
		else
		{
			foreach (T key, U value : sender)
			{
				if (!receiver.Contains(key))
					receiver.Insert(key, value);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] receiver
	//! \param[in] sender
	//! \param[in] allowOverride
	// REFERENCE-NATIVE version
	static void InsertAll(notnull map<ref T, U> receiver, notnull map<ref T, U> sender, bool allowOverride = true)
	{
		if (allowOverride) // done so for performance
		{
			foreach (T key, U value : sender)
			{
				receiver.Insert(key, value);
			}
		}
		else
		{
			foreach (T key, U value : sender)
			{
				if (!receiver.Contains(key))
					receiver.Insert(key, value);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] receiver
	//! \param[in] sender
	//! \param[in] allowOverride
	// REFERENCE-REFERENCE version
	static void InsertAll(notnull map<ref T, ref U> receiver, notnull map<ref T, ref U> sender, bool allowOverride = true)
	{
		if (allowOverride) // done so for performance
		{
			foreach (T key, U value : sender)
			{
				receiver.Insert(key, value);
			}
		}
		else
		{
			foreach (T key, U value : sender)
			{
				if (!receiver.Contains(key))
					receiver.Insert(key, value);
			}
		}
	}
}

class SCR_MapHelper<Class T, Class U>
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] input
	//! \param[in] value
	//! \return
	static T GetKeyByValue(notnull map<T, U> input, U value)
	{
		for (int i, count = input.Count(); i < count; i++)
		{
			if (input.GetElement(i) == value)
			{
				return input.GetKey(i);
				break;
			}
		}

		const T result;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] input
	//! \param[in] oldKey
	//! \param[in] newKey
	//! \return
	static bool ReplaceKey(notnull map<T, U> input, T oldKey, T newKey)
	{
		if (!input.Contains(oldKey))
			return false;

		input.Set(newKey, input.Get(oldKey));
		input.Remove(oldKey);
		return true;
	}
}