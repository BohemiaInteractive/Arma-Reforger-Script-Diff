[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ModeOverrideEditorAttribute : SCR_BaseEditorAttribute
{		
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item)) 
			return null;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;
		
		SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
		if (!editorSettings)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(editorSettings.IsBaseOverrideEnabled());
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
		if (!editorSettings)
			return;
		
		editorSettings.EnableBaseOverride(var.GetBool());
		
		//~ If set false set default editor modes settings
		if (!var.GetBool())
			editorSettings.SetBaseModes(core.GetBaseModes(EEditorModeFlag.DEFAULT));
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_ModesEditorAttribute);
		
		manager.SetAttributeEnabled(SCR_ModesEditorAttribute, var && var.GetBool());
	}
};