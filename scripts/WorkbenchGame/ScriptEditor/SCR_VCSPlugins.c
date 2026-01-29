#ifdef WORKBENCH
[WorkbenchPluginAttribute("VCS Configuration", "Configure Blame/Diff/Log/Commit commands", "", "", { "ResourceManager", "ScriptEditor" }, "VCS", 0xF1DE)]
class SCR_VCSSettingsPlugin : WorkbenchPlugin
{
	/*
		Category: Commands
	*/

	[Attribute(defvalue: TORTOISESVN_BLAME, desc: "Find out whodunit", category: "Commands")]
	protected string m_sBlameCommandLine;

	[Attribute(defvalue: TORTOISESVN_DIFF, desc: "Find out local changes", category: "Commands")]
	protected string m_sDiffCommandLine;

	[Attribute(defvalue: TORTOISESVN_LOG, desc: "Find out the file history", category: "Commands")]
	protected string m_sLogCommandLine;

	[Attribute(defvalue: TORTOISESVN_COMMIT, desc: "Commit the current directory", category: "Commands")]
	protected string m_sCommitCommandLine;

	/*
		Category: Settings
	*/

	[Attribute(defvalue: "1", desc: "Commit root directory, otherwise only commit the currently-opened file", category: "Settings")]
	protected bool m_bCommitRootDirectory;

	/*
		Category: Debug
	*/

	[Attribute(defvalue: "0", desc: "Prints the command in log console", category: "Debug")]
	protected bool m_bPrintCommandOnUse;

	protected static const string TORTOISESVN_BLAME = "TortoiseProc /command:blame /path:\"$path\" /startrev:1 /endrev:-1 /ignoreeol /ignoreallspaces /line:$line";
	protected static const string TORTOISESVN_DIFF = "TortoiseProc /command:diff /path:\"$path\"";
	protected static const string TORTOISESVN_LOG = "TortoiseProc /command:log /path:\"$path\"";
	protected static const string TORTOISESVN_COMMIT = "TortoiseProc /command:commit /path:\"$dir\"";

	protected static const string GITEXT_BLAME = "gitex blame \"$path\"";
	protected static const string GITEXT_DIFF = "gitex difftool \"$path\"";
	protected static const string GITEXT_LOG = "gitex filehistory \"$path\"";
	protected static const string GITEXT_COMMIT = "gitex commit \"$dir\"";

