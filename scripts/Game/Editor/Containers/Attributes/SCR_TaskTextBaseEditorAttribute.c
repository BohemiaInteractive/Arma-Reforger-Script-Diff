[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_TaskTextBaseEditorAttribute: SCR_BaseEditorAttribute
{
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	protected ETaskTextType m_TextType;
	
	[Attribute("#AR-Editor_Attribute_CustomTaskName_LocationPlaceholder", desc: "Param used in the SCR_DropdownWithParamAttributeUIComponent to indicate for the player what will be replaced with the location name", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sLocationPlaceholderText;
	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		if (task && IsValidTaskType(task.GetTextType()))
			return SCR_BaseEditorAttributeVar.CreateInt(task.GetTextIndex());
		else
			return null;
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableTaskComponent task = SCR_EditableTaskComponent.Cast(item);
		task.SetTextIndex(var.GetInt());
	}

	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		//~ First entry is always the param used in the UI
		outEntries.Insert(new SCR_BaseEditorAttributeEntryText(m_sLocationPlaceholderText));
		
		SCR_TextsTaskManagerComponent textsComponent = SCR_TextsTaskManagerComponent.GetInstance();
		if (!textsComponent)
			return 0;
		
		array<ref SCR_UIDescription> infos = {};
		int count = textsComponent.GetTexts(m_TextType, infos);
		for (int i; i < count; i++)
		{
			outEntries.Insert(new SCR_BaseEditorAttributeEntryText(infos[i].GetName()));
		}
	
		return outEntries.Count();
	}
	
	protected bool IsValidTaskType(ETaskTextType taskType)
	{
		return taskType == m_TextType;
	}
}
