[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityState, "m_State")]
/** @ingroup Editor_Components_Entities
*/
/*!
*/
class SCR_HighlightedEditableEntityFilter : SCR_BaseEditableEntityFilter
{
	SCR_HighlightedEditorComponent m_HighlightedEditorComponent;
	
	
	void OnHighlightAdded(SCR_EditableEntityComponent entity)
	{
		if (Validate(entity))
			Add(entity);
	}
	
	void OnHighlightRemoved(SCR_EditableEntityComponent entity)
	{
		if (Validate(entity))
			Remove(entity);
	}
	
	override bool CanAdd(SCR_EditableEntityComponent entity)
	{		
		return m_HighlightedEditorComponent && m_HighlightedEditorComponent.IsEntityHighlighted(entity);
	}
	
	
	//-------------------------- On Editor Activate/Deactivate --------------------------\\
	override void EOnEditorActivate()
	{
		if (!m_HighlightedEditorComponent)
			m_HighlightedEditorComponent = SCR_HighlightedEditorComponent.Cast(SCR_HighlightedEditorComponent.GetInstance(SCR_HighlightedEditorComponent));
		
		if (m_HighlightedEditorComponent)
		{
			m_HighlightedEditorComponent.GetOnAddHighlight().Insert(OnHighlightAdded);
			m_HighlightedEditorComponent.GetOnRemoveHighlight().Insert(OnHighlightRemoved);
		}
	}
	override void EOnEditorDeactivate()
	{
		if (m_HighlightedEditorComponent)
		{
			m_HighlightedEditorComponent.GetOnAddHighlight().Remove(OnHighlightAdded);
			m_HighlightedEditorComponent.GetOnRemoveHighlight().Remove(OnHighlightRemoved);
		}
	}
};