	//------------------------------------------------------------------------------------------------
	protected override void Configure()
	{
		Workbench.ScriptDialog(
		"Configure Blame/Diff/Log/Commit commands",
		"Settings are per Workbench module (e.g Resource Manager can have different settings than Script Editor).\n\n"
			+ "Available parameters:\n"
			+ "- $path: the absolute file path (unquoted)\n"
			+ "- $dir: the absolute file/commit directory (unquoted)\n"
			+ "- $line: the current line number",
		this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Set TortoiseSVN values")]
	protected void ButtonTortoiseSVN()
	{
		m_sBlameCommandLine = TORTOISESVN_BLAME;
		m_sDiffCommandLine = TORTOISESVN_DIFF;
		m_sLogCommandLine = TORTOISESVN_LOG;
		m_sCommitCommandLine = TORTOISESVN_COMMIT;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Set Git Extensions values")]
	protected void ButtonGitExtensions()
	{
		m_sBlameCommandLine = GITEXT_BLAME;
		m_sDiffCommandLine = GITEXT_DIFF;
		m_sLogCommandLine = GITEXT_LOG;
		m_sCommitCommandLine = GITEXT_COMMIT;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected void ButtonClose();

	//------------------------------------------------------------------------------------------------
	string	GetBlameCommandLine()		{ return m_sBlameCommandLine;		}
	string	GetDiffCommandLine()		{ return m_sDiffCommandLine;		}
	string	GetLogCommandLine()			{ return m_sLogCommandLine;			}
	string	GetCommitCommandLine()		{ return m_sCommitCommandLine;		}
	bool	GetCommitRootDirectory()	{ return m_bCommitRootDirectory;	}
	bool	GetPrintCommandOnUse()		{ return m_bPrintCommandOnUse;		}
}

class SCR_VCSRootPlugin : WorkbenchPlugin
{
	protected typename m_WorkbenchModuleTypename;

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		array<string> absoluteFilePaths = {};
		array<int> lineNumbers = {};

		SCR_VCSSettingsPlugin vcsSettingsPlugin = GetVCSSettings(m_WorkbenchModuleTypename);
		if (!vcsSettingsPlugin)
			return; // no settings?!

		bool commitRootDirectory = vcsSettingsPlugin.GetCommitRootDirectory();
		bool printCommand = vcsSettingsPlugin.GetPrintCommandOnUse();

		if (!GetFilesToProcess(absoluteFilePaths, lineNumbers))
		{
			// if no files open, commit root directory
			string thisStr = ToString();
			if (commitRootDirectory && thisStr.StartsWith("" + SCR_VCSCommitPlugin)) // ugly!
			{
				string command = GetCommandLine();
				string directory;
				if (!Workbench.GetAbsolutePath("", directory, true))
					return;

				command.Replace("$dir", directory);
				if (printCommand)
					Print("" + command, LogLevel.NORMAL);

				Workbench.RunCmd(command);
			}
			else // quit
			{
				Print("Cannot obtain current file(s) - " + thisStr, LogLevel.WARNING);
			}

			return;
		}

		int absoluteFilePathsCount = absoluteFilePaths.Count();
		if (absoluteFilePathsCount < 1)
			return;

		if (lineNumbers.Count() < absoluteFilePathsCount)
			lineNumbers.Resize(absoluteFilePathsCount); // fill with zeroes

		string originalCommand = GetCommandLine();
		if (printCommand)
			Print("" + originalCommand, LogLevel.NORMAL);

		string directory;
		if (commitRootDirectory)
			commitRootDirectory = Workbench.GetAbsolutePath("", directory, true); // cannot really fail?

		foreach (int i, string absoluteFilePath : absoluteFilePaths)
		{
			string command = originalCommand;

			if (!commitRootDirectory)
				directory = FilePath.StripFileName(absoluteFilePath);

			command.Replace("$path", absoluteFilePath);
			command.Replace("$dir", directory);
			command.Replace("$line", lineNumbers[i].ToString());

			if (printCommand)
				Print("" + command, LogLevel.NORMAL);

			Workbench.RunCmd(command);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_VCSSettingsPlugin GetVCSSettings(typename workbenchModuleTypename)
	{
		WBModuleDef workbenchModule = Workbench.GetModule(workbenchModuleTypename);
		if (!workbenchModule)
			return null;

		return SCR_VCSSettingsPlugin.Cast(workbenchModule.GetPlugin(SCR_VCSSettingsPlugin));
	}

	//------------------------------------------------------------------------------------------------
	protected string GetBlameCommandLine()
	{
		SCR_VCSSettingsPlugin vcsSettingsPlugin = GetVCSSettings(m_WorkbenchModuleTypename);
		if (!vcsSettingsPlugin)
			return string.Empty;

		return vcsSettingsPlugin.GetBlameCommandLine();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDiffCommandLine()
	{
		SCR_VCSSettingsPlugin vcsSettingsPlugin = GetVCSSettings(m_WorkbenchModuleTypename);
		if (!vcsSettingsPlugin)
			return string.Empty;

		return vcsSettingsPlugin.GetDiffCommandLine();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetLogCommandLine()
	{
		SCR_VCSSettingsPlugin vcsSettingsPlugin = GetVCSSettings(m_WorkbenchModuleTypename);
		if (!vcsSettingsPlugin)
			return string.Empty;

		return vcsSettingsPlugin.GetLogCommandLine();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetCommitCommandLine()
	{
		SCR_VCSSettingsPlugin vcsSettingsPlugin = GetVCSSettings(m_WorkbenchModuleTypename);
		if (!vcsSettingsPlugin)
			return string.Empty;

		return vcsSettingsPlugin.GetCommitCommandLine();
	}

	//------------------------------------------------------------------------------------------------
	// to be overridden by each module root plugin
	protected bool GetFilesToProcess(notnull out array<string> absoluteFilePaths, notnull out array<int> lineNumbers);

	//------------------------------------------------------------------------------------------------
	// to be overridden with Get*CommandLine by each plugin
	protected string GetCommandLine();
}

class SCR_VCSScriptEditorRootPlugin : SCR_VCSRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override bool GetFilesToProcess(notnull out array<string> absoluteFilePaths, notnull out array<int> lineNumbers)
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (!scriptEditor)
			return false;

		string file;
		string absoluteFilePath;
		if (!scriptEditor.GetCurrentFile(file) || !Workbench.GetAbsolutePath(file, absoluteFilePath))
		{
			Print("File cannot be opened " + file, LogLevel.WARNING);
			return false;
		}

		absoluteFilePaths.Insert(absoluteFilePath);
		lineNumbers.Insert(scriptEditor.GetCurrentLine() + 1); // 0-based

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_VCSScriptEditorRootPlugin()
	{
		m_WorkbenchModuleTypename = ScriptEditor;
	}
}

class SCR_VCSResourceManagerRootPlugin : SCR_VCSRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override bool GetFilesToProcess(notnull out array<string> absoluteFilePaths, notnull out array<int> lineNumbers)
	{
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return false;

		array<ResourceName> selection = {};
		resourceManager.GetResourceBrowserSelection(selection.Insert, true);

		if (selection.IsEmpty())
		{
			BaseContainer currentContainer = resourceManager.GetContainer(0);
			if (!currentContainer)
				return false; // image, script or something

			BaseContainer ancestor = currentContainer.GetAncestor();
			if (!ancestor)
				return false;

			selection.Insert(ancestor.GetResourceName());
		}

		foreach (ResourceName resourceName : selection)
		{
			string absoluteFilePath;
			if (!Workbench.GetAbsolutePath(resourceName.GetPath(), absoluteFilePath))
				continue;

			absoluteFilePaths.Insert(absoluteFilePath);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_VCSResourceManagerRootPlugin()
	{
		m_WorkbenchModuleTypename = ResourceManager;
	}
}

[WorkbenchPluginAttribute("VCS Log", "Show current file's changelog", "Alt+Shift+L", "", { "ScriptEditor" }, "VCS", 0xF1DE)]
class SCR_VCSLogScriptEditorPlugin : SCR_VCSScriptEditorRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetLogCommandLine();
	}
}

[WorkbenchPluginAttribute("File(s) VCS Changelog", "Show Resource Browser-selected file(s) changelog or (whenever possible) the currently opened file if no files are selected", "Alt+Shift+L", "", { "ResourceManager" }, "VCS", 0xF1DE)]
class SCR_VCSLogResourceManagerPlugin : SCR_VCSResourceManagerRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetLogCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Diff", "Show current file's local changes", "Alt+Shift+I", "", { "ResourceManager" }, "VCS", 0xF002)]
class SCR_VCSDiffResourceManagerPlugin : SCR_VCSResourceManagerRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetDiffCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Blame", "Show current file's line authors", "Alt+Shift+B", "", { "ResourceManager" }, "VCS", 0xF4FC)]
class SCR_VCSBlameResourceManagerPlugin : SCR_VCSResourceManagerRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetBlameCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Commit", "Commit current file directory's files", "Alt+Shift+C", "", { "ResourceManager" }, "VCS", 0xF4FC)]
class SCR_VCSCommitResourceManagerPlugin : SCR_VCSResourceManagerRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetCommitCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Diff", "Show current file's local changes", "Alt+Shift+I", "", { "ScriptEditor" }, "VCS", 0xF002)]
class SCR_VCSDiffPlugin : SCR_VCSScriptEditorRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetDiffCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Blame", "Show current file's line authors", "Alt+Shift+B", "", { "ScriptEditor" }, "VCS", 0xF4FC)]
class SCR_VCSBlamePlugin : SCR_VCSScriptEditorRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetBlameCommandLine();
	}
}

[WorkbenchPluginAttribute("VCS Commit", "Commit current file directory's files", "Alt+Shift+C", "", { "ScriptEditor" }, "VCS", 0xF4FC)]
class SCR_VCSCommitPlugin : SCR_VCSScriptEditorRootPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override string GetCommandLine()
	{
		return GetCommitCommandLine();
	}
}
#endif // WORKBENCH
