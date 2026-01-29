/**
Modes and Budget attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ModesEditorAttribute: SCR_BaseMultiSelectPresetsEditorAttribute
{		
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!item.IsInherited(BaseGameMode))
			return null;
		
		//--- Global modes, influences which modes will players have access to upon connecting
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;
		
		SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
		if (!editorSettings)
			return null;
		
		//--- Select overridden base modes. If there is no override, select modes added by default to everyone.
		EEditorMode modes;
		if (!editorSettings.GetBaseModes(modes))
			modes = core.GetBaseModes(EEditorModeFlag.DEFAULT);

		super.ReadVariable(item, manager);
		SetFlags(modes);
		
		return SCR_BaseEditorAttributeVar.CreateVector(GetFlagVector());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{		
		if (!var)
			return;
		
		super.WriteVariable(item, var, manager, playerID);
		
		//--- Global modes
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;
		
		SCR_EditorSettingsEntity editorSettings = core.GetSettingsEntity();
		if (!editorSettings)
			return;
		
		editorSettings.SetBaseModes(GetFlags(0));
	}
	
	protected override void CreatePresets()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core) return;
		
		m_aValues.Clear();	
		
		SCR_EditorModePrefab mode;
		SCR_SortedArray<SCR_EditorModePrefab> modePrefabs = new SCR_SortedArray<SCR_EditorModePrefab>;
		for (int i, count = core.GetBaseModePrefabs(modePrefabs, EEditorModeFlag.EDITABLE, true); i < count; i++)
		{
			mode = modePrefabs[i];
			SCR_EditorAttributeFloatStringValueHolder value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(mode.GetInfo(), mode.GetMode());
			m_aValues.Insert(value);
		}
	}
};