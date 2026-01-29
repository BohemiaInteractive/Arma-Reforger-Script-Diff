#ifdef WORKBENCH

//! \brief A Basic Code Formatter - use Ctrl+Shift+K to trigger.
//! Ctrl+Alt+Shift+K can be used to force processing all the lines of the currently opened file.
//! \see SCR_BasicCodeFormatterForcedPlugin

// ###############
// It is recommended to be VERY CAREFUL with breakpoints here
// as GetCurrentFile, GetLinesCount, GetLineText, SETLineText etc may target this file
// Once a breakpoint is hit (without FileIO), switch back to the edited file and press F5
// ###############

// TODO: ref/notnull method (?? e.g ref notnull ScriptInvoker GetOnHealthChanged(bool createNew = true))
// TODO: detect methods without separator (beware of comments / doxygen comments)
// TODO: detect unused private/protected class variables / methods (method variables... later.)
// TODO: detect pointer declaration in loops (for/foreach/while)
// TODO: detect missing empty line after an if two-liner (or if-else)

[WorkbenchPluginAttribute(
	name: "Basic Code Formatter",
	shortcut: "Ctrl+Shift+K",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF036)]
class SCR_BasicCodeFormatterPlugin : WorkbenchPlugin
{
	/*
		Category: Formatting
	*/

	[Attribute(defvalue: "1", desc: "Fix e.g 'for( int' → 'for (int', ';;' → ';', ') ;' → ');', 'NULL' → 'null', ',' → ', ', etc.", category: "Formatting")]
	protected bool m_bGeneralFormatting;

	[Attribute(defvalue: "1", desc: "Remove empty spaces at the end of code lines", category: "Formatting")]
	protected bool m_bTrimLineEnds;

	[Attribute(defvalue: "1", desc: "Replace four-spaces tabs with the tab character and remove spaces mixed with tabs", category: "Formatting")]
	protected bool m_bFixTabs;

	[Attribute(defvalue: "1", desc: "Fix method separators (" + SCR_StringHelper.DOUBLE_SLASH + "---)", category: "Formatting")]
	protected bool m_bFixMethodSeparators;

//	[Attribute(defvalue: "1", desc: "Replace consecutive empty lines with only one", category: "Formatting")]
//	protected bool m_bRemoveMultiEmptyLines;

	[Attribute(defvalue: "1", desc: "Add one final line return if the file is missing one", category: "Formatting")]
	protected bool m_bAddFinalLineReturn;

	[Attribute(desc: "Accepted class/enum prefixes (e.g 'SCR_', 'TAG_', etc) - an underscore is automatically added if missing. 'SCR_' is automatically whitelisted.\nCase-sensitive", category: "Formatting")]
	protected ref array<string> m_aAcceptedScriptPrefixes;

	/*
		Category: Batch Process
	*/

