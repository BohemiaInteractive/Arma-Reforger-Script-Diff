class SCR_ArrayHelper
{
	//------------------------------------------------------------------------------------------------
	//! Given array of weights, get a random index based on the weight values
	//! \code
	//! int index1 = SCR_ArrayHelper.GetWeightedIndex({ 80, 15, 5 }, Math.RandomFloat01());
	//! // index1 has 80% chance of being 0, 15% chance of being 1, 5% chance being 2
	//!
	//! int index2 = SCR_ArrayHelper.GetWeightedIndex({ 10, 10, 10, 10 }, Math.RandomFloat01());
	//! // index2 has 25% chance of being 0, 25% chance of being 1, 25% chance of being 2, 25% chance of being 3
	//! \endcode
	//! i.e in an array [80, 15, 5], 80% of the time, we get index 0, 15% index 1 and 5% index 2, weight sum does not have to add up to any particular value,
	//! i.e [100, 100, 100, 100] gives equal 25% distribution (provided random distribution) for all 4 indices
	//! \param[in] weights array with weights
	//! \param[in] value a value between [0..1], can be random, perlin, or any other; if outside these boundaries, will be Math.Mod-ed
	//! \return index selected based on weight or -1 if weights is empty
	static int GetWeightedIndex(notnull array<float> weights, float value01)
	{
		if (weights.IsEmpty())
			return -1;

		if (value01 < 0 || value01 > 1)
			value01 = Math.Mod(value01, 1);

		float probabilitySum;
		for (int i, count = weights.Count(); i < count; i++)
		{
			probabilitySum += weights[i];
		}

		float add = 0;
		probabilitySum *= value01;

		for (int i, count = weights.Count(); i < count; i++)
		{
			add += weights[i];
			if (add > probabilitySum)
				return i;
		}

		return 0;
	}
}

