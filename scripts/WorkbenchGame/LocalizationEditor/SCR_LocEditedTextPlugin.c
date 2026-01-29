#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Move edited text to source", wbModules: { "LocalizationEditor" })]
class SCR_LocEditedTextPlugin : LocalizationEditorPlugin
{
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

		locEditor.BeginModify("LocEditedTextPlugin");
		BaseContainer item;
		string edited;
		foreach (int index : selected)
		{
			item = items.Get(index);
			item.Get("Target_en_us_edited", edited);
			if (!edited.IsEmpty())
			{
				locEditor.ModifyProperty(item, item.GetVarIndex("Target_en_us"), edited);
				locEditor.ModifyProperty(item, item.GetVarIndex("Target_en_us_edited"), string.Empty);
			}
		}
		locEditor.EndModify();
	}
}
#endif // WORKBENCH
