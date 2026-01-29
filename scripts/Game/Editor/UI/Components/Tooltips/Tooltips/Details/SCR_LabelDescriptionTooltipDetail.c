[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_LabelDescriptionTooltipDetail : SCR_EntityTooltipDetail
{	
	[Attribute(desc: "The discription shown")]
	protected LocalizedString m_sDescription;
		
	[Attribute("0", desc: "One of the given labels should be on the Entity in order to dsiplay the tooltip", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aLabelRequired;
	
	[Attribute("0", desc: "In which modes should the tooltip be shown", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected ref array<EEditorMode> m_aShownInEditorModes;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		if (m_sDescription.IsEmpty() || m_aShownInEditorModes.IsEmpty() || m_aLabelRequired.IsEmpty())
			return false;
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || !m_aShownInEditorModes.Contains(editorManager.GetCurrentMode()))
			return false;
		
		SCR_EditableEntityUIInfo editableUiInfo = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
		if (!editableUiInfo)
			return false;
		
		TextWidget text = TextWidget.Cast(widget);
		if (!text)
			return false;
		
		text.SetText(m_sDescription);
		
		//~ Check if entity has one of the labels
		foreach (EEditableEntityLabel label: m_aLabelRequired)
		{
			if (editableUiInfo.HasEntityLabel(label))
				return true;
		}

		return false;
	}
}
