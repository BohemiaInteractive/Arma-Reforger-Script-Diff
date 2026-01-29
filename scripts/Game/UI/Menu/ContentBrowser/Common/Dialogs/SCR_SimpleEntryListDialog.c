/*!
Dialog displaying list of names
*/

//------------------------------------------------------------------------------------------------
//! Shows a list of simple entries
class SCR_SimpleEntryListDialog : SCR_ConfigurableDialogUi
{
	protected ResourceName ADDON_LINE_LAYOUT = "{99668AEF3063A9F2}UI/layouts/Menus/Common/SimpleEntry.layout";
	
	protected ref array<string> m_aNames = {};
	protected ref array<SCR_SimpleEntryComponent> m_aLines = {};
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("AddonList"));
		if (!layout)
		{
			Print("SCR_SimpleEntryListDialog - can't create list due to missing list layout" , LogLevel.WARNING);
			return;
		}
		
		// Create widgets
		for (int i = 0, count = m_aNames.Count(); i < count; i++)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT, layout);
			
			SCR_SimpleEntryComponent comp = SCR_SimpleEntryComponent.Cast(w.FindHandler(SCR_SimpleEntryComponent));
			if (!comp)
				continue;
			
			comp.SetMessages(m_aNames[i], string.Empty);
			
			m_aLines.Insert(comp);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SimpleEntryListDialog CreateDialog(array<string> names, string preset, ResourceName dialogsConfig = "")
	{
		if (dialogsConfig == string.Empty)
			dialogsConfig = SCR_WorkshopDialogs.DIALOGS_CONFIG;
						
		SCR_SimpleEntryListDialog ListDialog = new SCR_SimpleEntryListDialog(names, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, ListDialog);
		return ListDialog;
	}
	
	//------------------------------------------------------------------------------------------------
	//! !!! Don't use the constructor! Use the Create... methods instead.
	protected void SCR_SimpleEntryListDialog(array<string> names, string preset)
	{
		m_aNames.InsertAll(names);
		
		if (!preset.IsEmpty())
			SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, preset, this);
	}
};