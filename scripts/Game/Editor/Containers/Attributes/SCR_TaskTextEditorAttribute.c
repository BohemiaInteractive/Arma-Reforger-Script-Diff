[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_TaskTextEditorAttribute: SCR_BaseEditorAttribute
{
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	protected ETaskTextType m_TextType;
	
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLocationNamePlaceholder;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		if (task && task.GetTextType() == m_TextType)
			return SCR_BaseEditorAttributeVar.CreateInt(task.GetTextIndex());
		else
			return null;
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		task.SetTextIndex(var.GetInt());
	}
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		SCR_TextsTaskManagerComponent textsComponent = SCR_TextsTaskManagerComponent.GetInstance();
		if (!textsComponent)
			return 0;
		
		array<ref SCR_UIDescription> infos = {};
		int count = textsComponent.GetTexts(m_TextType, infos);
		string text;
		for (int i; i < count; i++)
		{
			text = string.Format(infos[i].GetName(), m_sLocationNamePlaceholder);
			outEntries.Insert( new SCR_BaseEditorAttributeEntryText(text) );
		}		
		return count;
	}
};