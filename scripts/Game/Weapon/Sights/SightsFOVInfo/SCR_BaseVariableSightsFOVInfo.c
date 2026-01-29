class SCR_BaseVariableSightsFOVInfo: ScriptedSightsFOVInfo
{
	//------------------------------------------------------------------------------------------------
	/*!
		Returns the number of available elements.
	*/
	int GetCount() { return 0; }

	//------------------------------------------------------------------------------------------------
	/*!
		Returns currently selected element or -1 if none.
	*/
	int GetCurrentIndex() { return -1; }

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the index of next available element.
		\param allowOverflow When enabled the selection will be able to cycle start to end.
		\return Next index or -1 if none.
	*/
	protected int GetNextIndex(bool allowOverflow = true)
	{
		int count = GetCount();
		if (count < 1)
			return -1;

		int current = GetCurrentIndex();
		int next = current + 1;
		if (next > count - 1)
		{
			if (allowOverflow)
				return 0;
			else
				return -1;
		}

		return next;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns the index of previous available element.
		\param allowUnderflow When enabled the selection will be able to cycle start to end.
		\return Next index or -1 if none.
	*/
	protected int GetPreviousIndex(bool allowUnderflow = true)
	{
		int count = GetCount();
		if (count < 1)
			return -1;

		int current = GetCurrentIndex();
		int previous = current - 1;
		if (previous < 0)
		{
			if (allowUnderflow)
				return count - 1;
			else return -1;
		}

		return previous;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Set provided element as the current one.
		\param index Index of FOV element.
	*/
	protected void SetIndex(int index);

	//------------------------------------------------------------------------------------------------
	/*!
		Selects next available element.
		\param allowOverflow When enabled the selection will be able to cycle start to end.
		\return True on success, false otherwise.
	*/
	bool SetNext(bool allowOverflow = true)
	{
		int next = GetNextIndex(allowOverflow);
		if (next != -1)
		{
			SetIndex(next);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Selects previous available element.
		\param allowUnderflow When enabled the selection will be able to cycle start to end.
		\return True on success, false otherwise.
	*/
	bool SetPrevious(bool allowUnderflow = true)
	{
		int prev = GetPreviousIndex(allowUnderflow);
		if (prev != -1)
		{
			SetIndex(prev);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when current FOV of the optic does not match the target FOV.
	bool IsAdjusting()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when current FOV of the optic does not match the target FOV.
	float GetBaseFOV()
	{
		return GetCurrentFOV();
	}
};
