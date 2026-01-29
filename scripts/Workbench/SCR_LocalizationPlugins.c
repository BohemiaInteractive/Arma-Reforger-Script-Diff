#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Modified Localisation Plugin", wbModules: { "LocalizationEditor" }, awesomeFontCode: 0xF246)]
class SCR_LocModifiedPlugin : LocalizationEditorPlugin
{
	[Attribute(category: "Authorship")]
	protected string UserName;

	//------------------------------------------------------------------------------------------------
	protected string GetUserName()
	{
		if (UserName.IsEmpty())
			Workbench.GetUserName(UserName);

		return UserName;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure Modified Localisation Plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	void OkButton()
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnChange(BaseContainer stringTableItem, string propName, string propValue)
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("Modified"), Workbench.GetPackedUtcTime().ToString());

		string userName = GetUserName();

		if (propName == "Id")
			editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("Author"), userName);

		editor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("LastChanged"), userName);
	}

	//------------------------------------------------------------------------------------------------
	override void OnImport(BaseContainer newItem, BaseContainer oldItem)
	{
		newItem.Set("Modified", Workbench.GetPackedUtcTime());

		string userName = GetUserName();

		if (oldItem)
			newItem.Set("LastChanged", userName);
		else
			newItem.Set("Author", userName);
	}
}

[WorkbenchPluginAttribute(name: "LocExportPlugin", wbModules: {"LocalizationEditor"})]
class SCR_LocExportPlugin : LocalizationEditorPlugin
{
	//------------------------------------------------------------------------------------------------
	override string GetExportColumn(BaseContainer item, string languageCode)
	{
#ifndef LOCALIZATION_BUILD_HIDDEN
		bool hidden = false;
		item.Get("Hidden", hidden);

		if (hidden)
			return string.Empty; // do not export this item
#endif
		if (languageCode == "en_us")
		{
			string edited;
			item.Get("Target_en_us_edited", edited);

			if (!edited.IsEmpty())
				return "Target_en_us_edited";
		}
		else
		{
			string translation;
			item.Get("Target_"+languageCode, translation);
			if (translation.IsEmpty())
				return "Target_en_us";
		}

		return "Target_" + languageCode;
	}
}

/*
[WorkbenchPluginAttribute(name: "LocTestPlugin", wbModules: {"LocalizationEditor"})]
class SCR_LocTestPlugin : LocalizationEditorPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		editor.AddUserFilter({ 1, 2, 3, 4, 5 }, "Test");
	}
}
*/

[WorkbenchPluginAttribute(name: "Check length", wbModules: { "LocalizationEditor" })]
class SCR_LocLengthPlugin : LocalizationEditorPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		LocalizationEditor editor = Workbench.GetModule(LocalizationEditor);
		BaseContainer stringTableSrc = editor.GetTable();
		if (!stringTableSrc)
			return;

		array<int> indices = {};
		array<string> targets;
//		string id;
		BaseContainerList rows = stringTableSrc.GetObjectArray("Items");
		BaseContainer row;

		for (int iRow, count = rows.Count(); iRow < count; iRow++)
		{
			row = rows[iRow];

			if (!targets)
			{
				targets = new array<string>;
				for (int iCol; iCol < row.GetNumVars(); iCol++)
				{
					string colName = row.GetVarName(iCol);
					if (colName.Contains("Target_"))
						targets.Insert(colName);
				}
			}

			string ID;
			row.Get("Id", ID);

			foreach (string targetCol : targets)
			{
				int maxLength;
				row.Get("MaxLength", maxLength);

				if (maxLength > 0)
				{
					string targetText;
					row.Get(targetCol, targetText);

					if (targetText.Length() > maxLength)
					{
						indices.Insert(iRow);
						Print(string.Format("LocLengthPlugin: ID: '%1', %2: '%3' is too long (%4/%5)", ID, targetCol, targetText, targetText.Length(), maxLength), LogLevel.NORMAL);
					}
				}
			}
		}

		if (!indices.IsEmpty())
			editor.AddUserFilter(indices, "Long texts");
	}
}
#endif // WORKBENCH
