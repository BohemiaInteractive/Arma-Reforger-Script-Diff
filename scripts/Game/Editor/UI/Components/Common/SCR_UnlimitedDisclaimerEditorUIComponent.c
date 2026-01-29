class SCR_UnlimitedDisclaimerEditorUIComponent : ScriptedWidgetComponent
{
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorSettingsEntity settingsEntity = core.GetSettingsEntity();
		if (!settingsEntity)
			return;
		
		w.SetVisible(!settingsEntity.IsUnlimitedEditorLegal());
	}
}
