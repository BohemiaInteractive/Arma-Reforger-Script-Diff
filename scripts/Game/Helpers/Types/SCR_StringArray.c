// typedef?

class SCR_StringArray : array<string>
{
	//------------------------------------------------------------------------------------------------
	//! \return the number of empty entries in the array
	int CountEmptyEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (Get(i).IsEmpty())
				result++;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the number of empty or whitespace entries in the array
	int CountEmptyOrWhiteSpaceEntries()
	{
		int result;
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(Get(i)))
				result++;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] separator
	//! \param[in] joinEmptyEntries
	//! \return all items linked with the provided separator (e.g { "a", "b", "c" } separator ":" = "a:b:c")
	string Join(string separator = string.Empty, bool joinEmptyEntries = true)
	{
		return SCR_StringHelper.Join(separator, this, joinEmptyEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if an entry is empty, false otherwise
	bool HasEmptyEntry()
	{
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (Get(i).IsEmpty())
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true of an entry is empty or whitespace, false otherwise
	bool HasEmptyOrWhiteSpaceEntry()
	{
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(Get(i)))
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove empty entries, keep all others including the whitespace ones
	//! \return the amount of removed entries
	int RemoveEmptyEntries()
	{
		int removed;
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (Get(i).IsEmpty())
			{
				RemoveOrdered(i);
				++removed;
			}
		}

		return removed;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove empty and whitespace entries, keeping all the other ones
	//! \return the amount of removed entries
	int RemoveEmptyOrWhiteSpaceEntries()
	{
		int removed;
		for (int i = Count() - 1; i >= 0; --i)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(Get(i)))
			{
				RemoveOrdered(i);
				++removed;
			}
		}

		return removed;
	}

	//------------------------------------------------------------------------------------------------
	//! Set all entries to lowercase
	void ToLower()
	{
		foreach (int i, string entry : this)
		{
			entry.ToLower();
			Set(i, entry);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Set all entries to uppercase
	void ToUpper()
	{
		foreach (int i, string entry : this)
		{
			entry.ToUpper();
			Set(i, entry);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Trim all entries (uses string.TrimInPlace())
	void Trim()
	{
		foreach (int i, string entry : this)
		{
			entry.TrimInPlace();
			Set(i, entry);
		}
	}
}
