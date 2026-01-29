//! Returns true when editor state is the given state
[BaseContainerProps()]
class SCR_EditorIsStateActionCondition: SCR_AvailableActionCondition
{
	 
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditorState))]
	protected EEditorState m_EditorState;
	
	protected SCR_StatesEditorComponent m_StatesManager;
	
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_StatesManager)
		{
			m_StatesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
			
			if (!m_StatesManager)
				return true;
		}

		return GetReturnResult(m_StatesManager.GetState() == m_EditorState);
	}
};