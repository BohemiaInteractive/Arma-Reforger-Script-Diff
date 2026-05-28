[WorkbenchPluginAttribute(name: PLUGIN_NAME, description: "Find non-localized text in UI", wbModules: { "LocalizationEditor" }, shortcut: "Ctrl+Shift+A")]
class CheckLocalizationPlugin: LocalizationEditorPlugin
{
	[Attribute(defvalue: "0", desc: "Add non-localized texts to the currently opened file")]
	protected bool m_bAddToLocalizationFile;

	[Attribute(defvalue: "", desc: "To which field must the non-localized text be set - e.g m_sSource, Target_en_us, etc")]
	protected string m_sSourceField;

	[Attribute(defvalue: "ENF-", desc: "Id field prefix for auto-generated ids")]
	protected string m_sIdPrefix;

	protected static const string PLUGIN_NAME = "Check localization in UI";
	protected static const int LAYOUT_WARNING_THRESHOLD = 100;	//!< rings at 100 and above
	protected static const int MAX_DISPLAYED_TEXT_LENGTH = 50;	//!< cuts the translation line at 50 chars (45 + ellipsis)
	protected static const int MAX_DISPLAYED_TEXTS = 6;			//!< max item ids displayed in the "are you sure?" modal

	protected static const string ID_FIELD = "Id";
	protected static const string ID_WHITELIST = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
	protected static const int ID_MAX_LENGTH = 12;

