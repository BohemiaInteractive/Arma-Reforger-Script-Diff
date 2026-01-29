#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Class Renaming Plugin",
	description: "Rename one or multiple classes in scripts and Prefabs\n[WARNING] will also rename Prefab/Component properties that match criteria",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF0EC)]
class SCR_ClassRenamingPlugin : WorkbenchPlugin
{
	/*
		Renaming
	*/

	[Attribute(defvalue: "1", desc: "Only rename classes that exist in script files and that can be modified\nIf unchecked, replace all words that match criteria", category: "Renaming")]
	protected bool m_bOnlyRenameExistingEditableClasses;

	[Attribute(defvalue: "1", desc: "Only rename words/classes that begin with a capital letter [A-Z]", category: "Renaming")]
	protected bool m_bClassMustStartWithACapitalLetter;

	[Attribute(defvalue: "1", desc: "Process .c files in the provided directories", category: "Renaming")]
	protected bool m_bProcessScriptFiles;

	[Attribute(defvalue: "1", desc: "Process .conf files in the provided directories", category: "Renaming")]
	protected bool m_bProcessConfigFiles;

	[Attribute(defvalue: "1", desc: "Process .et files in the provided directories", category: "Renaming")]
	protected bool m_bProcessPrefabFiles;

	[Attribute(defvalue: "1", desc: "Process .layer files in the provided directories", category: "Renaming")]
	protected bool m_bProcessLayerFiles;

	[Attribute(defvalue: "1", desc: "Go through all the steps without overwriting files", category: "Renaming")]
	protected bool m_bDemoMode;

	[Attribute(desc: "Replacement parameters - the first matching rule applies", category: "Renaming")]
	protected ref array<ref SCR_ClassRenamingParam> m_aParameters;

	/*
		Directories
	*/

	[Attribute(defvalue: "scripts/", category: "Directories")]
	protected ref array<string> m_aScriptDirectories;

	[Attribute(defvalue: "Configs/", category: "Directories")]
	protected ref array<string> m_aConfigDirectories;

	[Attribute(defvalue: "Prefabs/", category: "Directories")]
	protected ref array<string> m_aPrefabDirectories;

	[Attribute(defvalue: "worlds/", category: "Directories")]
	protected ref array<string> m_aLayerDirectories;

	protected int m_iMode = -1;
	protected ref map<string, string> m_mClassesLocation;

	// modes
	protected static const int THIS_FILE = 0;
	protected static const int ALL_FILES = 1;

	// replacement modes
	protected static const int MODE_PMATCH = 0; //!< perfect match "SEARCH"
	protected static const int MODE_PREFIX = 1; //!< prefix match "SEARCHothervalue"
	protected static const int MODE_SUFFIX = 2; //!< suffix match "othervalueSEARCH"
	protected static const int MODE_MIDDLE = 3; //!< middle match "otherSEARCHvalue" - MUST have a start AND an end

	// but still cannot start with number
	protected static const string CLASS_CHARS = SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + SCR_StringHelper.UNDERSCORE;
	protected static const ref array<string> KEYWORDS = {
		"auto", "autoptr", "class", "const", "continue",
		"delete", "else", "event", "extends", "external",
		"for", "foreach", "if", "inout", "modded",
		"native", "new", "notnull", "null", "out",
		"override", "owned", "private", "protected", "proto",
		"ref", "reference", "return", "sealed", "static",
		"super", "switch", "this", "thread", "typedef",
		"vanilla", "volatile", "while",
	};

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		CheckAndSetParameters();

		if (Workbench.ScriptDialog(
			"Class Renaming Plugin",
			"[ PLEASE READ CAREFULLY ]\n\n" +
			"This plugin renames classes in ALL LOADED ADDONS' scripts, config, Prefabs and terrain layers.\n\n" +
			"It will also rename Prefab/Component properties that match criteria,\n" +
			"so beware of same-name properties (e.g \"Damage\" could be both a class and a property).", this) != 1)
			return;

		if (!CheckAndSetParameters())
		{
			Print("Not running due to incorrect parameters - are there rules, and are they enabled?", LogLevel.WARNING);
			return;
		}

