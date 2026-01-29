#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Temporarily unlock selected string(s)", shortcut: "Ctrl+E", wbModules: { "LocalizationEditor" })]
class SCR_LocStatusPlugin : LocalizationEditorPlugin
{
	protected ref array<string> m_aUnlocked = {};

	//------------------------------------------------------------------------------------------------
	protected void ResetStatus(BaseContainer stringTableItem, string id, EStringTableStatus status)
	{
		LocalizationEditor locEditor = Workbench.GetModule(LocalizationEditor);
		locEditor.BeginModify("LocStatusPlugin");
		locEditor.ModifyProperty(stringTableItem, stringTableItem.GetVarIndex("Status"), EStringTableStatus.DEVELOPMENT_PENDING.ToString());
		locEditor.EndModify();

		Workbench.ScriptDialog("Notification", string.Format("Please note that editing the text of '%1' changed its status\nfrom %2 to %3!\n\nWhen done, please change the string status to DEVELOPMENT_DONE.", id, typename.EnumToString(EStringTableStatus, status), typename.EnumToString(EStringTableStatus, EStringTableStatus.DEVELOPMENT_PENDING)), this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnChange(BaseContainer stringTableItem, string propName, string propValue)
	{
		if (propName == "Target_en_us")
		{
			string id;
			EStringTableStatus status;
			stringTableItem.Get("Id", id);
			stringTableItem.Get("Status", status);

			switch (status)
			{
				case EStringTableStatus.PROOFREADING_PENDING:
				case EStringTableStatus.PROOFREADING_DONE:
				case EStringTableStatus.TRANSLATION_PENDING:
				{
					Workbench.ScriptDialog("Warning", string.Format("String '%1' is marked as %2!\n\nPlease modify 'Target_en_us_edited' instead of 'Target_en_us'.\n\nThe edited version will appear in the game while waiting for translation\n in the next batch.", id, typename.EnumToString(EStringTableStatus, status)), this);
					ResetStatus(stringTableItem, id, status);
					break;
				}

				case EStringTableStatus.DEVELOPMENT_DONE:
				case EStringTableStatus.TRANSLATION_DONE:
				{
					ResetStatus(stringTableItem, id, status);
					break;
				}
			}
		}

		if (propName == "Target_en_us_edited" && !propValue.IsEmpty() && propValue != "@@@")
		{
			string id;
			EStringTableStatus status;
			stringTableItem.Get("Id", id);
			stringTableItem.Get("Status", status);

			switch (status)
			{
				case EStringTableStatus.DEVELOPMENT_PENDING:
				case EStringTableStatus.DEVELOPMENT_DONE:
				case EStringTableStatus.TRANSLATION_DONE:
				{
					Workbench.ScriptDialog("Notification", string.Format("Editing of the string '%1' is not restricted.\n\nPlease modify 'Target_en_us' directly instead of 'Target_en_us_edited'.", id, typename.EnumToString(EStringTableStatus, status)), this);
					break;
				}

				default:
				{
					ResetStatus(stringTableItem, id, status);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//--- Lock based on status (always unlocked when importing)
	override bool IsReadOnly(BaseContainer item, bool isImporting)
	{
		if (!isImporting)
		{
			EStringTableStatus status;
			item.Get("Status", status);

			string id;
			item.Get("Id", id);

			switch (status)
			{
				case EStringTableStatus.PROOFREADING_PENDING:
				case EStringTableStatus.PROOFREADING_DONE:
				case EStringTableStatus.TRANSLATION_PENDING:
				case EStringTableStatus.TRANSLATION_DONE:
				{
					//--- Locked unless temporarily unlocked by plugin
					return !m_aUnlocked.Contains(id);
				}
			}
		}

		//--- Unlocked by default
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSelectionChanged()
	{
		m_aUnlocked.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		LocalizationEditor locEditor = Workbench.GetModule(LocalizationEditor);
		BaseContainer table = locEditor.GetTable();
		if (!table)
			return;

		BaseContainerList items = table.GetObjectArray("Items");
		array<int> selected = {};
		locEditor.GetSelectedRows(selected);

		m_aUnlocked.Clear();
		BaseContainer item;
		string id;
		foreach (int index : selected)
		{
			item = items.Get(index);
			item.Get("Id", id);
			m_aUnlocked.Insert(id);
		}

		locEditor.RefreshUI();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}
#endif // WORKBENCH