//! T being the array ITEM class, not the full array<item>
//! In the case of references, simply use SCR_ArrayHelperT<OtherClass>, not SCR_ArrayHelperT<ref OtherClass>
//! Some methods may seem duplicated but they are actually variants for <T> and <ref T>
class SCR_ArrayHelperT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Check if two arrays are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] array1
	//! \param[in] array2
	//! \return true if equal, false if not
	static bool AreEqual(notnull array<T> array1, notnull array<T> array2)
	{
		if (array1.Count() != array2.Count())
			return false;

		foreach (int i, T value : array1)
		{
			if (array2[i] != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] source
	//! \return a copy of refs or null if source is null
	static array<T> GetCopy(array<T> source)
	{
		if (!source)
			return null;

		array<T> result = {};
		result.Copy(source);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets all items that are common to the two arrays
	//! \param[in] array1
	//! \param[in] array2
	//! \param[out] result
	static void Intersect(notnull array<T> array1, notnull array<T> array2, notnull out array<T> result/*, bool unique = true */)
	{
		result.Clear();
		int count1 = array1.Count();
		int count2 = array2.Count();
		if (count1 > count2)
		{
			for (int i = array2.Count() - 1; i >= 0; i--)
			{
				if (array1.Contains(array2[i]))
					result.Insert(array2[i]);
			}
		}
		else
		{
			for (int i = array1.Count() - 1; i >= 0; i--)
			{
				if (array2.Contains(array1[i]))
					result.Insert(array1[i]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove duplicates from the array by the righthand side (e.g { "a", "b", "a" } -> { "a", "b" }, not { "b", "a" }
	//! \param[in,out] items
	static void RemoveDuplicates(notnull inout array<T> items)
	{
		for (int i = items.Count() - 1; i >= 0; --i)
		{
			if (items.Find(items[i]) != i)
				items.RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse item orders
	//! OBSOLETE when array.Reverse appears
	//! \code
	//! array<int> values = { 2, 42, 33, 0 };
	//! SCR_ArrayHelperT<int>.Reverse(values);
	//! Print(values); // outputs { 0, 33, 42, 2 }
	//! \endcode
	//! \param[in,out] items
	static void Reverse(notnull inout array<T> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount * 0.5;
		itemsCount--;

		// 4 indices
		// flooredMiddle = 2
		// 0 <-> 3
		// 1 <-> 2
		// { 0 1 2 3 }
		// { 3 2 1 0 }

		// 5 indices
		// flooredMiddle = 2
		// 0 <-> 4
		// 1 <-> 3
		// 2 is central
		// { 0 1 2 3 4 }
		// { 4 3 2 1 0 }

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shuffle an array
	//! \code
	//! array<int> values = { 1, 2, 3, 4, 5 };
	//! SCR_ArrayHelperT<int>.Shuffle(values);
	//! Print(values); // outputs e.g { 4, 1, 5, 2, 3 }
	//! \endcode
	//! \param[in,out] items
	//! \param[in] shuffles number of shuffle passes to happen. min 1, max 10
	static void Shuffle(notnull inout array<T> items, int shuffles = 1)
	{
		if (items.Count() < 2)
			return;

		// two checks are faster than Math.ClampInt
		if (shuffles < 1)
			shuffles = 1;

		if (shuffles > 10)
			shuffles = 10;

		while (shuffles > 0)
		{
			for (int i, count = items.Count(); i < count; ++i)
			{
				int index1 = items.GetRandomIndex();
				int index2 = items.GetRandomIndex();
				if (index1 != index2)
					items.SwapItems(index1, index2);
			}

			shuffles--;
		}
	}

//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] toConvert
//	//! \param[in] result
//	// NATIVE version
//	static void ArrayToSet(notnull array<T> toConvert, notnull set<T> result)
//	{
//		result.Clear();
//		foreach (T item : toConvert)
//		{
//			result.Insert(item);
//		}
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] toConvert
//	//! \param[in] result
//	// NATIVE version
//	static void SetToArray(notnull set<T> toConvert, notnull array<T> result)
//	{
//		result.Clear();
//		foreach (T item : toConvert)
//		{
//			result.Insert(item);
//		}
//	}
}

//! REFERENCE version
class SCR_ArrayHelperRefT<Class T>
{
	//------------------------------------------------------------------------------------------------
	//! Check if two arrays are equal - shallow check, only class references are verified (not same class/same value)
	//! \param[in] array1
	//! \param[in] array2
	//! \return true if equal, false if not
	static bool AreEqual(notnull array<ref T> array1, notnull array<ref T> array2)
	{
		if (array1.Count() != array2.Count())
			return false;

		foreach (int i, T value : array1)
		{
			if (array2[i] != value)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Same as array.Copy, but for reference arrays
	//! \see GetCopy
	//! \param[in] source
	//! \param[in] destination
	static void CopyReferencesFromTo(notnull array<ref T> source, notnull array<ref T> destination)
	{
		destination.Clear();
		foreach (T sourceRef : source)
		{
			destination.Insert(sourceRef);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] source
	//! \return a copy of values or null if source is null
	static array<ref T> GetCopy(array<ref T> source)
	{
		if (!source)
			return null;

		array<ref T> result = {};
		foreach (T sourceRef : source)
		{
			result.Insert(sourceRef);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] source
	//! \return a copy of non-ref pointer values or null if source is null
	static array<T> GetNonRefCopy(array<ref T> source)
	{
		if (!source)
			return null;

		array<T> result = {};
		foreach (T sourceRef : source)
		{
			result.Insert(sourceRef);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets all items that are common to the two arrays
	//! \param[in] array1
	//! \param[in] array2
	//! \param[out] result
	static void Intersect(notnull array<ref T> array1, notnull array<ref T> array2, notnull out array<ref T> result/*, bool unique = true */)
	{
		result.Clear();
		int count1 = array1.Count();
		int count2 = array2.Count();
		if (count1 > count2)
		{
			for (int i = array2.Count() - 1; i >= 0; i--)
			{
				if (array1.Contains(array2[i]))
					result.Insert(array2[i]);
			}
		}
		else
		{
			for (int i = array1.Count() - 1; i >= 0; i--)
			{
				if (array2.Contains(array1[i]))
					result.Insert(array1[i]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove duplicates from the array by the righthand side (e.g { "a", "b", "a" } -> { "a", "b" }, not { "b", "a" }
	//! \param[in,out] items
	static void RemoveDuplicates(notnull inout array<ref T> items)
	{
		for (int i = items.Count() - 1; i >= 0; --i)
		{
			if (items.Find(items[i]) != i)
				items.RemoveOrdered(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse item orders
	//! OBSOLETE when array.Reverse appears
	//! \code
	//! array<int> values = { 2, 42, 33, 0 };
	//! SCR_ArrayHelperT<int>.Reverse(values);
	//! Print(values); // outputs { 0, 33, 42, 2 }
	//! \endcode
	//! \param[in,out] items
	static void Reverse(notnull inout array<ref T> items)
	{
		int itemsCount = items.Count();
		if (itemsCount < 2)
			return;

		int flooredMiddle = itemsCount * 0.5;
		itemsCount--;

		// 4 indices
		// flooredMiddle = 2
		// 0 <-> 3
		// 1 <-> 2
		// { 0 1 2 3 }
		// { 3 2 1 0 }

		// 5 indices
		// flooredMiddle = 2
		// 0 <-> 4
		// 1 <-> 3
		// 2 is central
		// { 0 1 2 3 4 }
		// { 4 3 2 1 0 }

		for (int i; i < flooredMiddle; i++)
		{
			items.SwapItems(i, itemsCount - i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Shuffle an array of references
	//! \code
	//! array<ref MyClass> values = { new MyClass(1), new MyClass(2), new MyClass(3), new MyClass(4), new MyClass(5) };
	//! SCR_ArrayHelperT<MyClass>.Shuffle(values);
	//! Print(values); // outputs e.g { MyClass(4), MyClass(1), MyClass(5), MyClass(2), MyClass(3) }
	//! \endcode
	//! \param[in,out] items
	//! \param[in] shuffles number of shuffle passes to happen. min 1, max 10
	static void ShuffleRef(notnull inout array<ref T> items, int shuffles = 1)
	{
		if (items.Count() < 2)
			return;

		// two checks are faster than Math.ClampInt
		if (shuffles < 1)
			shuffles = 1;

		if (shuffles > 10)
			shuffles = 10;

		while (shuffles > 0)
		{
			for (int i = 0, count = items.Count(); i < count; i++)
			{
				int index1 = Math.RandomInt(0, count);
				int index2 = Math.RandomInt(0, count);
				if (index1 != index2)
					items.SwapItems(index1, index2);
			}

			shuffles--;
		}
	}

//	//------------------------------------------------------------------------------------------------
//	//!
//	//! \param[in] toConvert
//	//! \param[in] result
//	// REFERENCE version
//	static void ArrayToSet(notnull array<ref T> toConvert, notnull set<ref T> result)
//	{
//		result.Clear();
//		foreach (T item : toConvert)
//		{
//			result.Insert(item);
//		}
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! \param[in] toConvert
//	//! \param[in] result
//	// REFERENCE version
//	static void SetToArray(notnull set<ref T> toConvert, notnull array<ref T> result)
//	{
//		result.Clear();
//		foreach (T item : toConvert)
//		{
//			result.Insert(item);
//		}
//	}
}