		if (!m_bProcessScriptFiles && !m_bProcessConfigFiles && !m_bProcessPrefabFiles && !m_bProcessLayerFiles)
		{
			Print("Not running - please select at least one type of files to process", LogLevel.NORMAL);
			return;
		}

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor) // ...?
		{
			Print("Script Editor is not available", LogLevel.ERROR);
			return;
		}

		array<string> addonFileSystems = SCR_AddonTool.GetAllAddonFileSystems();
		array<string> scriptDirectories = {};
		array<string> configDirectories = {};
		array<string> prefabDirectories = {};
		array<string> layerDirectories = {};
		foreach (string addonFileSystem : addonFileSystems)
		{
			// always grab script files
			foreach (string path : m_aScriptDirectories)
			{
				scriptDirectories.Insert(addonFileSystem + path);
			}

			if (m_bProcessConfigFiles)
			{
				foreach (string path : m_aConfigDirectories)
				{
					configDirectories.Insert(addonFileSystem + path);
				}
			}

			if (m_bProcessPrefabFiles)
			{
				foreach (string path : m_aPrefabDirectories)
				{
					prefabDirectories.Insert(addonFileSystem + path);
				}
			}

			if (m_bProcessLayerFiles)
			{
				foreach (string path : m_aLayerDirectories)
				{
					layerDirectories.Insert(addonFileSystem + path);
				}
			}
		}

		array<string> allScriptFilesAbsolutePaths;
		map<string, string> classNames;
		if (m_iMode == THIS_FILE)
		{
			string filePath;
			if (!scriptEditor.GetCurrentFile(filePath))
			{
				Print("No file opened", LogLevel.WARNING);
				return;
			}

			if (filePath == __FILE__)
			{
				Print("Cannot edit the plugin file itself", LogLevel.NORMAL);
				return;
			}

			if (!Workbench.GetAbsolutePath(filePath, filePath, true))
			{
				Print("This file is not accessible", LogLevel.WARNING);
				return;
			}

			if (m_bOnlyRenameExistingEditableClasses)
				classNames = GetClassesFromFiles({ filePath });
		}
		else
		if (m_iMode == ALL_FILES)
		{
			if (m_bOnlyRenameExistingEditableClasses)
			{
				allScriptFilesAbsolutePaths = GetAllEditableScriptFilesAbsolutePaths(scriptDirectories);
				if (allScriptFilesAbsolutePaths.IsEmpty())
				{
					Print("Cannot find any editable script file", LogLevel.NORMAL);
					return;
				}

				classNames = GetClassesFromFiles(allScriptFilesAbsolutePaths);
			}
		}
		else // should obviously not happen
		{
			Print("Wrong Mode: " + m_iMode, LogLevel.ERROR);
			return;
		}

		array<ref SCR_ClassRenamingParam> fromToParams = GetFromToParams();

		if (fromToParams.IsEmpty())
		{
			Print("No renaming rules provided - quitting", LogLevel.NORMAL);
			return;
		}

		int scriptFilesEdited;
		int configFilesEdited;
		int prefabFilesEdited;
		int layerFilesEdited;
		if (m_bOnlyRenameExistingEditableClasses)
		{
			map<string, string> fromToMap = GetFromToMap(classNames);
			int fromToMapCount = fromToMap.Count();
			if (fromToMapCount < 1)
			{
				Print("No classes found to replace", LogLevel.NORMAL);
				return;
			}

			if (fromToMapCount < 11)
			{
				Print("A total of " + fromToMapCount + " classes were found:", LogLevel.NORMAL);
				foreach (string key, string value : fromToMap)
				{
					Print(key + " => " + value, LogLevel.NORMAL);
				}
			}
			else
			{
				Print("A total of " + fromToMapCount + " classes were found", LogLevel.NORMAL);
			}

			if (m_bProcessScriptFiles)
			{
				if (!allScriptFilesAbsolutePaths) // saves a potential double call
					allScriptFilesAbsolutePaths = GetAllEditableScriptFilesAbsolutePaths(scriptDirectories);

				scriptFilesEdited = RenameInFiles(allScriptFilesAbsolutePaths, null, fromToMap, "Processing %1 Script Files");
			}

			if (m_bProcessConfigFiles)
				configFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(configDirectories, "conf"), null, fromToMap, "Processing %1 Config Files");

			if (m_bProcessPrefabFiles)
				prefabFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(prefabDirectories, "et"), null, fromToMap, "Processing %1 Prefab Files");

			if (m_bProcessLayerFiles)
				layerFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(layerDirectories, "layer"), null, fromToMap, "Processing %1 Layer Files");
		}
		else // rename every word that matches criteria
		{
			if (m_bProcessScriptFiles)
			{
				if (!allScriptFilesAbsolutePaths) // saves a potential double call
					allScriptFilesAbsolutePaths = GetAllEditableScriptFilesAbsolutePaths(scriptDirectories);

				if (allScriptFilesAbsolutePaths.IsEmpty())
				{
					Print("Cannot find any editable script file", LogLevel.NORMAL);
					return;
				}

				scriptFilesEdited = RenameInFiles(allScriptFilesAbsolutePaths, fromToParams, null, "Processing %1 Script Files");
			}

			if (m_bProcessConfigFiles)
				configFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(configDirectories, "conf"), fromToParams, null, "Processing %1 Config Files");

			if (m_bProcessPrefabFiles)
				prefabFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(prefabDirectories, "et"), fromToParams, null, "Processing %1 Prefab Files");

			if (m_bProcessLayerFiles)
				layerFilesEdited = RenameInFiles(GetAllEditableFilesAbsolutePaths(layerDirectories, "layer"), fromToParams, null, "Processing %1 Layer Files");
		}

		string demoPrefix;
		if (m_bDemoMode)
		{
			Print("No changes were done (Demo Mode)", LogLevel.NORMAL);
			demoPrefix = "[DEMO] ";
		}
		else
		if (scriptFilesEdited + configFilesEdited + prefabFilesEdited + layerFilesEdited < 1)
		{
			Print("No changes were done", LogLevel.NORMAL);
			return;
		}

		Print(scriptFilesEdited.ToString() + " script files edited", LogLevel.NORMAL);
		Print(configFilesEdited.ToString() + " config files edited", LogLevel.NORMAL);
		Print(prefabFilesEdited.ToString() + " Prefab files edited", LogLevel.NORMAL);
		Print(layerFilesEdited.ToString() + " layer files edited", LogLevel.NORMAL);

		Workbench.Dialog(
			demoPrefix + "Renaming completed",
			string.Format(
				"%1Renamed all classes:\n%2 in script files.\n%3 in config files.\n%4 in Prefab files.\n%5 in layer files.\n\nYou may have to reopen Workbench to see the changes or to process another Prefab class rename.",
				demoPrefix,
				scriptFilesEdited,
				configFilesEdited,
				prefabFilesEdited,
				layerFilesEdited));
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if parameters are acceptable, false otherwise
	protected bool CheckAndSetParameters()
	{
		bool result = true;

		if (!m_aParameters)
			m_aParameters = {};

		if (m_aParameters.IsEmpty())
		{
			SCR_ClassRenamingParam param = new SCR_ClassRenamingParam();
			param.m_bEnabled = false; // on purpose
			param.m_sFrom = "BIS_*";
			param.m_sTo = "SCR_";
			m_aParameters.Insert(param);
			result = false;
		}
		else
		{
			bool goodParams = false;
			foreach (SCR_ClassRenamingParam param : m_aParameters)
			{
				param.m_sFrom.TrimInPlace();
				param.m_sTo.TrimInPlace();
				if (param.m_bEnabled && param.m_sFrom) // !.IsEmpty()
					goodParams = true;
			}

			if (!goodParams)
				result = false;
		}

		if (!m_aScriptDirectories)
			m_aScriptDirectories = {};

		for (int i = m_aScriptDirectories.Count() - 1; i >= 0; --i)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_aScriptDirectories[i]))
				m_aScriptDirectories.Remove(i);
		}

		if (m_aScriptDirectories.IsEmpty())
			m_aScriptDirectories.Insert("scripts/");

		for (int i = m_aPrefabDirectories.Count() - 1; i >= 0; --i)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_aPrefabDirectories[i]))
				m_aPrefabDirectories.Remove(i);
		}

		if (!m_aPrefabDirectories || m_aPrefabDirectories.IsEmpty())
			m_aPrefabDirectories = { "PrefabLibrary/", "Prefabs/", "PrefabsEditable/" };

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] scriptDirectories
	//! \return all EDITABLE script files' absolute paths
	array<string> GetAllEditableScriptFilesAbsolutePaths(notnull array<string> scriptDirectories)
	{
		array<string> result = {};
		string absolutePath;
		foreach (string scriptDir : scriptDirectories)
		{
			foreach (ResourceName resourceName : SCR_WorkbenchHelper.SearchWorkbenchResources({ "c" }, null, scriptDir, true))
			{

				if (resourceName.GetPath() != __FILE__ && Workbench.GetAbsolutePath(resourceName.GetPath(), absolutePath, true))
					result.Insert(absolutePath);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] directories
	//! \param[in] extension without dot, e.g 'conf'
	//! \return all EDITABLE config files' absolute paths
	protected array<string> GetAllEditableFilesAbsolutePaths(notnull array<string> directories, string extension)
	{
		array<string> result = {};
		string absolutePath;
		foreach (string directory : directories)
		{
			foreach (ResourceName resourceName : SCR_WorkbenchHelper.SearchWorkbenchResources({ extension }, null, directory, true))
			{
				if (Workbench.GetAbsolutePath(resourceName.GetPath(), absolutePath, true))
					result.Insert(absolutePath);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get classes from the provided script files
	//! \param[in] absoluteScriptFilePaths
	//! \return a className -> absoluteFilePath map - never returns null
	protected map<string, string> GetClassesFromFiles(notnull array<string> absoluteScriptFilePaths)
	{
		map<string, string> result = new map<string, string>();
		foreach (ResourceName absoluteScriptFilePath : absoluteScriptFilePaths)
		{
			foreach (string fileClass : GetClassesFromFile(absoluteScriptFilePath))
			{
				result.Insert(fileClass, absoluteScriptFilePath);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] absoluteFilePath the absolute path of the file to scan for classes
	//! \return an array of found classes or null on error
	protected array<string> GetClassesFromFile(string absoluteFilePath)
	{
		array<string> lines = SCR_FileIOHelper.ReadFileContent(absoluteFilePath);
		if (!lines)
			return null;

		array<string> result = {};
		foreach (string line : lines)
		{
			int length = line.Length();
			if (length < 7) // "class " + 1, "class A" is valid
				continue;

			if (!line.StartsWith("class ") && !line.StartsWith("class\t"))
				continue;

			string c;
			string className;
			for (int i = 6; i < length; i++)
			{
				c = line[i];
				if (c == " " || c == "\t" || c == ":" || c == "/") // a bit raw but it will work
					break;

				className += c;
			}

			if (className) // !.IsEmpty()
			{
				if (!m_bClassMustStartWithACapitalLetter || SCR_StringHelper.UPPERCASE.Contains(className[0]))
					result.Insert(className);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] classFiles className->absoluteFilePath map - only class name is used
	//! \return the conversion map, fromClassName -> toClassName - never returns null
	array<ref SCR_ClassRenamingParam> GetFromToParams()
	{
		array<ref SCR_ClassRenamingParam> result = {};

		SCR_ClassRenamingParam newParam;
		foreach (SCR_ClassRenamingParam param : m_aParameters)
		{
			if (!param.m_bEnabled)
				continue;

			newParam = new SCR_ClassRenamingParam();
			// newParam.m_bEnabled = true; // not needed

			bool startsWithStar = param.m_sFrom.StartsWith(SCR_StringHelper.STAR);
			bool endsWithStar = param.m_sFrom.EndsWith(SCR_StringHelper.STAR);

			if (startsWithStar && endsWithStar)
				newParam.m_iMode = MODE_MIDDLE;
			else if (startsWithStar)
				newParam.m_iMode = MODE_SUFFIX;
			else if (endsWithStar)
				newParam.m_iMode = MODE_PREFIX;
			else
				newParam.m_iMode = MODE_PMATCH;

			newParam.m_sFrom = param.m_sFrom;
			//newParam.m_sFrom.Replace(SCR_StringHelper.STAR, string.Empty); // bug for object properties, using temp var to circumvent it
			string tmp = param.m_sFrom;
			tmp.Replace(SCR_StringHelper.STAR, string.Empty);
			newParam.m_sFrom = tmp;
			newParam.m_sFrom.TrimInPlace();

			if (newParam.m_sFrom) // !.IsEmpty()
			{
				newParam.m_sTo = param.m_sTo;
				newParam.m_sTo.TrimInPlace();
				if (newParam.m_sTo || newParam.m_iMode != MODE_PMATCH) // prevent exact match to erase the class
					result.Insert(newParam);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] classFiles className->absoluteFilePath map - only class name is used
	//! \return the conversion map, fromClassName->toClassName - never returns null
	map<string, string> GetFromToMap(notnull map<string, string> classFiles)
	{
		array<ref SCR_ClassRenamingParam> newParams = GetFromToParams();

		if (newParams.IsEmpty())
			return new map<string, string>();

		map<string, string> result = new map<string, string>();

		foreach (string className, string fp : classFiles)
		{
			foreach (SCR_ClassRenamingParam param : newParams)
			{
				if (param.m_iMode == MODE_PMATCH)
				{
					if (className == param.m_sFrom)
					{
						result.Insert(className, param.m_sTo);
						break;
					}
				}
				else
				if (param.m_iMode == MODE_PREFIX)
				{
					if (className.StartsWith(param.m_sFrom) && !className.EndsWith(param.m_sFrom))
					{
						string newClassName = param.m_sTo + className.Substring(param.m_sFrom.Length(), className.Length() - param.m_sFrom.Length());
						result.Insert(className, newClassName);
						break;
					}
				}
				else
				if (param.m_iMode == MODE_SUFFIX)
				{
					if (className.EndsWith(param.m_sFrom) && !className.StartsWith(param.m_sFrom))
					{
						string newClassName = className.Substring(0, className.LastIndexOf(param.m_sFrom)) + param.m_sTo;
						result.Insert(className, newClassName);
						break;
					}
				}
				else
				if (param.m_iMode == MODE_MIDDLE)
				{
					if (className.Contains(param.m_sFrom) &&
						!className.StartsWith(param.m_sFrom) &&
						!className.EndsWith(param.m_sFrom))
					{
						int index = className.IndexOf(param.m_sFrom);
						string newClassName = className.Substring(0, index) + param.m_sTo + className.Substring(index + param.m_sFrom.Length(), className.Length() - index + param.m_sFrom.Length());
						result.Insert(className, newClassName);
						break;
					}
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether the classes exist or not in script, rename them
	//! \param[in] absolutePaths
	//! \param[in] fromToParams
	//! \param[in] fromToMap
	//! \param[in] progressBarText
	//! \return number of replacements
	int RenameInFiles(notnull array<string> absolutePaths, array<ref SCR_ClassRenamingParam> fromToParams, map<string, string> fromToMap, string progressBarText = "Processing %1 files")
	{
		bool isParams = fromToParams != null;
		if (!isParams && !fromToMap)
		{
			Print("RenameInFiles scripting error - provide fromTo info", LogLevel.ERROR);
			return 0;
		}

		float countF = absolutePaths.Count();
		if (countF < 1)
			return 0;

		int result;
		array<string> lines;
		WBModuleDef workbenchModule = Workbench.GetModule(ScriptEditor);
		progressBarText = string.Format(progressBarText, countF);

		Print("" + progressBarText, LogLevel.NORMAL);
		WBProgressDialog progressDialog = new WBProgressDialog(progressBarText, workbenchModule);
		progressDialog.SetProgress(0);
		float currProgress, prevProgress;
		foreach (int i, string scriptFile : absolutePaths)
		{
			currProgress = i / countF;
			if (currProgress - prevProgress >= 0.01)		// min 1%
			{
				progressDialog.SetProgress(currProgress);	// expensive
				Sleep(1);
				prevProgress = currProgress;
			}

			lines = SCR_FileIOHelper.ReadFileContent(scriptFile, false);
			if (!lines)
			{
				Print("Cannot read " + scriptFile, LogLevel.WARNING);
				continue;
			}

			int fileResult;
			if (isParams)
				fileResult = RenameInLines(lines, fromToParams, null);
			else
				fileResult = RenameInLines(lines, null, fromToMap);

			if (fileResult > 0)
			{
				if (lines[lines.Count() - 1].Trim() != string.Empty)
					lines.Insert(string.Empty); // ensure the last line return

				if (m_bDemoMode || SCR_FileIOHelper.WriteFileContent(scriptFile, lines))
					result++;
				else
					Print("Cannot write " + scriptFile, LogLevel.WARNING);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] fromToParams
	//! \param[in] lines
	//! \return
	protected int RenameInLines(out notnull array<string> lines, array<ref SCR_ClassRenamingParam> fromToParams, map<string, string> fromToMap)
	{
		bool isParams = fromToParams != null;
		if (!isParams && !fromToMap)
		{
			Print("RenameInLines scripting error - provide fromTo info", LogLevel.ERROR);
			return 0;
		}

		int result;

		foreach (int i, string line : lines)
		{
			string word;
			string newLine;
			for (int j, countJ = line.Length(); j < countJ; ++j)
			{
				string c = line[j];
				bool isClassChar = CLASS_CHARS.Contains(c);
				bool isLastChar = j == countJ - 1;
				if (word)
				{
					if (CLASS_CHARS.Contains(c))
					{
						word += c;
					}
					else // word end - let's deal with it
					{
						if (KEYWORDS.Contains(word))
						{
							newLine += word + c;
							word = string.Empty;
							continue;
						}

						string newClassName;
						if (isParams)
							newClassName = GetNewClassNameIfMatch(word, fromToParams);
						else
							newClassName = fromToMap.Get(word);

						if (newClassName)
						{
							newLine += newClassName + c;
							result++;
						}
						else
						{
							newLine += word + c;
						}

						word = string.Empty;
					}
				}
				else // not in a word
				{
					if (isClassChar && !SCR_StringHelper.DIGITS.Contains(c)) // class start
						word = c;
					else
						newLine += c;
				}
			}

			// I have the final word
			if (word)
			{
				string newClassName;
				if (isParams)
					newClassName = GetNewClassNameIfMatch(word, fromToParams);
				else
					newClassName = fromToMap.Get(word);

				if (newClassName)
					newLine += newClassName;
				else
					newLine += word;
			}

			if (line != newLine)
				lines[i] = newLine;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] className
	//! \param[in] fromToParams
	//! \return empty string if no matching param was found
	string GetNewClassNameIfMatch(string className, notnull array<ref SCR_ClassRenamingParam> fromToParams)
	{
		string result;
		foreach (SCR_ClassRenamingParam param : fromToParams)
		{
			if (param.m_iMode == MODE_PMATCH)
			{
				if (className == param.m_sFrom)
				{
					result = param.m_sTo;
					break;
				}
			}
			else
			if (param.m_iMode == MODE_PREFIX)
			{
				if (className.StartsWith(param.m_sFrom) && !className.EndsWith(param.m_sFrom))
				{
					result = param.m_sTo + className.Substring(param.m_sFrom.Length(), className.Length() - param.m_sFrom.Length());
					break;
				}
			}
			else
			if (param.m_iMode == MODE_SUFFIX)
			{
				if (className.EndsWith(param.m_sFrom) && !className.StartsWith(param.m_sFrom))
				{
					result = className.Substring(0, className.LastIndexOf(param.m_sFrom)) + param.m_sTo;
					break;
				}
			}
			else
			if (param.m_iMode == MODE_MIDDLE)
			{
				if (className.Contains(param.m_sFrom) &&
					!className.StartsWith(param.m_sFrom) &&
					!className.EndsWith(param.m_sFrom))
				{
					int index = className.IndexOf(param.m_sFrom);
					result = className.Substring(0, index) + param.m_sTo + className.Substring(index + param.m_sFrom.Length(), className.Length() - index + param.m_sFrom.Length());
					break;
				}
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	[ButtonAttribute("All classes")]
	int ButtonRunAll()
	{
		m_iMode = ALL_FILES;
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	[ButtonAttribute("Curr. file classes", true)]
	int ButtonRun()
	{
		m_iMode = THIS_FILE;
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	[ButtonAttribute("Cancel")]
	int ButtonCancel()
	{
		return 0;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleFields({ "m_sFrom", "m_sTo" }, "%1 to %2")]
class SCR_ClassRenamingParam
{
	[Attribute(defvalue: "1", desc: "Allow this replacement to happen")]
	bool m_bEnabled;

	[Attribute(desc: "What class name part to rename (case-sensitive). '*' works as a (limited) wildcard:\n- abc = exact match\n- abc* = prefix\n- *abc = suffix\n- *abc* = middle (needs a before AND an after)")]
	string m_sFrom;

	[Attribute(desc: "With what to replace")]
	string m_sTo;

	int m_iMode; // shh, it's a sikrit variable
}
#endif
