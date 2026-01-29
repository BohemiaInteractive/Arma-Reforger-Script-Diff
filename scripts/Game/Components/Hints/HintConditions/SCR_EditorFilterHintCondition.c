[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorFilterHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityState))]
	protected EEditableEntityState m_State;
	
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(EEditableEntityType, new ParamEnum("<None>", "-1")))]
	protected EEditableEntityType m_Type;
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		if (entitiesInsert && !entitiesInsert.IsEmpty())
		{
			if (m_Type == -1)
			{
				Activate();
			}
			else
			{
				for (int i, count = entitiesInsert.Count(); i < count; i++)
				{
					if (entitiesInsert[i].GetEntityType() == m_Type)
						Activate();
				}
			}
		}
		if (entitiesRemove && !entitiesRemove.IsEmpty())
		{
			if (m_Type == -1)
			{
				Deactivate();
			}
			else
			{
				for (int i, count = entitiesRemove.Count(); i < count; i++)
				{
					if (entitiesRemove[i].GetEntityType() == m_Type)
						Deactivate();
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(m_State);
		if (filter)
			filter.GetOnChanged().Insert(OnFilterChange);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(m_State);
		if (filter)
			filter.GetOnChanged().Remove(OnFilterChange);
	}
}