	[Attribute(defvalue: "0", desc: "Process all script files in the addon defined below (only process the currently opened file if unchecked)", category: "Batch Process")]
	protected bool m_bBatchProcessAddon;

	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Addon in which script files will be batch-formatted (if \"Batch Process Addon\" above is selected)", enums: SCR_ParamEnumArray.FromAddons(titleFormat: 2, hideCoreModules: 2), category: "Batch Process")]
	protected int m_iAddon;

	[Attribute(defvalue: "1", desc: "Only log reports where processing did something to the file", category: "Batch Process")]
	protected bool m_bOnlyLogChanges;

	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Max number of formatting reports to display - use -1 to display all reports", params: "-1 100 1", category: "Batch Process")]
	protected int m_iMaxLoggedReports;

	/*
		Category: Options
	*/

	[Attribute(defvalue: "0", desc: "Demo mode only logs (possible) fixes and does not modify the file at all", category: "Options")]
	protected bool m_bDemoMode;

	[Attribute(defvalue: "1", desc: "Display a dialog on execution if unchecked - dialog is always displayed when batch-processing is enabled", category: "Options")]
	protected bool m_bSilentExecution;

	[Attribute(defvalue: "1", desc: "Only format DiffCommand-detected modified lines instead of all of them", category: "Options")]
	protected bool m_bOnlyFormatModifiedLines;

	[Attribute(defvalue: "1", desc: "Write fixes in console log", category: "Options")]
	protected bool m_bLogFixes;

	[Attribute(defvalue: "1", desc: "Show potential improvements report", category: "Options")]
	protected bool m_bReportFindings;

	[Attribute(defvalue: "scripts/xxx/generated/", desc: "Directories in which to avoid formatting (case-insensitive, no wildcards)", category: "Options")]
	protected ref array<string> m_aExcludedDirectories;

	[Attribute(desc: "Spell check config - set it to null to reset its default values", category: "Options")]
	protected ref SCR_BasicCodeFormatterSpellCheckConfig m_SpellCheckConfig;

	[Attribute(defvalue: "1", desc: "Whether or not display flagged words details in the report", category: "Options")]
	protected bool m_bShowSpellCheckFindingsDetails;

	/*
		Category: Advanced
	*/

	[Attribute(defvalue: DEFAULT_DIFF_CMD, desc: "The command line used to generate the diff file\n%1 = absolute target filepath\n%2 = absolute destination filepath", category: "Advanced")]
	protected string m_sDiffCommand;

	// member variables

	protected ref array<ref array<string>> m_aGeneralFormatting_Start;
	protected ref array<ref array<string>> m_aGeneralFormatting_Middle;
	protected ref array<ref array<string>> m_aGeneralFormatting_End;

	protected ref array<string> m_aForbiddenDivisions;
	protected ref array<string> m_aPrefixLineChecks;			//!< what should be followed by a TAG_ prefix
	protected ref array<string> m_aPrefixChecks;

	protected ref array<string> m_aForbiddenDirectories;

	protected ref array<string> m_aForForEachWhileArray;
	protected ref array<string> m_aIfForForEachWhileArray;
	protected ref array<string> m_aEndBracketSemicolonArray;
	protected ref array<string> m_aNewArrayNewRefArray;
	protected ref array<string> m_aScriptInvokerArray;

	protected ref map<string, string> m_mVariableTypePrefixes;
	protected ref map<string, string> m_mVariableTypePrefixesStart;
	protected ref map<string, string> m_mVariableTypePrefixesEnd;

	//! mistake-correction map; mistake can contain star(s)
	protected ref map<string, string> m_mForbiddenWords;		// detection only, no replacement due to casing

	// constants

	protected static const int LINE_NUMBER_LIMIT = 12;			//!< used by JoinLineNumbers to limit the amount of shown line number groups
	protected static const string LINE_NUMBER_RANGE = "%1-%2";	//!< used by JoinLineNumbers to give a line range (e.g 5-17, 2001-2013, etc)

	protected static const string LOG_SEPARATOR = "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -";
	protected static const string BRACKET_OPEN = "{";			// avoids {} Script Editor indent shenanigans
	protected static const string BRACKET_CLOSE = "}";
	protected static const string MEMBER_PREFIX = "m_";
	protected static const string STATIC_PREFIX = "s_";

	// can be string-hardcoded without too much risks
	protected static const ref array<string> NATIVE_TYPES = {
		"bool", "float", "int", "string", "typename", "vector",
		"FactionKey", "LocalizedString", "ResourceName",
	};

	// "varName' '= 42", "varName, otherVarName", "varName= 42", "varName;" - tabs are replaced by spaces in HasBadVariableNaming
	protected static const ref array<string> VARIABLE_NAME_ENDING = { SCR_StringHelper.SPACE, ",", "=", ";", SCR_StringHelper.SLASH }; // technically, "everything but [a-zA-Z0-9_] and []"

	protected static const string METHOD_SEPARATOR = SCR_StringHelper.DOUBLE_SLASH + "------------------------------------------------------------------------------------------------";
	protected static const string DIFF_FILENAME = "tempDiffFile.txt";

	// TODO: start /B svn diff "%1" > "%2" + delay? ResourceManager.WaitForFile?
	protected static const string DEFAULT_DIFF_CMD = "cmd /c svn diff \"%1\" > \"%2\""; // %1 = absolute target filepath, %2 = absolute destination filepath
	protected static const ref array<string> FORMAT_IGNORE = { SCR_StringHelper.DOUBLE_SLASH, SCR_StringHelper.SLASH + "*", SCR_StringHelper.DOUBLE_QUOTE };
	protected static const ref array<string> FORCED_PREFIXES = { "SCR_" };
	protected static const ref array<string> EXCLUDED_DIRECTORIES = {
		"scripts/Core/generated/",
		"scripts/GameLib/generated/",
		"scripts/Game/generated/",
		"scripts/WorkbenchGameCommon/generated/",
	};
	protected static const string GENERATED_SCRIPT_WARNING = "Do not modify, this script is generated"; // must be the exact line, tabs included if any
	protected static const ResourceName SPELLCHECK_CONFIG = "{53D7DE332A43449F}Configs/Workbench/ScriptEditor/BasicCodeFormatterPlugin/BasicCodeFormatterSpellCheckConfig.conf";

	//------------------------------------------------------------------------------------------------
	//! Running method
	protected override void Run()
	{
		if (m_bBatchProcessAddon || !m_bSilentExecution)
		{
			if (!Workbench.ScriptDialog("Configure 'Basic Code formatter' plugin", "Configure formatting options", this))
				return;
		}

		Initialise();

		if (m_bBatchProcessAddon)
			RunAddonFilesBatchProcess();
		else
			RunCurrentFile();
	}

	//------------------------------------------------------------------------------------------------
	//! Run the code formatter forced (formats all lines of the currently-opened file)
	void RunForced()
	{
		bool batchProcessAddon = m_bBatchProcessAddon;
		bool demoMode = m_bDemoMode;
		bool onlyFormatModifiedLines = m_bOnlyFormatModifiedLines;

		m_bBatchProcessAddon = false;
		m_bOnlyFormatModifiedLines = false;
		m_bDemoMode = false;

		Run();

		m_bBatchProcessAddon = batchProcessAddon;
		m_bDemoMode = demoMode;
		m_bOnlyFormatModifiedLines = onlyFormatModifiedLines;
	}

	//------------------------------------------------------------------------------------------------
	//! Required initialisation (prefixes filter)
	protected void Initialise()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
		{
			Print("Script Editor API is not available", LogLevel.ERROR);
			return;
		}

		if (m_aPrefixChecks)
			m_aPrefixChecks.Clear();
		else
			m_aPrefixChecks = {};

		for (int i = m_aAcceptedScriptPrefixes.Count() - 1; i >= 0; i--)
		{
			string prefix = m_aAcceptedScriptPrefixes[i].Trim();
			if (!prefix) // !IsEmpty for perf
			{
				m_aAcceptedScriptPrefixes.RemoveOrdered(i);
				continue;
			}

			if (!prefix.EndsWith("_"))
				m_aAcceptedScriptPrefixes[i] = prefix + "_";
		}

		array<string> toRemove = {};
		foreach (string forcedPrefix : FORCED_PREFIXES)
		{
			if (!m_aAcceptedScriptPrefixes.Contains(forcedPrefix))
			{
				m_aAcceptedScriptPrefixes.Insert(forcedPrefix);
				toRemove.Insert(forcedPrefix);
			}
		}

		foreach (string prefix : m_aAcceptedScriptPrefixes)
		{
			foreach (string prefixLineCheck : m_aPrefixLineChecks)
			{
				m_aPrefixChecks.Insert(prefixLineCheck + prefix);
			}
		}

		foreach (string forcedPrefix : toRemove)
		{
			m_aAcceptedScriptPrefixes.RemoveItem(forcedPrefix);
		}

		// excluded directories
		m_aForbiddenDirectories = {};
		m_aForbiddenDirectories.Copy(EXCLUDED_DIRECTORIES);
		foreach (int i, string excludedDirectory : m_aExcludedDirectories)
		{
			if (!excludedDirectory)
				continue;

			if (!excludedDirectory.EndsWith(SCR_StringHelper.SLASH))			// add final slash
				excludedDirectory += SCR_StringHelper.SLASH;

			m_aExcludedDirectories[i] = excludedDirectory;	// update user setting

			excludedDirectory.ToLower();
			m_aForbiddenDirectories.Insert(excludedDirectory);
		}

		if (m_mForbiddenWords)
			m_mForbiddenWords.Clear();
		else
			m_mForbiddenWords = new map<string, string>();

		if (!m_SpellCheckConfig)
			m_SpellCheckConfig = SCR_ConfigHelperT<SCR_BasicCodeFormatterSpellCheckConfig>.GetConfigObject(SPELLCHECK_CONFIG);

		if (m_SpellCheckConfig && m_SpellCheckConfig.m_aEntries)
		{
			foreach (SCR_BasicCodeFormatterSpellCheckConfig_ForbiddenWordEntry entry : m_SpellCheckConfig.m_aEntries)
			{
				if (!entry.m_bEnabled)
					continue;

				string mistake = SCR_StringHelper.Filter(entry.m_sMistake, SCR_StringHelper.ALPHANUMERICAL + SCR_StringHelper.STAR);
				if (!mistake) // .IsEmpty()
					continue;

				// case-insensitive
				mistake.ToLower();

				if (m_mForbiddenWords.Contains(mistake))
					continue;

				string correction = entry.m_sCorrection;
				correction.TrimInPlace();
				if (correction)
					correction = string.Format("use \"%1\" instead", correction);

				string comment = entry.m_sComment;
				comment.TrimInPlace();

				if (comment) // !.IsEmpty()
				{
					if (correction) // !.IsEmpty()
						correction = string.Format("%1 (%2)", correction, comment);
					else
						correction = comment;
				}

				m_mForbiddenWords.Insert(mistake, correction);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Run the current file
	protected void RunCurrentFile()
	{
		string currentFile;
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		scriptEditor.GetCurrentFile(currentFile);
		SCR_BasicCodeFormatterPluginFileReport report = ProcessFile(currentFile, false);
		if (report)
			PrintReport(report, m_bLogFixes, m_bReportFindings); // always print current file's report
	}

	//------------------------------------------------------------------------------------------------
	//! Show dialogs to treat all addon script files
	protected void RunAddonFilesBatchProcess()
	{
		string absoluteAddonDirectory;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddon, string.Empty, absoluteAddonDirectory, true))
		{
			Print("Wrong addon selected: " + SCR_AddonTool.GetAddonID(m_iAddon) + " (read-only?)", LogLevel.ERROR);
			return;
		}

		string addonName = SCR_AddonTool.GetAddonID(m_iAddon);
		string addonFileSystem = SCR_AddonTool.ToFileSystem(addonName);
		int addonFileSystemLength = addonFileSystem.Length();
		int addonFileSystemLengthPlus3 = addonFileSystemLength + 3; // e.g "$FS:a.c"
		array<string> scriptFiles = SCR_WorkbenchHelper.SearchWorkbenchFiles({ "c" }, null, addonFileSystem);

		array<string> relativeFilePaths = {};
		foreach (string scriptFile : scriptFiles)
		{
			if (!scriptFile) // .IsEmpty()
				continue;

			int scriptFileLength = scriptFile.Length();
			if (scriptFileLength < addonFileSystemLengthPlus3) // .IsEmpty()
				continue;

			relativeFilePaths.Insert(scriptFile.Substring(addonFileSystemLength, scriptFileLength - scriptFileLength));
		}

		int editableScriptsCount = relativeFilePaths.Count();
		if (editableScriptsCount < 1)
		{
			Print("No script files found in " + addonName + " - leaving", LogLevel.WARNING);
			return;
		}

		if (!Workbench.ScriptDialog(
				"Warning",
				"You are about to format " + editableScriptsCount + " \"" + addonName + "\" addon script files.\n\n" + "Continue?",
				new SCR_OKCancelWorkbenchDialog())
		)
			return;

		array<ref SCR_BasicCodeFormatterPluginFileReport> reports = ProcessFiles(relativeFilePaths, true);

		if (m_iMaxLoggedReports < 1)
			return;

		int displayedReports;
		foreach (SCR_BasicCodeFormatterPluginFileReport report : reports)
		{
			if (!report)
				continue;

			if (m_bOnlyLogChanges && report.IsClean())
				continue;

			PrintReport(report, m_bLogFixes, m_bReportFindings);

			displayedReports++;
			if (m_iMaxLoggedReports > 0 && displayedReports >= m_iMaxLoggedReports)
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Process multiple files (using ProcessFile)
	//! \param[in] relativeFilePaths multiple relative file paths
	//! \param[in] useFileIO true = use FileIO's API (no Ctrl+Z available, but does not open a Script Editor tab),\n
	//! false = use ScriptEditor API method (opens a tab, allows for Ctrl+Z)
	//! \return array of reports (in the order of relativeFilePaths)
	protected array<ref SCR_BasicCodeFormatterPluginFileReport> ProcessFiles(array<string> relativeFilePaths, bool useFileIO)
	{
		string currentFile;
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		scriptEditor.GetCurrentFile(currentFile);

		SCR_BasicCodeFormatterPluginFileReport report;
		array<ref SCR_BasicCodeFormatterPluginFileReport> result = {};
		foreach (string relativeFilePath : relativeFilePaths)
		{
			report = ProcessFile(relativeFilePath, useFileIO && relativeFilePath != currentFile); // do NOT use FileIO on the current file - the Script Editor may lose edits track
			result.Insert(report);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Process a single file
	//! \param[in] relativeFilePath the file's relative path
	//! \param[in] useFileIO true = use FileIO's API (no Ctrl+Z available, but does not open a Script Editor tab),\n
	//! false = use ScriptEditor API method (opens a tab, allows for Ctrl+Z)
	//! \return provided file's report, null on error
	protected SCR_BasicCodeFormatterPluginFileReport ProcessFile(string relativeFilePath, bool useFileIO)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(relativeFilePath))
		{
			Print("Provided relative file path is empty", LogLevel.WARNING);
			return null;
		}

		foreach (string forbiddenDir : m_aForbiddenDirectories)
		{
			if (relativeFilePath.StartsWith(forbiddenDir))
			{
				PrintFormat("File is in excluded directory, skipping (%1) - %2", forbiddenDir, relativeFilePath, level: LogLevel.NORMAL);
				return null;
			}
		}

		bool isReadOnly = false;
		bool demoMode = m_bDemoMode;

		// checking the file is modifiable
		string absoluteFilePath;
		if (!Workbench.GetAbsolutePath(relativeFilePath, absoluteFilePath, true))
		{
			Print("Cannot find the absolute file path, switching to readonly mode - is file read-only? " + relativeFilePath, LogLevel.WARNING);
			demoMode = true;
			isReadOnly = true;
		}

		SCR_BasicCodeFormatterPluginFileReport report = new SCR_BasicCodeFormatterPluginFileReport(relativeFilePath, relativeFilePath == __FILE__);

		bool reportFindings = !report.m_bIsPluginFile && m_bReportFindings;
		bool doGeneralFormatting = !report.m_bIsPluginFile && m_bGeneralFormatting;

		bool isPreviousLineEmpty;
		bool emptyLineGroupLogged;

		bool isInRepository;

		if (m_bOnlyFormatModifiedLines && !isReadOnly)
		{
			report.m_iDiffTime = System.GetTickCount();
			report.m_aDiffLines = GetFileModifiedLineNumbers(absoluteFilePath, isInRepository);
			report.m_iDiffTime = System.GetTickCount(report.m_iDiffTime);

			if (report.m_aDiffLines && report.m_aDiffLines.IsEmpty()) // no changes found
			{
				if (isInRepository)	// an unchanged committed file must not be formatted
					doGeneralFormatting = false;
				else				// a non-committed file gets fully checked
					report.m_aDiffLines = null;
			}
		}

		array<string> pieces; // instanciation not needed (see GetIndentAndLineContentAsPieces)
		array<string> friendlyWords = {};
		/*
			Start
		*/
		const int startTick = System.GetTickCount();

		array<string> lines = ReadFileContent(relativeFilePath, useFileIO);
		report.m_iReadTime = System.GetTickCount(startTick);

		if (lines.Contains(GENERATED_SCRIPT_WARNING))
		{
			Print("Skipping generated script " + relativeFilePath, LogLevel.NORMAL);
			return null;
		}

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		int linesCount = lines.Count(); // get array count instead of incrementing a variable
		report.m_iLinesTotal = linesCount;

		bool prevFormatThisLine;
		string prevFullLine;
		string prevContent;
		foreach (int lineNumber, string fullLine : lines)
		{
			int lineNumberPlus1 = lineNumber + 1;
			bool formatThisLine = !report.m_aDiffLines || report.m_aDiffLines.Contains(lineNumberPlus1); // null = format everything
			if (!formatThisLine && !reportFindings)
			{
				prevFormatThisLine = formatThisLine; // false
				prevFullLine = fullLine;
				prevContent = fullLine.Trim();
				continue;
			}

			string indentation;
			GetIndentAndLineContentAsPieces(fullLine, indentation, pieces);
			int piecesCount = pieces.Count();

			if (formatThisLine)
			{
				if (m_bTrimLineEnds)
				{
					int endSpacesRemoved;
					if (piecesCount < 1)
					{
						endSpacesRemoved = indentation.Length();
						indentation = string.Empty;
					}
					else
					{
						string endPiece = pieces[piecesCount - 1];
						endPiece = SCR_StringHelper.TrimRight(endPiece);
						endSpacesRemoved = pieces[piecesCount - 1].Length() - endPiece.Length();
						if (endSpacesRemoved > 0)
							pieces.Set(piecesCount - 1, endPiece);
					}

					if (endSpacesRemoved > 0)
					{
						report.m_iEndSpacesRemovedTotal += endSpacesRemoved;
						report.m_aTrimmings.Insert(lineNumberPlus1);
					}
				}

				if (m_bFixTabs)
				{
					int fourSpacesReplaced = indentation.Replace(SCR_StringHelper.QUADRUPLE_SPACE, SCR_StringHelper.TAB);
					int spaceInTabsReplaced = indentation.Replace(SCR_StringHelper.SPACE, string.Empty);

					if (fourSpacesReplaced > 0 || spaceInTabsReplaced > 0)
					{
						report.m_iFourSpacesReplacedTotal += fourSpacesReplaced;
						report.m_iSpaceInTabsReplacedTotal += spaceInTabsReplaced;
						report.m_aFixedIndentations.Insert(lineNumberPlus1);
					}
				}

				if (m_bFixMethodSeparators)
				{
					if (piecesCount == 1 && pieces[0].StartsWith(SCR_StringHelper.DOUBLE_SLASH + "---") && pieces[0].EndsWith("---") && !SCR_StringHelper.Filter(pieces[0], "/-", true) && pieces[0] != METHOD_SEPARATOR)
					{
						pieces.Set(0, METHOD_SEPARATOR);
						report.m_iMethodSeparatorFixedTotal++;
					}
				}

				if (doGeneralFormatting)
					report.m_iGeneralFormattingTotal += GeneralFormatting(indentation, pieces);
			}

			string currContent = SCR_StringHelper.Join(string.Empty, pieces);
			if (reportFindings)
			{
				if (indentation != SCR_StringHelper.TAB && currContent.StartsWith(SCR_StringHelper.DOUBLE_SLASH + "---") && currContent.EndsWith("---"))
					report.m_aBadSeparatorFound.Insert(lineNumberPlus1);

				string fullLineLC = SCR_StringHelper.FormatValueNameToUserFriendly(fullLine);
				fullLineLC.ToLower();
				fullLineLC.Split(SCR_StringHelper.SPACE, friendlyWords, true);
				foreach (string friendlyWord : friendlyWords) // all lowercase
				{
					foreach (string forbiddenWord, string correction : m_mForbiddenWords)
					{
						if (SCR_StringHelper.SimpleStarSearchMatches(friendlyWord, forbiddenWord, false, false))
						{
							if (report.m_mForbiddenWordFound.Contains(forbiddenWord))
								report.m_mForbiddenWordFound.Get(forbiddenWord).Insert(lineNumberPlus1);
							else
								report.m_mForbiddenWordFound.Insert(forbiddenWord, { lineNumberPlus1 });

							break; // one match per word is enough
						}
					}
				}

				string findingsString = currContent.Trim();
				bool isCurrentLineEmpty = !findingsString; // !IsEmpty for perf
				if (isPreviousLineEmpty)
				{
					if (isCurrentLineEmpty)
					{
						if (!emptyLineGroupLogged)
						{
							report.m_aDoubleEmptyLineFound.Insert(lineNumber); // previous line number
							emptyLineGroupLogged = true;
						}
					}
					else
					{
						emptyLineGroupLogged = false;
					}
				}
				else // let's check for scope opening/endings
				{
					if (isCurrentLineEmpty)
					{
						if (prevContent.StartsWith("{")) // }
							report.m_aSuperfluousEmptyLineFound.Insert(lineNumberPlus1);
					}
					else
					{ // { {
						if (prevFullLine.StartsWith("\t\t") && prevContent.StartsWith("}") && !currContent.StartsWith("}") && !currContent.StartsWith("else"))
							report.m_aMissingEmptyLineFound.Insert(lineNumberPlus1);
					}
				}

				findingsString = string.Empty;
				array<string> splits = {};
				foreach (string piece : pieces)
				{
					if (piece.StartsWith(SCR_StringHelper.DOUBLE_SLASH))
					{
						findingsString = SCR_StringHelper.TrimRight(findingsString);
						break; // cheaper
					}

					if (!SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE)) // don't check comments
						findingsString += piece;
				}

				if (findingsString) // !IsEmpty for perf
				{
					if (findingsString.Contains(" = new ") && !findingsString.Contains("(")) // )
						report.m_aNewWithoutParentheseFound.Insert(lineNumberPlus1);

					if (SCR_StringHelper.ContainsAny(findingsString, m_aNewArrayNewRefArray))
						report.m_aNewArrayFound.Insert(lineNumberPlus1);

					if (indentation.StartsWith("\t\t") && findingsString.StartsWith("ref "))
						report.m_aUselessRefFound.Insert(lineNumberPlus1);

					if (findingsString.Contains("auto "))
						report.m_aAutoKeywordFound.Insert(lineNumberPlus1);

					if (findingsString.Contains("autoptr "))
						report.m_aAutoptrKeywordFound.Insert(lineNumberPlus1);

					if (findingsString.StartsWith("for (")) // )
					{
						findingsString.Split(";", splits, true);
						if (splits[1].Contains("(")) // )
							report.m_aForCountCall.Insert(lineNumberPlus1);
					}

					foreach (string forbiddenDivision : m_aForbiddenDivisions)
					{
						if (findingsString.Contains(forbiddenDivision))
						{
							report.m_aDivideByXFound.Insert(lineNumberPlus1);
							break;
						}
					}

					if (
						lineNumber < linesCount - 1
						&& SCR_StringHelper.StartsWithAny(findingsString, m_aForForEachWhileArray)
						&& SCR_StringHelper.CountOccurrences(findingsString, "(") == SCR_StringHelper.CountOccurrences(findingsString, ")") // e.g multiline "while"
					)
					{
						string nextLine = lines[lineNumberPlus1];
						if (!nextLine.Trim().StartsWith(BRACKET_OPEN))
							report.m_aUnscopedLoopFound.Insert(lineNumberPlus1);
					}

					if (SCR_StringHelper.StartsWithAny(findingsString, m_aIfForForEachWhileArray) && SCR_StringHelper.EndsWithAny(findingsString, m_aEndBracketSemicolonArray))
						report.m_aOneLinerFound.Insert(lineNumberPlus1);

					if ((!isInRepository || formatThisLine) && HasBadVariableNaming(indentation, findingsString)) // )] // do not suggest renaming existing attributes, they are most likely public by now
						report.m_aBadVariableNamingFound.Insert(lineNumberPlus1);

					if (SCR_StringHelper.ContainsAny(findingsString, m_aScriptInvokerArray))
						report.m_aBadScriptInvokerFound.Insert(lineNumberPlus1);

					if (findingsString.StartsWith("Print") && findingsString.EndsWith(";"))
					{
						if (findingsString.StartsWith("Print(")) // )
						{
							if (!findingsString.Contains("LogLevel.") && !findingsString.Contains("logLevel)"))
								report.m_aWildPrintFound.Insert(lineNumberPlus1);

							if (findingsString.Contains("Print(string.Format(")) // ))
								report.m_aPrintToPrintFormatFound.Insert(lineNumberPlus1);
						}
						else
						if (findingsString.StartsWith("PrintFormat(")) // )
						{
							if (!findingsString.Contains("level:"))
								report.m_aWildPrintFound.Insert(lineNumberPlus1);
						}
					}

					if (findingsString.Contains("enums: ParamEnumArray.FromEnum(")) // )
						report.m_aEnumsToEnumTypeFound.Insert(lineNumberPlus1);

					if (SCR_StringHelper.StartsWithAny(findingsString, m_aPrefixLineChecks) && !SCR_StringHelper.StartsWithAny(findingsString, m_aPrefixChecks))
						report.m_aNonPrefixedClassOrEnumFound.Insert(lineNumberPlus1);
				}

				isPreviousLineEmpty = isCurrentLineEmpty;
			}

			prevFormatThisLine = formatThisLine;

			string finalLine = indentation + currContent;
			prevFullLine = finalLine;
			prevContent = currContent;

			if (finalLine == fullLine)
				continue;

			report.m_iLinesEdited++;

			if (demoMode)
				continue;

			if (useFileIO)
			{
				lines[lineNumber] = finalLine;
			}
			else
			{
				if (scriptEditor.SetOpenedResource(relativeFilePath))		// forces the edited file to be focused
					scriptEditor.SetLineText(finalLine, lineNumber);		// directly use Script Editor to avoid going through lines twice
			}
		}
		/*
			Finish
		*/

		if (!demoMode && m_bAddFinalLineReturn && linesCount > 0 && lines[linesCount - 1] != string.Empty)
		{
			if (useFileIO)
				lines.Insert(string.Empty);
			else
				AddFinalLineReturnToCurrentFile();

			report.m_bHasAddedFinalLineReturn = true;
		}

		// later, with proper line insertion/deletion
//		if (m_bRemoveMultiEmptyLines)
//		{
//			if (useFileIO)
//				doSomething();
//			else
//				RemoveEmptyLinesInCurrentFile();
//		}

		// write time!
		if (!demoMode && useFileIO)
			SCR_FileIOHelper.WriteFileContent(relativeFilePath, lines);

		report.m_iFormatTime = System.GetTickCount(startTick) - report.m_iReadTime; // startTick happens before read time measurement

		return report;
	}

	//------------------------------------------------------------------------------------------------
	//! Get relative file's content lines as array of strings
	//! \param[in] relativeFilePath the file's relative path
	//! \param[in] useFileIO use FileIO API, otherwise use Script Editor API (opening the file in a tab)
	//! \return file lines
	protected array<string> ReadFileContent(string relativeFilePath, bool useFileIO)
	{
		if (useFileIO)
			return SCR_FileIOHelper.ReadFileContent(relativeFilePath);

		// open file in Script Editor otherwise
		array<string> result = {};
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		scriptEditor.SetOpenedResource(relativeFilePath); // open tab

		for (int lineNumber, linesCount = scriptEditor.GetLinesCount(); lineNumber < linesCount; lineNumber++)
		{
			string line;
			// forces the read file to be focused - HEAVY on performance (e.g from 100 to 900ms) - use only for breakpoint debug
			// if (scriptEditor.SetOpenedResource(relativeFilePath))
				scriptEditor.GetLineText(line, lineNumber);

			result.Insert(line);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Output fixes and findings in the log console
	//! \param[in] report the generated report
	//! \param[in] printFixes print applied fixes (edited lines, trimmings, formatting etc)
	//! \param[in] printFindings print findings that may require user attention (one-liners, bad variable naming, etc)
	protected void PrintReport(notnull SCR_BasicCodeFormatterPluginFileReport report, bool printFixes, bool printFindings)
	{
		if (printFixes)
		{
			array<string> reportArray = {};

			if (report.m_iLinesEdited > 0)
				reportArray.Insert(string.Format("%1 lines changed", report.m_iLinesEdited));

			if (!report.m_aTrimmings.IsEmpty())
				reportArray.Insert(report.m_aTrimmings.Count().ToString() + " line trimmings"); // at line(s) " + JoinLineNumbers(report.m_aTrimmings));

			if (!report.m_aFixedIndentations.IsEmpty())
				reportArray.Insert(report.m_aFixedIndentations.Count().ToString() + "× indent fixes at lines " + JoinLineNumbers(report.m_aFixedIndentations));

			if (report.m_iGeneralFormattingTotal > 0)
				reportArray.Insert(string.Format("%1 formattings", report.m_iGeneralFormattingTotal));

			if (report.m_iEndSpacesRemovedTotal > 0)
				reportArray.Insert(string.Format("%1 end spaces trimming", report.m_iEndSpacesRemovedTotal));

			if (report.m_iFourSpacesReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 4-spaces indent -> tabs replaced", report.m_iFourSpacesReplacedTotal));

			if (report.m_iSpaceInTabsReplacedTotal > 0)
				reportArray.Insert(string.Format("%1 space(s) in indentation removed", report.m_iSpaceInTabsReplacedTotal));

			if (report.m_iMethodSeparatorFixedTotal > 0)
				reportArray.Insert(string.Format("%1 method separators fixed", report.m_iMethodSeparatorFixedTotal));

			if (report.m_bHasAddedFinalLineReturn)
				reportArray.Insert("added final newline");

			string reportLine = "";
			if (m_bDemoMode && !reportArray.IsEmpty())
				reportLine = "<span style='color: turquoise'>[DEMO]</span> ";

			if (report.m_bIsPluginFile)
				reportLine += "<span style='color: orange'>[PLUGIN FILE]</span> ";

			reportLine += "<strong>" + FilePath.StripPath(report.m_sRelativeFilePath) + "</strong> - ";

			if (reportArray.IsEmpty())
				reportLine += "no fixes to report";
			else
				reportLine += SCR_StringHelper.Join(", ", reportArray, false);

			reportLine += " (read: " + report.m_iReadTime + " ms, format: " + report.m_iFormatTime + " ms";

			if (report.m_iDiffTime > 0)
				reportLine += ", diff: " + report.m_iDiffTime + " ms";
			else
				report.m_iDiffTime = 0; // default value = -1

			reportLine += " - total: " + (report.m_iReadTime + report.m_iFormatTime + report.m_iDiffTime) + " ms)";

			Print((string)LOG_SEPARATOR, LogLevel.NORMAL);
			Print((string)reportLine, LogLevel.NORMAL); // cast prevents "string reportLine = " print prefix

			if (report.m_aDiffLines && !report.m_aDiffLines.IsEmpty())
			{
				int diffLinesCount = report.m_aDiffLines.Count();
				float percentage = diffLinesCount / report.m_iLinesTotal * 100;
				PrintFormat("Checked (%1/%2) edited lines %3 (%4%%)", diffLinesCount, report.m_iLinesTotal, JoinLineNumbers(report.m_aDiffLines), percentage.ToString(lenDec: 2), level: LogLevel.NORMAL); // not really ISO 31-0 compatible...
			}
			else
			{
				Print("Checked all " + report.m_iLinesTotal + " lines (100% of the file)", LogLevel.NORMAL);
			}
		}

		if (printFindings && !report.m_bIsPluginFile)
		{
			if (!report.m_aNewWithoutParentheseFound.IsEmpty())
				PrintFinding("new instance(s) without ()", report.m_aNewWithoutParentheseFound, "always use parentheses when instanciating a class");

			if (!report.m_aUselessRefFound.IsEmpty())
				PrintFinding("ref something", report.m_aUselessRefFound, "ref is not needed in script scope, only on class instance declaration");

			if (!report.m_aNewArrayFound.IsEmpty())
				PrintFinding("'new array<>'", report.m_aNewArrayFound, "replace by {} whenever possible");

			if (!report.m_aBadSeparatorFound.IsEmpty())
				PrintFinding("bad separator(s) detected", report.m_aBadSeparatorFound, "use separators only for methods, not for classes or anything else - keep the minimum amount of classes per file");

			if (!report.m_aDoubleEmptyLineFound.IsEmpty())
				PrintFinding("multiple consecutive empty lines", report.m_aDoubleEmptyLineFound, "leave only one");

			if (!report.m_aSuperfluousEmptyLineFound.IsEmpty())
				PrintFinding("superfluous empty line(s)", report.m_aSuperfluousEmptyLineFound, "an empty line is not welcome after the beginning of a scope");

			if (!report.m_aMissingEmptyLineFound.IsEmpty())
				PrintFinding("missing empty line(s)", report.m_aMissingEmptyLineFound, "have an empty line between the end of a scope (or an unscoped if/if-else) and code after that");

			if (!report.m_aAutoKeywordFound.IsEmpty())
				PrintFinding("'auto' keyword", report.m_aAutoKeywordFound, "use the direct type instead");

			if (!report.m_aAutoptrKeywordFound.IsEmpty())
				PrintFinding("'autoptr' keyword", report.m_aAutoptrKeywordFound, "remove if used on a standard variable, or replace with 'ref' if used for a member variable array");

			if (!report.m_aForCountCall.IsEmpty())
				PrintFinding("for-loop method call", report.m_aForCountCall, "a method is called every iteration; is it wanted? (usually arr.Count())");

			if (!report.m_aDivideByXFound.IsEmpty())
				PrintFinding("potentially avoidable division", report.m_aDivideByXFound, "use ' * 0.5' instead of ' / 2', ' * 0.1' instead of ' / 10' etc whenever possible");

			if (!report.m_aUnscopedLoopFound.IsEmpty())
				PrintFinding("loop without brackets", report.m_aUnscopedLoopFound, "always use brackets {} with for/foreach/while loops");

			if (!report.m_aOneLinerFound.IsEmpty())
				PrintFinding("one-liner", report.m_aOneLinerFound, "use a line return after an if/for/foreach/while condition");

			if (!report.m_aBadVariableNamingFound.IsEmpty())
				PrintFinding("badly-named variables", report.m_aBadVariableNamingFound, "use proper prefixes: m_s for ResourceName/string, m_v for vectors, NO m_p, CASED_CONSTS, etc");

			if (!report.m_aBadScriptInvokerFound.IsEmpty())
				PrintFinding("raw ScriptInvoker", report.m_aBadScriptInvokerFound, "use typed ScriptInvokerBase<> - see SCR_ScriptInvokerHelper.c for examples");

			if (!report.m_aWildPrintFound.IsEmpty())
				PrintFinding("wild Print()", report.m_aWildPrintFound, "use Print with LogLevel.XXX (or PrintFormat with level: LogLevel.XXX) to show this is not a temporary debug print");

			if (!report.m_aPrintToPrintFormatFound.IsEmpty())
				PrintFinding("string.Format Print", report.m_aPrintToPrintFormatFound, "Print(string.Format()) can be converted to PrintFormat()");

			if (!report.m_aEnumsToEnumTypeFound.IsEmpty())
				PrintFinding("enums using ParamEnumArray.FromEnum", report.m_aEnumsToEnumTypeFound, "\"enums:\" can be converted to \"enumType: EnumType\"");

			if (!report.m_aNonPrefixedClassOrEnumFound.IsEmpty())
			{
				array<string> prefixes;
				if (m_aAcceptedScriptPrefixes.IsEmpty())
					prefixes = FORCED_PREFIXES;
				else
					prefixes = m_aAcceptedScriptPrefixes;

				PrintFinding("non-prefixed class/enum", report.m_aNonPrefixedClassOrEnumFound, "classes and enums should be prefixed; see the settings to setup accepted prefixes (currently \"" + SCR_StringHelper.Join("\", \"", prefixes) + "\")");
			}

			if (!report.m_mForbiddenWordFound.IsEmpty())
			{
				const string description = "flagged words";
				if (m_bShowSpellCheckFindingsDetails)
				{
					int count;
					array<string> mapKeys = {}; // SCR_MapHelperT<string, ref array<int>>.GetKeys(report.m_mForbiddenWordFound);
					foreach (string key, array<int> lineNumbers : report.m_mForbiddenWordFound)
					{
						mapKeys.Insert(key);
						count += lineNumbers.Count();
					}

					PrintFormat(
						"<span style='color: #DAD; font-weight: bold'>%1×</span> %2 found; see Options > Spell Check Config:",
						count,
						description,
						level: LogLevel.NORMAL);

					mapKeys.Sort();
					array<int> lineNumbers;
					foreach (string mapKey : mapKeys)
					{
						string tip = string.Format("\"%1\" is not an accepted word", mapKey);
						string correction;
						if (m_mForbiddenWords.Find(mapKey, correction))
						{
							correction.TrimInPlace();
							if (correction) // !.IsEmpty()
								tip += "; " + correction;
						}

						tip.TrimInPlace();
						if (tip) // !.IsEmpty()
							tip = " - " + tip;

						lineNumbers = report.m_mForbiddenWordFound.Get(mapKey);
						int lineNumbersCount = lineNumbers.Count();
						for (int i = lineNumbersCount - 1; i >= 0; i--)
						{
							if (lineNumbers.Find(lineNumbers[i]) != i)
								lineNumbers.RemoveOrdered(i);
						}

						PrintFormat(
							"- <span style='color: #DAD; font-weight: bold'>%1×</span> %2 found at line(s) <span style='color: #FDA'>%3</span>%4",
							lineNumbersCount,
							string.Format("\"%1\"", mapKey),
							JoinLineNumbers(lineNumbers),
							tip,
							level: LogLevel.NORMAL);
					}
				}
				else // no details version
				{
					array<string> mapKeys = {};
					array<int> finalLineNumbers = {};
					foreach (string mapKey, array<int> lineNumbers : report.m_mForbiddenWordFound)
					{
						mapKeys.Insert(mapKey);
						finalLineNumbers.InsertAll(lineNumbers);
					}

					mapKeys.Sort();
					SCR_ArrayHelperT<int>.RemoveDuplicates(finalLineNumbers);

					PrintFinding(string.Format("%1 lines (%2 variations)", description, mapKeys.Count()), finalLineNumbers, string.Format("see Options > Spell Check Config (%1)", SCR_StringHelper.Join(", ", mapKeys)));
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! General formatting like spacing, "NULL" → "null", ";;" → ";", etc
	//! \param[in] indentation used to determine the scope level
	//! \param[in,out] pieces text pieces input/output
	//! \return number of replacements that happened
	protected int GeneralFormatting(string indentation, inout notnull array<string> pieces)
	{
		int piecesCount = pieces.Count();
		if (piecesCount < 1)
			return 0;

		// a semicolon used to be placed after a class, not anymore
		if (!indentation && pieces[0].StartsWith(BRACKET_CLOSE + ";")) // !IsEmpty for perf
		{
			pieces[0] = SCR_StringHelper.ReplaceTimes(pieces[0], BRACKET_CLOSE + ";", BRACKET_CLOSE, 1);
			return 1;
		}

		if (pieces[0].StartsWith(SCR_StringHelper.DOUBLE_SLASH)) // don't format (inline) comments
			return 0;

		int replacements;
		array<ref array<string>> format;
		string toReplace;
		string replaceWith;
		bool hasChanged;

		// line start
		int startPieceIndex = -1;
		string startPiece;
		foreach (int pieceIndex, string piece : pieces)
		{
			if (!SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
			{
				startPiece = piece;
				startPieceIndex = pieceIndex;
				break;
			}
		}

		if (startPiece) // !IsEmpty for perf
		{
			format = m_aGeneralFormatting_Start;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					PrintFormat("%1:%2 - safety exit! %3/%4", FilePath.StripPath(__FILE__), __LINE__, toReplace, replaceWith, level: LogLevel.WARNING);
					continue;
				}

				while (startPiece.StartsWith(toReplace))
				{
					startPiece = SCR_StringHelper.ReplaceTimes(startPiece, toReplace, replaceWith, 1);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(startPieceIndex, startPiece);
					replacements++;
				}
			}

			// "class abc : parent" and "foreach x : y" colon spacing
			if (
				(startPiece.StartsWith("class ") || startPiece.StartsWith("enum ") || startPiece.StartsWith("foreach (")) &&
				startPiece.IndexOf(":") > -1
			)
			{
				hasChanged = false;
				int colonIndex = startPiece.IndexOf(":");
				if (startPiece[colonIndex - 1] != SCR_StringHelper.SPACE)
				{
					startPiece = SCR_StringHelper.InsertAt(startPiece, SCR_StringHelper.SPACE, colonIndex);
					hasChanged = true;
				}

				colonIndex = startPiece.IndexOf(":");
				if (colonIndex < startPiece.Length() -1 && startPiece[colonIndex + 1] != SCR_StringHelper.SPACE)
				{
					startPiece = SCR_StringHelper.InsertAt(startPiece, SCR_StringHelper.SPACE, colonIndex + 1);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(startPieceIndex, startPiece);
					replacements++;
				}
			}
		}

		// line middles
		for (int pieceIndex; pieceIndex < piecesCount; pieceIndex++)
		{
			string piece = pieces[pieceIndex];

			if (SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
				continue;

			format = m_aGeneralFormatting_Middle;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					PrintFormat("%1:%2 - safety exit! %3/%4", FilePath.StripPath(__FILE__), __LINE__, toReplace, replaceWith, level: LogLevel.WARNING);
					continue;
				}

				while (piece.Contains(toReplace))
				{
					piece.Replace(toReplace, replaceWith);
					hasChanged = true;
				}

				if (hasChanged)
				{
					pieces.Set(pieceIndex, piece);
					replacements++;
				}
			}
		}

		// now line ends
		int endPieceIndex = -1;
		string endPiece;
		for (int pieceIndex = piecesCount - 1; pieceIndex >= 0; pieceIndex--)
		{
			if (!SCR_StringHelper.StartsWithAny(pieces[pieceIndex], FORMAT_IGNORE))
			{
				endPiece = pieces[pieceIndex];
				endPieceIndex = pieceIndex;
				break;
			}
		}

		if (endPiece) // !IsEmpty for perf
		{
			format = m_aGeneralFormatting_End;
			for (int i, cnt = format.Count(); i < cnt; i++)
			{
				toReplace = format[i][0];
				replaceWith = format[i][1];
				hasChanged = false;

				if (replaceWith.Contains(toReplace))
				{
					PrintFormat("%1:%2 - safety exit! %3/%4", FilePath.StripPath(__FILE__), __LINE__, toReplace, replaceWith, level: LogLevel.WARNING);
					continue;
				}

				while (endPiece.EndsWith(toReplace))
				{
					endPiece = endPiece.Substring(0, endPiece.Length() - toReplace.Length()) + replaceWith;
					hasChanged = true;
				}

				// start & end
				if (startPiece) // well, I hope a start piece exists if an end piece exists
				{
					if (startPiece.StartsWith("["))
					{
						if (endPiece.Trim().EndsWith(")];"))
						{
							int count = SCR_StringHelper.CountOccurrences(endPiece, ")];");
							endPiece = SCR_StringHelper.ReplaceTimes(endPiece, ")];", ")]", 1, count - 1);
							hasChanged = true;
						}
					}
				}

				if (hasChanged)
				{
					pieces.Set(endPieceIndex, endPiece);
					replacements++;
				}
			}
		}

		// replace a,b / a;b with a, b / a; b
		// replace 1/5 with 1 / 5 (+ - / *) - keep -1 but format 1-1 to 1 - 1
		for (int pieceIndex; pieceIndex < piecesCount; pieceIndex++)
		{
			string piece = pieces[pieceIndex];
			if (SCR_StringHelper.StartsWithAny(piece, FORMAT_IGNORE))
				continue;

			for (int i, length = piece.Length(); i < length; i++)
			{
				// after this point is for "check after"
				if (i == length - 1)
					break;

				string prevChar;
				if (i > 0)
					prevChar = piece[i - 1];

				string character = piece[i];

				string nextChar;
				if (i < length - 1)
					nextChar = piece[i + 1];

				if ((character == "," || character == ";") && (nextChar != SCR_StringHelper.SPACE && nextChar != SCR_StringHelper.TAB && nextChar != SCR_StringHelper.DOUBLE_QUOTE)) // tab = beware of line end + tab + comment
				{
					piece = SCR_StringHelper.InsertAt(piece, SCR_StringHelper.SPACE, i + 1);
					i++;
					length++;
					replacements++;
					continue;
				}

				// after this point is for "check before"
				if (i < 1)
					continue;

				if (character == "+" || character == SCR_StringHelper.DASH || character == "*" || character == SCR_StringHelper.SLASH)
				{
					if (SCR_StringHelper.DIGITS.Contains(prevChar))
					{
						piece = SCR_StringHelper.InsertAt(piece, SCR_StringHelper.SPACE, i);
						length++;
						replacements++;
						continue;
					}

					if (SCR_StringHelper.DIGITS.Contains(nextChar))
					{
						if (!(character == "-" && !(SCR_StringHelper.DIGITS.Contains(prevChar) || (i > 1 && SCR_StringHelper.DIGITS.Contains(piece[i - 2]))))) // exception for values like -1
						{
							piece = SCR_StringHelper.InsertAt(piece, SCR_StringHelper.SPACE, i + 1);
							i++;
							length++;
							replacements++;
							continue;
						}
					}
				}
			}

			pieces.Set(pieceIndex, piece);
		}

		return replacements;
	}

//	//------------------------------------------------------------------------------------------------
//	protected void RemoveEmptyLinesInCurrentFile()
//	{
//		ScriptEditor scriptEditor = Workbench.GetModule(scriptEditor);
//
//		// remove double empty lines
//		int actualLine;
//		bool isCurrentLineEmpty;
//		bool wasPreviousLineEmpty;
//		for (int i, cnt = scriptEditor.GetLinesCount(); i < cnt; i++)
//		{
//			scriptEditor.GetLineText(fullLine, i);
//			isCurrentLineEmpty = !fullLine; // !IsEmpty for perf
//
//			if (!isCurrentLineEmpty || !wasPreviousLineEmpty)
//			{
//				if (i != actualLine)
//					scriptEditor.SetLineText(fullLine, actualLine);
//
//				actualLine++;
//			}
//
//			wasPreviousLineEmpty = isCurrentLineEmpty;
//		}
//
//		// Print("last line with actual content = " + actualLine, LogLevel.NORMAL);
//
//		// delete trailing lines
//		for (int cnt = scriptEditor.GetLinesCount() - 1; actualLine <= cnt; cnt--)
//		{
//			scriptEditor.RemoveLine(cnt);
//		}
//
//		// remove top empty lines
//		scriptEditor.GetLineText(fullLine, 0);
//		for (int i = 0; i < 50; i++)
//		{
//			if (fullLine) // !IsEmpty for perf
//				break;
//
//			scriptEditor.RemoveLine(0);
//			scriptEditor.GetLineText(fullLine, 0);
//		}
//
//		// remove bottom empty lines
//		for (int i; i < 500; i++)
//		{
//			scriptEditor.RemoveLine(actualLine);
//		}
//	}

	//------------------------------------------------------------------------------------------------
	//! Add the final line return to a file (to end with a line return instead of the usual closing bracket)
	//! \return true if an empty line has been added, false otherwise
	protected bool AddFinalLineReturnToCurrentFile()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		int lastLineNumber = scriptEditor.GetLinesCount() - 1;
		string fullLine;
		scriptEditor.GetLineText(fullLine, lastLineNumber);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(fullLine))
			return false;

		if (m_bDemoMode)
			return true;

		// haxx
		scriptEditor.InsertLine(string.Empty, lastLineNumber);
		scriptEditor.SetLineText(fullLine, lastLineNumber);
		scriptEditor.SetLineText(string.Empty, lastLineNumber + 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Split line between indentation and content (trailing spaces included if any)
	//! \param[in] fullLine the line's content
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] content gets the text
	static void GetIndentAndLineContent(string fullLine, out string indentation, out string content)
	{
		int lineLength = fullLine.Length();
		if (lineLength < 1)
		{
			indentation = string.Empty;
			content = string.Empty;
			return;
		}

		int firstCharIndex = -1;
		for (int i; i < lineLength; i++)
		{
			string character = fullLine[i];
			if (character != SCR_StringHelper.TAB && character != SCR_StringHelper.SPACE)
			{
				firstCharIndex = i;
				break;
			}
		}

		if (firstCharIndex < 0)
		{
			indentation = fullLine;
			content = string.Empty;
			return;
		}

		if (firstCharIndex == 0)
		{
			indentation = string.Empty;
			content = fullLine;
			return;
		}

		indentation = fullLine.Substring(0, firstCharIndex);
		content = fullLine.Substring(firstCharIndex, lineLength - firstCharIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Get line content as array of strings. Cannot return an empty array
	//! \param[in] fullLine the line to analyse
	//! \param[out] indentation gets the left spacing (tabs and spaces)
	//! \param[out] pieces the line as split in code, string, comment parts - first element being indentation (empty when none). can be provided null, result is never null or empty
	protected static void GetIndentAndLineContentAsPieces(string fullLine, out string indentation, out array<string> pieces)
	{
		pieces = {};

		string content;
		GetIndentAndLineContent(fullLine, indentation, content);

		if (!content) // !IsEmpty for perf
			return;

		bool isInCommentBlock;
		bool isInString;
		string currentContent;
		for (int i, contentLength = content.Length(); i < contentLength; i++)
		{
			string previousChar;
			if (i > 0)
				previousChar = content[i - 1];

			string currentChar = content[i];
			string nextChar;
			if (i < contentLength - 1)
				nextChar = content[i + 1];

			/*
				string management
			*/
			if (!isInCommentBlock && currentChar == SCR_StringHelper.DOUBLE_QUOTE)
			{
				if (isInString)
				{
					if (previousChar != SCR_StringHelper.ANTISLASH) // not escaped
					{
						pieces.Insert(currentContent + currentChar);
						currentContent = string.Empty;
						isInString = false;
						continue;
					}
				}
				else
				{
					pieces.Insert(currentContent);
					currentContent = currentChar;
					isInString = true;
					continue;
				}
			}
			else
			/*
				comment management
			*/
			if (!isInString && currentChar == SCR_StringHelper.SLASH)
			{
				if (isInCommentBlock)
				{
					if (previousChar == "*")
					{
						pieces.Insert(currentContent + currentChar);
						currentContent = string.Empty;
						isInCommentBlock = false;
						continue;
					}
				}
				else
				{
					if (nextChar == "*")
					{
						pieces.Insert(currentContent);
						currentContent = currentChar;
						isInCommentBlock = true;
						continue;
					}
					else
					if (nextChar == SCR_StringHelper.SLASH) // the rest is comment
					{
						if (currentContent) // !IsEmpty for perf
							pieces.Insert(currentContent);

						pieces.Insert(content.Substring(i, contentLength - i));
						return;
					}
				}
			}

			currentContent += currentChar;

			if (i == contentLength - 1 && currentContent) // !IsEmpty for perf
				pieces.Insert(currentContent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks for bad prefixes and non-uppercased consts
	//! \param[in] fullLine the line to check - indentation is used to determine the variable's level (1 tab = member variable)
	//! \return true if a badly-named variable has been spotted
	protected bool HasBadVariableNaming(string indentation, string findingsString)
	{
		// only interested in one-tab variables :) (aka member variables)
		if (indentation != SCR_StringHelper.TAB)
			return false;

		if (findingsString.StartsWith("[")
			|| findingsString.StartsWith("#")
			|| findingsString.EndsWith(")")
			|| findingsString.EndsWith(SCR_StringHelper.COMMA)
			|| findingsString.Contains(SCR_StringHelper.COLON))
			return false;

		findingsString.Replace(SCR_StringHelper.TAB, SCR_StringHelper.SPACE);
		array<string> pieces = {};
		findingsString.Split(SCR_StringHelper.SPACE, pieces, true);
		int piecesCount = pieces.Count();
		if (piecesCount < 2)
			return false;

		if (pieces[1] == "=") // that's an enum or something alike
			return false;

		if (pieces.Contains("override")
			|| pieces.Contains("void")
			|| pieces.Contains("event")
			|| pieces.Contains("proto")
			|| pieces.Contains("external")	// that's a method
//			|| pieces.Contains("new")		// e.g ref map<string, string> m_mMap = new map<string, string>()
			|| pieces.Contains("return"))	// that's a global method's assignation
			return false;

		bool isProtected = pieces.RemoveItemOrdered("protected");
		if (isProtected)
			piecesCount--;

		bool isPrivate = pieces.RemoveItemOrdered("private");
		if (isPrivate)
			piecesCount--;

		bool isStatic = pieces.RemoveItemOrdered("static");
		if (isStatic)
			piecesCount--;

		bool isConst = pieces.RemoveItemOrdered("const");
		if (isConst)
			piecesCount--;

		bool isRef = pieces.RemoveItemOrdered("ref");
		if (isRef)
			piecesCount--;

		string type;
		for (int i; i < piecesCount; i++)
		{
			type += pieces[0];
			pieces.RemoveOrdered(0);
			if (SCR_StringHelper.CountOccurrences(type, "<") - SCR_StringHelper.CountOccurrences(type, ">") < 1)
				break;
		}

		string rest = SCR_StringHelper.Join(SCR_StringHelper.SPACE, pieces, false).Trim();
		if (!rest)
			return false; // can't create a false positive on e.g protected string\n m_sValue;

		string varName;
		int indexEnd = SCR_StringHelper.IndexOf(rest, VARIABLE_NAME_ENDING);
		if (indexEnd < 0)
		{
			varName = rest.Trim(); // out of options
		}
		else
		{
			varName = rest.Substring(0, indexEnd).Trim();
			rest = rest.Substring(indexEnd, rest.Length() - indexEnd).Trim();
			if (!rest.Contains("=") && !rest.Contains(SCR_StringHelper.SEMICOLON)) // should cut most of it
				return false;
		}

		if (varName.Length() < 3)
			return true;

		if (!SCR_StringHelper.CheckCharacters(varName, true, true, true, true))
			return false;

		if (isConst)
			return varName != SCR_StringHelper.Filter(varName, SCR_StringHelper.UPPERCASE + SCR_StringHelper.DIGITS + "_"); // varName is not in format [A-Z0-9_]+

		// m_ or s_ variables from now on

		string expectedPrefix;
		if (isStatic)
			expectedPrefix = STATIC_PREFIX;
		else
			expectedPrefix = MEMBER_PREFIX;

		if (!varName.StartsWith(expectedPrefix))				// myVar
			return true;

		if (!SCR_StringHelper.ContainsUppercase(varName))		// m_ivalue
			return true;

		if (varName.Contains("[") && varName.Contains("]"))		// m_aMyVar[5]
			type = "array<x>"; // inner type is not important

		if (m_mVariableTypePrefixes.Contains(type))
		{
			expectedPrefix += m_mVariableTypePrefixes.Get(type);
			return !varName.StartsWith(expectedPrefix);
		}

		foreach (string key, string value : m_mVariableTypePrefixesStart)
		{
			if (type.StartsWith(key))
			{
				expectedPrefix += value;
				return !varName.StartsWith(expectedPrefix);
			}
		}

		foreach (string key, string value : m_mVariableTypePrefixesEnd)
		{
			if (type.EndsWith(key))
			{
				expectedPrefix += value;
				return !varName.StartsWith(expectedPrefix);
			}
		}

		string hungarianChar = varName[2];
		if (SCR_StringHelper.ContainsLowercase(hungarianChar)) // m_zSomething
		{
			if (hungarianChar == "e")	// enum exception...
			{
				typename typeTypename = type.ToType();
				if (!typeTypename)
					return true;

				for (int i; i < 8; ++i) // 1, 2, 4, 8 and that's it
				{
					if (typename.EnumToString(typeTypename, i) != "unknown")
						return false;
				}

				return true;
			}

			return true;
		}

		// all good? return OK

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Joins the provided line numbers together with commas and ampersand for the last element;
	//! also groups them by range (e.g 1,2,3,4,6,7,9  = 1-5, 6-7, 9).\n
	//! examples:\n
	//! {} = ""\n
	//! { 1 } = "1"\n
	//! { 1, 2, 3 } = "1-3"\n
	//! { 1, 2, ... , 10, 11 } = "1-11"\n
	//! { 1, 2, 3, 7, 8, 9 } = "1-3 & 7-9"\n
	//! { 1, 2, 3, 5, 7, 8, 9 } = "1-3, 5 & 7-9"\n
	//! { 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23 } = "1, 3, 5, 7, 9, 11, 13, 15, 17, 19, ..."
	//! \param[in] lineNumbers said line numbers in ascending order
	//! \param[in] maxNumbers maximum number of lines before ellipsis
	//! \return the joined numbers
	protected static string JoinLineNumbers(notnull array<int> lineNumbers, int maxNumbers = LINE_NUMBER_LIMIT)
	{
		if (lineNumbers.IsEmpty())
			return string.Empty;

		int count = lineNumbers.Count();
		if (count == 1)
			return lineNumbers[0].ToString();

		int countG;
		bool wasSequel, isSequel;
		array<string> groups = {};
		bool hitLimit;
		foreach (int i, int currValue : lineNumbers)
		{
			wasSequel = isSequel;
			isSequel = i > 0 && currValue == lineNumbers[i - 1] + 1;
			if (isSequel)
			{
				if (i == count - 1)
					groups[countG - 1] = string.Format(LINE_NUMBER_RANGE, groups[countG - 1], currValue);

				continue;
			}
			else
			{
				if (wasSequel)
					groups[countG - 1] = string.Format(LINE_NUMBER_RANGE, groups[countG - 1], lineNumbers[i - 1]);

				groups.Insert(currValue.ToString());
				countG++;

				if (maxNumbers > 0 && countG >= maxNumbers) // we have enough, leave
				{
					hitLimit = i < count - 1;
					break;
				}
			}
		}

		string result = groups[0];
		for (int i = 1; i < countG - 1; i++)
		{
			result += ", " + groups[i];
		}

		if (hitLimit)
			return result + ", " + groups[countG - 1] + ", ...";

		if (countG > 1)
			result += " & " + groups[countG - 1];

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Print the finding with line numbers
	//! examples:
	//! "int(s)", {}, ""							= "No int(s) found"
	//! "int(s)", {}, "a tip"						= "No int(s) found"
	//! "int(s)", { 1, 2, 3 }, ""					= "3 int(s) found at line(s) 1, 2 & 3"
	//! "int(s)", { 1, 2, 3 }, "use longs instead"	= "3 int(s) found at line(s) 1, 2 & 3 - use longs instead"
	//! \param[in] description the finding(s)' description, starting with a lowercase as it is meant to be used in the middle of a sentence
	//! \param[in] lineNumbers the lines where the findings have been found
	//! \param[in] tip a suggestion to fix the finding
	protected void PrintFinding(string description, notnull array<int> lineNumbers, string tip = string.Empty)
	{
		description.TrimInPlace();
		if (lineNumbers.IsEmpty())
		{
			Print("No " + description + " found", LogLevel.NORMAL);
			return;
		}

		tip.TrimInPlace();
		if (tip) // !IsEmpty for perf
			tip = " - " + tip;

		PrintFormat(
			"<span style='color: #DAD; font-weight: bold'>%1×</span> %2 found at line(s) <span style='color: #FDA'>%3</span>%4",
			lineNumbers.Count(),
			description,
			JoinLineNumbers(lineNumbers),
			tip,
			level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the 1-based line numbers of lines that were modified since the last edit (according to the local VCS' diff)
	//! This method creates a temporary txt file next to the analysed one containing the diff result
	//! The temporary txt file is deleted (if everything goes well) after its parsing
	//! \param[in] absoluteFilePath the ABSOLUTE file path (e.g C:/ArmaReforger/Data/scripts/myFile.c)
	//! \param[in] isInRepository
	//! \return array of line numbers (first line = 1!), empty on no changes / new file, null on error
	protected array<int> GetFileModifiedLineNumbers(string absoluteFilePath, out bool isInRepository)
	{
		isInRepository = false;

		if (!FileIO.FileExists(absoluteFilePath))
		{
			Print("Absolute file does not exist, skipping diff for \"" + absoluteFilePath + "\"", LogLevel.ERROR);
			return null;
		}

		string absoluteFileDirectory = FilePath.StripFileName(absoluteFilePath);
		if (!absoluteFileDirectory.EndsWith(SCR_StringHelper.SLASH))
			absoluteFileDirectory += SCR_StringHelper.SLASH;

		string absoluteCmdOutputFilePath = absoluteFileDirectory + DIFF_FILENAME;
		string commandLine = string.Format(m_sDiffCommand, absoluteFilePath, absoluteCmdOutputFilePath);

		int errorCode = Workbench.RunCmd(commandLine, true);
		if (errorCode != 0)
		{
			Print("Failed to obtain diff - the file may not be VCS-added yet or the command is wrong (error code " + errorCode + ")", LogLevel.ERROR);
			Print("command line = " + commandLine, LogLevel.ERROR);

			if (FileIO.FileExists(absoluteCmdOutputFilePath))
			{
				if (!FileIO.DeleteFile(absoluteCmdOutputFilePath))
					Print("Temp diff file could NOT be deleted", LogLevel.WARNING);
			}

			return null;
		}

		if (!FileIO.FileExists(absoluteCmdOutputFilePath))
		{
			Print("Temp diff file does not exist", LogLevel.ERROR);
			return null;
		}

		isInRepository = true;

		bool started;
		int lineNumber;
		array<int> result;
		FileHandle fileHandle = FileIO.OpenFile(absoluteCmdOutputFilePath, FileMode.READ);
		if (fileHandle)
		{
			result = {};

			while (!fileHandle.IsEOF())
			{
				string line;
				fileHandle.ReadLine(line);
				if (line.StartsWith("@@ -"))
				{
					started = true;
					int plusIndex = line.IndexOf("+") + 1; // note the +1
					int commaIndex = line.IndexOfFrom(plusIndex, ",");
					string diffLineNumber = line.Substring(plusIndex, commaIndex - plusIndex);
					lineNumber = diffLineNumber.ToInt();
					continue; // lineNumber--; // because it is announcing the *next* line
				}

				if (!started)
					continue;

				if (line.StartsWith("-"))
					continue;

				if (line.StartsWith("+"))
					result.Insert(lineNumber);

				lineNumber++;
			}

			fileHandle.Close();
		}
		else
		{
			Print("Cannot open the temp diff file", LogLevel.ERROR);
		}

		if (!FileIO.DeleteFile(absoluteCmdOutputFilePath))
			Print("Cannot delete the temp diff file - " + absoluteCmdOutputFilePath, LogLevel.WARNING); // we have our result, the file will unfortunately stay

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		if (!m_SpellCheckConfig)
			m_SpellCheckConfig = SCR_ConfigHelperT<SCR_BasicCodeFormatterSpellCheckConfig>.GetConfigObject(SPELLCHECK_CONFIG);

		Workbench.ScriptDialog("Configure 'Basic Code Formatter' plugin", "Formats code, basically.", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool ButtonOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_BasicCodeFormatterPlugin()
	{
		// line start
		m_aGeneralFormatting_Start = {};
		m_aGeneralFormatting_Start.Insert({ "if(",			"if (" });
		m_aGeneralFormatting_Start.Insert({ "else if(",		"else if (" });
		m_aGeneralFormatting_Start.Insert({ "for(",			"for (" });
		m_aGeneralFormatting_Start.Insert({ "foreach(",		"foreach (" });
		m_aGeneralFormatting_Start.Insert({ "while(",		"while (" });
		m_aGeneralFormatting_Start.Insert({ "switch(",		"switch (" });

		// line middle
		m_aGeneralFormatting_Middle = {};
		m_aGeneralFormatting_Middle.Insert({ SCR_StringHelper.DOUBLE_SPACE,	SCR_StringHelper.SPACE });			// double spaces
		m_aGeneralFormatting_Middle.Insert({ ";;",			";" });				// double semi-colon (so, colon?)
		// m_aGeneralFormatting_Middle.Insert({ " , ",			", " });
		// m_aGeneralFormatting_Middle.Insert({ " ; ",			"; " });
		m_aGeneralFormatting_Middle.Insert({ " ,",			"," });
		m_aGeneralFormatting_Middle.Insert({ " ;",			";" });
		m_aGeneralFormatting_Middle.Insert({ " NULL;",		" null;" });
		m_aGeneralFormatting_Middle.Insert({ " NULL,",		" null," });
		m_aGeneralFormatting_Middle.Insert({ " NULL)",		" null)" });
		m_aGeneralFormatting_Middle.Insert({ "{ }",			"{}" });
		m_aGeneralFormatting_Middle.Insert({ "array <",		"array<" });
		foreach (string nativeType : NATIVE_TYPES)
		{
			m_aGeneralFormatting_Middle.Insert({ "<ref " + nativeType + ">",		"<" + nativeType + ">" });
			m_aGeneralFormatting_Middle.Insert({ "<ref " + nativeType + ",",		"<" + nativeType + "," });
			m_aGeneralFormatting_Middle.Insert({ ", ref " + nativeType + ">",		", " + nativeType + ">" });
			m_aGeneralFormatting_Middle.Insert({ ", ref " + nativeType + ",",		", " + nativeType + "," });
		}
		// m_aGeneralFormatting_Middle.Insert({ "autoptr ",	string.Empty });	// useless as all classes inherit from Managed and are therefore managed by ARC - WARNING - autoptr can be used as a substitute to ref!!
		m_aGeneralFormatting_Middle.Insert({ "( ",			"(" });
		m_aGeneralFormatting_Middle.Insert({ " )",			")" });

		// line end
		m_aGeneralFormatting_End = {};
		// m_aGeneralFormatting_End.Insert({ ")];",	")]" });
		m_aGeneralFormatting_End.Insert({ ", )]",	")]" });
		m_aGeneralFormatting_End.Insert({ ",)]",	")]" });
		// m_aGeneralFormatting_End.Insert({ "++i)",	"i++)" }); // veeery tempted
		// m_aGeneralFormatting_End.Insert({ "--i)",	"i--)" }); // to fix both :D

		m_aForbiddenDivisions = {};
		array<int> forbiddenDivisors = { 2, 4, 5, 8, 10, 20, 50, 100, 1000, 10000, 100000, 1000000 };
		foreach (int forbiddenDivisor : forbiddenDivisors)
		{
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + ";");
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + ")");
			m_aForbiddenDivisions.Insert(" / " + forbiddenDivisor + SCR_StringHelper.SPACE);
		}

		m_aPrefixLineChecks = { "class ", "enum " };

		m_aForForEachWhileArray = { "for ", "foreach ", "while " };
		m_aIfForForEachWhileArray = { "if ", "for ", "foreach ", "while " };
		m_aNewArrayNewRefArray = { "new array<", "new ref array<" };
		m_aScriptInvokerArray = { "ScriptInvoker()", "ScriptInvoker<", "ScriptInvoker\t", "ScriptInvoker " };
		m_aEndBracketSemicolonArray = { BRACKET_CLOSE, ";" };

		m_mVariableTypePrefixes = new map<string, string>();
		m_mVariableTypePrefixes.Insert("bool", "b");
		m_mVariableTypePrefixes.Insert("float", "f");
		m_mVariableTypePrefixes.Insert("int", "i");
		m_mVariableTypePrefixes.Insert("string", "s");
		m_mVariableTypePrefixes.Insert("ResourceName", "s");
		m_mVariableTypePrefixes.Insert("LocalizedString", "s");
		m_mVariableTypePrefixes.Insert("vector", "v");

		m_mVariableTypePrefixesStart = new map<string, string>();
		m_mVariableTypePrefixesStart.Insert("array<", "a");
		m_mVariableTypePrefixesStart.Insert("map<", "m");

		// enums - Q&D
		for (int i, count = SCR_StringHelper.UPPERCASE.Length(); i < count; i++)
		{
			m_mVariableTypePrefixesStart.Insert("E" + SCR_StringHelper.UPPERCASE[i], "e");
			m_mVariableTypePrefixesStart.Insert("SCR_E" + SCR_StringHelper.UPPERCASE[i], "e");
		}

		m_mVariableTypePrefixesEnd = new map<string, string>();
		m_mVariableTypePrefixesEnd.Insert("Widget", "w"); // will need more advanced checks (TextWidget, etc)
	}
}

//! Basic Code Formatter file report - databag
class SCR_BasicCodeFormatterPluginFileReport
{
	string m_sRelativeFilePath;
	bool m_bIsPluginFile;

	int m_iReadTime = -1;
	int m_iFormatTime = -1;
	int m_iDiffTime = -1;

	int m_iEndSpacesRemovedTotal;
	int m_iSpaceInTabsReplacedTotal;
	int m_iFourSpacesReplacedTotal;
	int m_iMethodSeparatorFixedTotal;
	int m_iGeneralFormattingTotal;

	int m_iLinesEdited;
	int m_iLinesTotal;

	// fixes
	ref array<int> m_aDiffLines;		//!< can be null
	ref array<int> m_aTrimmings = {};	// content is actually not used, only count... for now?
	ref array<int> m_aFixedIndentations = {};
	bool m_bHasAddedFinalLineReturn;

	// reports
	ref array<int> m_aBadSeparatorFound = {};
	ref array<int> m_aDoubleEmptyLineFound = {};
	ref array<int> m_aSuperfluousEmptyLineFound = {};
	ref array<int> m_aMissingEmptyLineFound = {};
	ref array<int> m_aNewWithoutParentheseFound = {};
	ref array<int> m_aUselessRefFound = {};
	ref array<int> m_aNewArrayFound = {};
	ref array<int> m_aAutoKeywordFound = {};
	ref array<int> m_aAutoptrKeywordFound = {};
	ref array<int> m_aForCountCall = {};
	ref array<int> m_aDivideByXFound = {};
	ref array<int> m_aUnscopedLoopFound = {};
	ref array<int> m_aOneLinerFound = {};
	ref array<int> m_aBadVariableNamingFound = {};
	ref array<int> m_aBadScriptInvokerFound = {};
	ref array<int> m_aWildPrintFound = {};
	ref array<int> m_aPrintToPrintFormatFound = {};
	ref array<int> m_aEnumsToEnumTypeFound = {};
	ref array<int> m_aNonPrefixedClassOrEnumFound = {};
	ref map<string, ref array<int>> m_mForbiddenWordFound = new map<string, ref array<int>>();

	//------------------------------------------------------------------------------------------------
	//! Check if the report has anything... to report
	//! \return true if there is nothing to report, false otherwise
	bool IsClean()
	{
		if (m_iLinesEdited != 0)
			return false;

		bool areAllElementsEmpty = // max 16 '&&' elements
			m_aBadSeparatorFound.IsEmpty() &&
			m_aDoubleEmptyLineFound.IsEmpty() &&
			m_aSuperfluousEmptyLineFound.IsEmpty() &&
			m_aMissingEmptyLineFound.IsEmpty() &&
			m_aNewWithoutParentheseFound.IsEmpty() &&
			m_aUselessRefFound.IsEmpty() &&
			m_aNewArrayFound.IsEmpty() &&
			m_aAutoKeywordFound.IsEmpty() &&
			m_aAutoptrKeywordFound.IsEmpty() &&
			m_aForCountCall.IsEmpty() &&
			m_aDivideByXFound.IsEmpty() &&
			m_aUnscopedLoopFound.IsEmpty() &&
			m_aOneLinerFound.IsEmpty() &&
			m_aBadVariableNamingFound.IsEmpty() &&
			m_aBadScriptInvokerFound.IsEmpty() &&
			m_aWildPrintFound.IsEmpty();

		if (!areAllElementsEmpty)
			return false;

		areAllElementsEmpty =
			m_aPrintToPrintFormatFound.IsEmpty() &&
			m_aEnumsToEnumTypeFound.IsEmpty() &&
			m_aNonPrefixedClassOrEnumFound.IsEmpty() &&
			m_mForbiddenWordFound.IsEmpty();

		return areAllElementsEmpty;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BasicCodeFormatterPluginFileReport(string relativeFilePath, bool isPluginFile)
	{
		m_sRelativeFilePath = relativeFilePath;
		m_bIsPluginFile = isPluginFile; // check not done here in case this class gets moved away from the plugin file
	}
}

//
//! \brief Force Basic Code Formatter - use Ctrl+Alt+Shift+K to trigger.
//! \see SCR_BasicCodeFormatterPlugin
//
[WorkbenchPluginAttribute(
	name: "Basic Code Formatter - forced",
	description: "Forces Code Formatting checking ALL lines only for the current file", // unused
	shortcut: "Ctrl+Alt+Shift+K",
	wbModules: { "ScriptEditor" },
	awesomeFontCode: 0xF036)]
class SCR_BasicCodeFormatterForcedPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return;

		SCR_BasicCodeFormatterPlugin formatterPlugin = SCR_BasicCodeFormatterPlugin.Cast(scriptEditor.GetPlugin(SCR_BasicCodeFormatterPlugin));
		if (!formatterPlugin)
		{
			Print("Failed to obtain Basic Code Formatter Plugin", LogLevel.ERROR);
			return;
		}

		formatterPlugin.RunForced();
	}
}

#endif // WORKBENCH
