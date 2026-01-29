class SCR_AttributesHelper
{
	protected static const string ENTRY_DELIMITER = ";";
	protected static const string VALUE_DELIMITER = "|";

	//------------------------------------------------------------------------------------------------
	//! Parses semicolon-separated string into ParamEnumArray. Enum value is assumed to be ascending.
	//! Entries with zero-length titles are omitted.
	//! \param[in] titles Example: First Enum; Second Enum; Best Enum
	//! \return an array of ParamEnum objects created from semicolon-separated string titles.
	static ParamEnumArray ParamFromTitles(string titles)
	{
		array<string> titlesArray = {};
		titles.Split(ENTRY_DELIMITER, titlesArray, false);
		ParamEnumArray params = new ParamEnumArray();
		foreach (int i, string title : titlesArray)
		{
			if (title) // !.IsEmpty() not necessary
				params.Insert(new ParamEnum(title.Trim(), i.ToString()));
		}

		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts semicolon-separated descriptions into ParamEnumArray for enum type.
	//! Entries with zero-length descriptions are omitted (pre-trimmed).
	//! \param[in] enumType Converts separated descriptions into ParamEnumArray for given enum type.
	//! \param[in] descriptions Example: "First Enum; Second Enum; Best Enum"
	//! \return an array of ParamEnum objects, each representing an enum value from the provided descriptions.
	static ParamEnumArray ParamFromDescriptions(typename enumType, string descriptions)
	{
		array<string> descriptionsArray = {};
		descriptions.Split(ENTRY_DELIMITER, descriptionsArray, false);
		int variableCount = enumType.GetVariableCount();
		ParamEnumArray params = new ParamEnumArray();
		foreach (int i, string description : descriptionsArray)
		{
			int enumValue;
			if (description && enumType.GetVariableValue(NULL, i, enumValue))
				params.Insert(new ParamEnum(enumType.GetVariableName(i), enumValue.ToString(), description.Trim()));
		}

		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Parses pipe-separated descriptions into ParamEnum array.
	//! Entries with empty titles are preserved. Entry fields are trimmed.
	//! \param[in] document e.g "key1|value1;key2;key3|value3|description3;"
	//! \return The method takes a string document containing pipe-separated descriptions, extracts each description into title, value, and description,
	static ParamEnumArray ParamFromEntries(string document)
	{
		array<string> entries = {};
		document.Split(ENTRY_DELIMITER, entries, false);
		array<string> values = {}; // Reuse instance of values array to avoid unnecessary allocations.
		ParamEnumArray params = new ParamEnumArray();
		foreach (string entry : entries)
		{
			string title, value, description;
			SplitEntryIntoValues(entry, values, title, value, description, VALUE_DELIMITER);
			params.Insert(new ParamEnum(title.Trim(), value.Trim(), description.Trim()));
			values.Clear();
		}

		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts an array of ParamEnum objects into a string with key, value, description format.
	//! Output is compatible with ParamFromEntries.
	//! \param[in] enumArray Converts enum array to string with key, value, description format.
	//! \return a string containing descriptions for each enum value in the array, separated by '|'.
	static string ParamToEntries(ParamEnumArray enumArray)
	{
		string result;
		foreach (int i, ParamEnum paramEnum : enumArray)
		{
			result += string.Format("%2%1%3%1%4%5", VALUE_DELIMITER, paramEnum.m_Key, paramEnum.m_Value, paramEnum.m_Desc, ENTRY_DELIMITER);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Splits entry into title, value, and description.
	//! Entry values are sperated by delimiter (Default '|')
	//! \param[in] entry Input text.
	//! \param[in] values Array that values are split into. Used to reduce allocates from repeated runs.
	//! \param[out] title Title untrimmed.
	//! \param[out] value Value untrimmed.
	//! \param[out] description Description untrimmed.
	static void SplitEntryIntoValues(string entry, notnull array<string> values, out string title, out string value, out string description, string delimiter = VALUE_DELIMITER)
	{
		entry.Split(delimiter, values, false);
		int valuesCount = values.Count();
		if (valuesCount <= 0)
			return;

		title = values[0];
		if (valuesCount <= 1)
			return;

		value = values[1];
		if (valuesCount <= 2)
			return;

		description = values[2];
	}
}
