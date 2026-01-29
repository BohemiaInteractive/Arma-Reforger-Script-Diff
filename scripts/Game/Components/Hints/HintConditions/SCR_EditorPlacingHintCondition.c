[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorPlacingHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(EEditableEntityType, new ParamEnum("<None>", "-1")))]
	protected EEditableEntityType m_Type;
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectedPrefabChange(ResourceName prefab, ResourceName prefabPrev)
	{
		if (prefab)
		{
			if (m_Type == -1)
			{
				GetGame().GetCallqueue().CallLater(Activate, 100); //--- Wait for asset browser to close
			}
			else
			{
				Resource resource = Resource.Load(prefab);
				IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
				IEntityComponentSource editableEntitySource = SCR_EditableEntityComponentClass.GetEditableEntitySource(entitySource);
				if (SCR_EditableEntityComponentClass.GetEntityType(editableEntitySource) == m_Type)
				{
					GetGame().GetCallqueue().CallLater(Activate, 100); //--- Wait for asset browser to close
				}
			}
		}
		else
		{
			Deactivate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent));
		if (placingManager)
			placingManager.GetOnSelectedPrefabChange().Insert(OnSelectedPrefabChange);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent));
		if (placingManager)
			placingManager.GetOnSelectedPrefabChange().Remove(OnSelectedPrefabChange);
	}
}