	//------------------------------------------------------------------------------------------------
	override event void Run()
	{
		m_sSourceField.TrimInPlace();
		m_sIdPrefix.TrimInPlace();

		LocalizationEditor localizationEditor = Workbench.GetModule(LocalizationEditor);
		if (!localizationEditor)
			return;

		if (m_bAddToLocalizationFile && m_sSourceField.IsEmpty())
		{
			Workbench.Dialog(PLUGIN_NAME, "The Source Field is empty - please provide it if you intend to add it to the localization file");
			Configure();
			return;
		}

		array<ResourceName> resourceNames = {};
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = { "layout" };
		ResourceDatabase.SearchResources(filter, resourceNames.Insert);

		int count = resourceNames.Count();
		if (count > LAYOUT_WARNING_THRESHOLD) // don't ask for a few files
		{
			if (Workbench.ScriptDialog(
				PLUGIN_NAME,
				"You are about to scan " + count + " layout files. Continue?",
				new WorkbenchDialog_OKCancel()) == 0)
			return;
		}

		array<ref CheckLocalizationPluginReport> reports = GetNonLocalizedReports(resourceNames);

		int reportsCount = reports.Count();
		if (reportsCount < 1)
		{
			Workbench.Dialog(PLUGIN_NAME, "All texts in UI are localized.");
			return;
		}

		array<string> translationsToAdd;
		if (m_bAddToLocalizationFile)
			translationsToAdd = {};

		string reportText;
		int missingCount;
		foreach (int i, CheckLocalizationPluginReport report : reports)
		{
			if (i == 0)
				reportText += report.m_sResourceName;
			else
				reportText += "\n\n" + report.m_sResourceName;

			foreach (CheckLocalizationPluginReportEntry reportEntry : report.m_aEntries)
			{
				reportText += string.Format("\n  \"%2\" at %1", reportEntry.m_sPath, reportEntry.m_sText);
				if (m_bAddToLocalizationFile)
					translationsToAdd.Insert(reportEntry.m_sText);

				++missingCount;
			}
		}

		Workbench.Dialog(PLUGIN_NAME, string.Format("Found %1 layouts with %2 non-localized texts!", reportsCount, missingCount), reportText);
		if (!m_bAddToLocalizationFile)
			return;

		if (!localizationEditor.GetTable()) // no table
			return;

		string message = "You are about to add the following " + missingCount + " non-localized texts to the translation table.";
		foreach (int i, string translationToAdd : translationsToAdd)
		{
			// will replace e.g the last wanted element by (...) in order to keep the good number of lines
			if (i >= MAX_DISPLAYED_TEXTS - 1 && missingCount > MAX_DISPLAYED_TEXTS)
			{
				message += "\n(...)";
				break;
			}

			translationToAdd.Replace("\n", "\\n");
			int length = translationToAdd.Length();
			if (length > MAX_DISPLAYED_TEXT_LENGTH)
				translationToAdd = translationToAdd.Substring(0, MAX_DISPLAYED_TEXT_LENGTH - 5) + "(...)"; // hardcoded ellipsis

			message += "\n- " + translationToAdd;
		}

		message += "\n\nContinue?";

		if (Workbench.ScriptDialog(PLUGIN_NAME, message, new WorkbenchDialog_OKCancel()) == 0)
			return;

		AddTranslationsToTable(localizationEditor, translationsToAdd);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceNames
	//! \return array of reports
	protected array<ref CheckLocalizationPluginReport> GetNonLocalizedReports(notnull array<ResourceName> resourceNames)
	{
		int count = resourceNames.Count();
		if (count < 1)
			return {};

		array<ref CheckLocalizationPluginReport> result = {};

		LocalizationEditor localisationEditor = Workbench.GetModule(LocalizationEditor);
		float prevProgress, currProgress;
		WBProgressDialog progress = new WBProgressDialog("Scanning Layouts", localisationEditor);
		foreach (int i, ResourceName resourceName : resourceNames)
		{
			currProgress = (i + 1) / count;
			if (currProgress - prevProgress >= 0.01)	// min 1%
			{
				progress.SetProgress(currProgress);		// expensive
				prevProgress = currProgress;
			}

			Resource resource = BaseContainerTools.LoadContainer(resourceName);
			if (!resource || !resource.IsValid())
				continue;

			WidgetSource widgetSource = WidgetSource.Cast(resource.GetResource());
			if (!widgetSource)
				continue;

			array<ref CheckLocalizationPluginReportEntry> reportEntries = GetNonLocalizedReportEntries(widgetSource);
			int entriesCount = reportEntries.Count();
			if (entriesCount < 1)
				continue;

			CheckLocalizationPluginReport report = new CheckLocalizationPluginReport();
			report.m_sResourceName = resourceName;
			report.m_aEntries = {};
			report.m_aEntries.Reserve(entriesCount);
			foreach (CheckLocalizationPluginReportEntry reportEntry : reportEntries)
			{
				report.m_aEntries.Insert(reportEntry);
			}

			result.Insert(report);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] widgetSource the widget to check recursively (the widget and its children will be checked)
	//! \param[in] path the widget name path
	//! \param[in] index child index
	//! \return array of report entries
	protected array<ref CheckLocalizationPluginReportEntry> GetNonLocalizedReportEntries(notnull WidgetSource widgetSource, string path = string.Empty, int index = -1)
	{
		array<ref CheckLocalizationPluginReportEntry> result = {};
		string widgetName = widgetSource.GetName();
		if (widgetName.IsEmpty())
		{
			if (index > -1)
				widgetName = string.Format("%1[%2]", widgetSource.GetClassName(), index);
			else
				widgetName = string.Format("%1[?]", widgetSource.GetClassName());
		}
		else
		{
			if (widgetName.Contains("/")) // root widget
				widgetName = ".";
		}

		if (path.IsEmpty())
			path = widgetName;
		else
			path = path + "/" + widgetName;

		string text;
		if (widgetSource.Get("Text", text) && !text.IsEmpty() && !text.Contains("#"))
		{
			CheckLocalizationPluginReportEntry report = new CheckLocalizationPluginReportEntry();
			report.m_sPath = path;
			report.m_sText = text;
			result.Insert(report);
		}

		for (int i, count = widgetSource.GetNumChildren(); i < count; i++)
		{
			WidgetSource childSource = widgetSource.GetChild(i);
			if (!childSource)
				continue;

			array<ref CheckLocalizationPluginReportEntry> reportEntries = GetNonLocalizedReportEntries(childSource, path, i);
			foreach (CheckLocalizationPluginReportEntry reportEntry : reportEntries)
			{
				result.Insert(reportEntry);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] localizationEditor
	//! \param[in] sourceTexts the texts to be added
	protected void AddTranslationsToTable(notnull LocalizationEditor localizationEditor, notnull array<string> sourceTexts)
	{
		if (sourceTexts.IsEmpty())
			return;

		BaseContainerList items = localizationEditor.GetTable().GetObjectArray("Items");

		array<string> existingIDs = {};
		for (int i, itemsCount = items.Count(); i < itemsCount; ++i)
		{
			string id;
			if (items.Get(i).Get(ID_FIELD, id))
				existingIDs.Insert(id);
		}

		localizationEditor.BeginModify(PLUGIN_NAME);
		int sourceIndex = -1;
		foreach (string sourceText : sourceTexts)
		{
			string id = GetTextId(sourceText);
			bool hasID = existingIDs.Contains(id);
			if (hasID)
			{
				for (int i; i < 1000; ++i)
				{
					id = GetTextId(sourceText, i);
					hasID = existingIDs.Contains(id);
					if (!hasID)
						break;
				}
			}

			if (hasID)
			{
				Workbench.Dialog(PLUGIN_NAME, "Failed on " + id);
				break;
			}

			BaseContainer item = localizationEditor.InsertItem(id, true, true);
			if (sourceIndex < 0)
			{
				sourceIndex = item.GetVarIndex(m_sSourceField);
				if (sourceIndex < 0)
				{
					localizationEditor.DeleteItem(id);
					Workbench.Dialog(PLUGIN_NAME, "Cannot find source field \"" + m_sSourceField + "\"");
					break;
				}
			}

			localizationEditor.ModifyProperty(item, sourceIndex, sourceText);
		}

		localizationEditor.EndModify();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] text the text source
	//! \param[in] attemptNumber the number of tries
	//! \return a valid ID (even on duplicate)
	protected string GetTextId(string text, int attemptNumber = -1)
	{
		bool afterSpace = true;
		string filteredAndCased;
		for (int i, length = text.Length(); i < length; ++i)
		{
			string character = text[i];
			if (character == " " || character == "\t" || character == "\n")
			{
				afterSpace = true;
			}
			else
			if (ID_WHITELIST.Contains(character))
			{
				if (afterSpace)
				{
					character.ToUpper();
					afterSpace = false;
				}

				filteredAndCased += character;
			}
		}

		if (filteredAndCased.IsEmpty())
		{
			if (attemptNumber < 0)
				return m_sIdPrefix + "Rename";
			else
				return m_sIdPrefix + "Rename_" + attemptNumber.ToString(3);
		}
		else
		{
			if (attemptNumber < 0)
				return m_sIdPrefix + filteredAndCased;
			else
				return m_sIdPrefix + filteredAndCased + attemptNumber.ToString(3);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected event void Configure()
	{
		Workbench.ScriptDialog(PLUGIN_NAME, string.Empty, this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected int ButtonClose()
	{
		return 0;
	}
}

class CheckLocalizationPluginReport
{
	ResourceName m_sResourceName;
	ref array<ref CheckLocalizationPluginReportEntry> m_aEntries;
}

class CheckLocalizationPluginReportEntry
{
	string m_sPath;
	string m_sText;
}
