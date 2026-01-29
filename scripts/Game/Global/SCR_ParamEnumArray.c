class SCR_ParamEnumArray : ParamEnumArray
{
	protected static const int CORE_MODULE_COUNT = 2;

	protected static const string ENTRY_SEPARATOR = SCR_StringHelper.SEMICOLON;
	protected static const string VALUE_SEPARATOR = SCR_StringHelper.COMMA;
	protected static const string ESCAPE_CHARACTER = SCR_StringHelper.SINGLE_QUOTE;

	//------------------------------------------------------------------------------------------------
	//! Get a ParamEnumArray of named addon indices
	//! Example:
	//! \code
	//! [Attribute(desc: "Pick an addon", uiwidget: UIWidgets.ComboBox, enums: SCR_ParamEnumArray.FromAddons())]
	//! protected int m_iAddon;
	//! \endcode
	//! \param[in] titleFormat 0 for "AddonID", 1 for "AddonTitle", 2 for "AddonTitle (AddonID)"
	//! \param[in] hideCoreModules 0 to hide nothing, 1 to hide core (vanilla) addons, 2 to hide core addons only when more addons are available
	//! \return an [Attribute] combobox-compatible ParamEnumArray value (for int variable)
	static ParamEnumArray FromAddons(int titleFormat = 2, int hideCoreModules = 0)
	{
		ParamEnumArray params = new ParamEnumArray();
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs);

		int count = addonGUIDs.Count();
		foreach (int i, string addonGUID : addonGUIDs)
		{
			if (hideCoreModules == 1 && GameProject.IsVanillaAddon(addonGUID))
				continue;

			if (hideCoreModules == 2 && count > CORE_MODULE_COUNT && GameProject.IsVanillaAddon(addonGUID))
				continue;

			string title;
			switch (titleFormat)
			{
				case 0: title = GameProject.GetAddonID(addonGUID); break;
				case 1: title = GameProject.GetAddonTitle(addonGUID); break;
				default:
				case 2: title = string.Format("%1 (%2)", GameProject.GetAddonTitle(addonGUID), GameProject.GetAddonID(addonGUID)); break;
			}

			params.Insert(new ParamEnum(title, i.ToString(), addonGUID));
		}

		return params;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a ParamEnumArray of array-provided enum values
	//! \param[in] enumTypeName
	//! \param[in] enumValues keeps the provided order and skips invalid/combined values
	//! \return wanted array if everything is fine, FromEnum result if enumValues is empty, null if enumTypeName is null
	static ParamEnumArray FromEnumArray(typename enumTypeName, notnull array<int> enumValues)
	{
		if (!enumTypeName)
			return null;

		if (enumValues.IsEmpty())
			return FromEnum(enumTypeName);

		ParamEnumArray result = new ParamEnumArray();
		int val;

		map<int, string> possibleValues = new map<int, string>();
		for (int i, count = enumTypeName.GetVariableCount(); i < count; ++i)
		{
			if (enumTypeName.GetVariableType(i) == int && enumTypeName.GetVariableValue(null, i, val))
				possibleValues.Insert(val, enumTypeName.GetVariableName(i));
		}

		foreach (int enumValue : enumValues)
		{
			if (possibleValues.Contains(enumValue))
				result.Insert(new ParamEnum(possibleValues.Get(enumValue), enumValue.ToString()));
		}

		return result;
	}

//	//------------------------------------------------------------------------------------------------
//	//! Get a ParamEnumArray of mask-provided enum values
//	//! \param[in] enumTypeName
//	//! \param[in] mask
//	//! \return ParamEnumArray or null if enumTypeName is null
//	static ParamEnumArray FromEnumMask(typename enumTypeName, int mask, bool avoidZero = false)
//	{
//		if (!enumTypeName)
//			return null;
//
//		ParamEnumArray result = new ParamEnumArray();
//		int val;
//
//		for (int i, count = enumTypeName.GetVariableCount(); i < count; ++i)
//		{
//			if (enumTypeName.GetVariableType(i) == int && enumTypeName.GetVariableValue(null, i, val))
//			{
//				if ((val == 0 && !avoidZero) || (val & mask) == val)
//					result.Insert(new ParamEnum(enumTypeName.GetVariableName(i), val.ToString()));
//			}
//		}
//
//		return result;
//	}

	//------------------------------------------------------------------------------------------------
	//! Get a ParamEnumArray of enum values with the ability to skip beginning and/or end ones
	//! \param[in] enumTypeName the enum typename
	//! \param[in] skipFirst how many values from start must be skipped
	//! \param[in] avoidLast how many values from end must be avoided
	//! \return the array, or full FromEnum on invalid skip/avoid values, or null on null enumTypeName
	static ParamEnumArray FromEnumSkip(typename enumTypeName, int skipFirst, int skipLast = 0)
	{
		if (!enumTypeName)
			return null;

		ParamEnumArray result = ParamEnumArray.FromEnum(enumTypeName);
		int count = result.Count();
		if (count < 2)
			return result;

		if (skipFirst < 0)
			skipFirst = 0;

		if (skipLast < 0)
			skipLast = 0;

		if (skipFirst + skipLast >= count)
			return result;

		if (skipFirst > count)
			skipFirst = 0;

		if (skipLast > count)
			skipLast = 0;

		for (int i; i < skipFirst; ++i)
		{
			result.RemoveOrdered(0);
		}

		count -= skipFirst;

		for (int i; i < skipLast; ++i)
		{
			result.Remove(--count);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get ComboBox entries from string, format:
	//! - "text1;text2;text3"
	//! - "text1,desc1;text2,desc2;text3,desc3"
	//! - "value1,text1,desc1;value2,text2,desc2;value3,text3,desc3"
	//! If value is not defined, array index is used ("0", "1", "2", etc)
	//! \note commas and semicolons can be escaped using a single quote (" ', ", " '; ")
	//! \param[in] input format can be (entries are separated by semicolons ";"):
	//! - text
	//! - text,description
	//! - value,text,description (if there is no description, use a trailing comma to keep an empty description e.g "value,text,")
	//! \return the resulting ParamEnumArray - never null
	static ParamEnumArray FromString(string input)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(input))
			return { new ParamEnum("No options set", "0", "No options set - please edit the Attribute's SCR_ParamEnumArray.FromString string value") };

		array<string> entries = SplitString(input, ENTRY_SEPARATOR);

		ParamEnumArray result = {};
		array<string> values;

		foreach (int i, string entry : entries)
		{
			entry.TrimInPlace();
			values = SplitString(entry, VALUE_SEPARATOR, 3);

			int valuesCount = values.Count();
			if (valuesCount < 1) // cannot happen
				continue;

			foreach (int j, string value2 : values)
			{
				value2.TrimInPlace();
				values[j] = value2;
			}

			string value = i.ToString();
			string text;
			string description;

			if (valuesCount == 1)		// 1 - text only
			{
				text = values[0];
				text.TrimInPlace();
			}
			else if (valuesCount == 2)	// 2 - text and description
			{
				text = values[0];
				description = values[1];

				text.TrimInPlace();
				description.TrimInPlace();
			}
			else						// 3 - VALUE, text and description - other splits are joined into description
			{
				value = values[0];
				text = values[1];
				description = values[2];

				// value.TrimInPlace();	// we keep spaces, they may eventually be useful
				text.TrimInPlace();
				description.TrimInPlace();
			}

			if (description)
				description += "\nValue: " + value;
			else
				description = "Value: " + value;

			result.Insert(new ParamEnum(text, value, description));
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Split the provided string, considering escaped separators
	//! \param[in] input the string to split
	//! \param[in] separator comma or semicolon (can factually be anything)
	//! \param[in] maxCount if > 0, limit the max number of splits
	//! \return the split string
	protected static array<string> SplitString(string input, string separator, int maxCount = 0)
	{
		if (!separator) // .IsEmpty()
			return { input };

		array<string> result = {};
		input.Split(separator, result, false);

		for (int i = result.Count() - 1; i >= 1; --i)
		{
			if (result[i - 1].EndsWith(ESCAPE_CHARACTER))
			{
				result[i - 1] = result[i - 1].Substring(0, result[i - 1].Length() - 1) + separator + result[i];
				result.RemoveOrdered(i);
			}
		}

		if (maxCount > 0)
		{
			int count = result.Count();
			if (count > maxCount)
			{
				for (int i = count - 1; i > maxCount - 1; --i)
				{
					result[2] = result[2] + separator + result[i];
					result.Remove(i);
				}
			}
		}

		return result;
	}
}
