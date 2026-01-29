[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorAttributesHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: { new ParamEnum("Game Mode", "0"), new ParamEnum("Camera", "1"), new ParamEnum("Editable Entity", "2") })]
	protected int m_iItemType;
	
	//------------------------------------------------------------------------------------------------
	protected void OnAttributesStart()
	{
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (attributesManager)
		{
			array<Managed> items = {};
			if (attributesManager.GetEditedItems(items) == 0 || !items[0])
				return;
			
			typename type;
			switch (m_iItemType)
			{
				case 0: type = SCR_BaseGameMode; break;
				case 1: type = SCR_ManualCamera; break;
				case 2: type = SCR_EditableEntityComponent; break;
			}
			
			if (items[0].Type().IsInherited(type))
				Activate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (attributesManager)
		{
			attributesManager.GetOnAttributesStart().Insert(OnAttributesStart);
			attributesManager.GetOnAttributesConfirm().Insert(Deactivate);
			attributesManager.GetOnAttributesCancel().Insert(Deactivate);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_AttributesManagerEditorComponent attributesManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (attributesManager)
		{
			attributesManager.GetOnAttributesStart().Remove(OnAttributesStart);
			attributesManager.GetOnAttributesConfirm().Remove(Deactivate);
			attributesManager.GetOnAttributesCancel().Remove(Deactivate);
		}
	}
}
