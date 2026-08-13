#ifdef WORKBENCH
class SCR_PrefabFilesHelper
{
	//------------------------------------------------------------------------------------------------
	// methods below should NOT exist in an ideal world; let's try and reach it one day
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Read the class name directly from the Prefab file
	//! \param[in] resourceName
	//! \return classname from file, empty string on error
	static string GetClassName(ResourceName resourceName)
	{
		array<string> fileLines = SCR_FileIOHelper.ReadFileContent(resourceName.GetPath(), false);
		if (!fileLines)
		{
			Print("Cannot read " + resourceName, LogLevel.WARNING);
			return string.Empty;
		}

		if (fileLines.IsEmpty()) // huh?
			return string.Empty;

		array<string> classSplit = {};
		fileLines[0].Split(" ", classSplit, true);

		if (classSplit.Count() < 2)
		{
			Print("Unknown file format for class-renaming " + resourceName, LogLevel.WARNING);
			return string.Empty;
		}

		classSplit[0].Trim();

		return classSplit[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Set the class name directly in the Prefab file
	//! \param[in] resourceName
	//! \param[in] newClassName
	//! \return true on success, false otherwise
	static bool SetClassName(ResourceName resourceName, string newClassName)
	{
		newClassName.Trim();
		if (!newClassName)
			return false;

		array<string> fileLines = SCR_FileIOHelper.ReadFileContent(resourceName.GetPath(), false);
		if (!fileLines)
		{
			Print("Cannot read " + resourceName, LogLevel.WARNING);
			return false;
		}

		if (fileLines.IsEmpty()) // huh?
			return false;

		array<string> classSplit = {};
		fileLines[0].Split(":", classSplit, true);
		int countSplit = classSplit.Count();
		if (countSplit < 1 || countSplit > 2)
		{
			Print("Unknown file format for class-renaming " + resourceName, LogLevel.WARNING);
			return false;
		}

		if (countSplit == 1)
		{
			fileLines[0].Split(" ", classSplit, true);
			countSplit = classSplit.Count();
			if (countSplit < 2)
			{
				Print("Unknown file format for class-renaming " + resourceName, LogLevel.WARNING);
				return false;
			}

			classSplit[0] = newClassName;
			fileLines[0] = SCR_StringHelper.Join(" ", classSplit, false);
		}
		else // countSplit == 2
		{
			fileLines[0] = newClassName + " :" + classSplit[1];
		}

		if (!SCR_FileIOHelper.WriteFileContent(resourceName.GetPath(), fileLines))
		{
			Print("Cannot write " + resourceName, LogLevel.WARNING);
			return false;
		}

		return true;
	}
}
#endif // WORKBENCH
