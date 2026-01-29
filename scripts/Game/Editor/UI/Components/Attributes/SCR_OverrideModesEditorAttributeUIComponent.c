/** @ingroup Editor_UI Editor_UI_Components Editor_UI_Attributes
*/
class SCR_OverrideModesEditorAttributeUIComponent: SCR_ButtonBoxMultiSelectAttributeUIComponent
{			
	protected override void ToggleEnableAttribute(bool enabled)
	{
		super.ToggleEnableAttribute(enabled);
		
		if(!enabled)
		{
			SCR_BaseEditorAttribute attribute = GetAttribute();
			if (!attribute) 
				return;
			
			SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
			if (!var)
				return;
			
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			if (!core)
				return;
			
			var.SetInt(core.GetBaseModes(EEditorModeFlag.DEFAULT));
			SetFromVar(var);
		}
	}
};