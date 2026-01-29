// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SimulateAndShowToggle: SCR_BaseEditorAttribute
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sSubDisplayName;
	
	[Attribute(desc: "If true the Sub checkbox will be disabled if Main is false. If false the Sub checkbox will be disabled if Main is true")]
	protected bool m_bDisableSubWhenMainIsFalse;
	[Attribute(defvalue: "1", desc: "If disabled will either set itself true or false")]
	protected bool m_bSetFalseIfDisabled;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) return null;
		
		#ifndef WORKBENCH
		return null;
		#endif

		//TODO GET VALUES
		vector saveValues = Vector(1, 0, 0);
		return SCR_BaseEditorAttributeVar.CreateVector(saveValues);
	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) return;
		
		//TODO: Set Values
	}
		
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sSubDisplayName));
		outEntries.Insert(new SCR_EditorAttributeEntryBool(m_bDisableSubWhenMainIsFalse));
		outEntries.Insert(new SCR_EditorAttributeEntryBool(m_bSetFalseIfDisabled));
		return outEntries.Count();
	}
};