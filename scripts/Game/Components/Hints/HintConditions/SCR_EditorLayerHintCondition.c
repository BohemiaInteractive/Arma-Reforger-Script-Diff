[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_EditorLayerHintCondition : SCR_BaseEditorHintCondition
{
	[Attribute("1")]
	protected bool m_bOnEnter;
	
	//------------------------------------------------------------------------------------------------
	protected void OnCurrentLayerChange(SCR_EditableEntityComponent currentLayer, SCR_EditableEntityComponent prevCurrentLayer)
	{
		if (m_bOnEnter)
		{
			if (currentLayer && !prevCurrentLayer)
				Activate();
			else
				Deactivate();
		}
		else
		{
			if (!currentLayer && prevCurrentLayer)
				Activate();
			else
				Deactivate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (layerManager)
			layerManager.GetOnCurrentLayerChange().Insert(OnCurrentLayerChange);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitConditionEditor(SCR_EditorManagerEntity editorManager)
	{
		SCR_LayersEditorComponent layerManager = SCR_LayersEditorComponent.Cast(SCR_LayersEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (layerManager)
			layerManager.GetOnCurrentLayerChange().Remove(OnCurrentLayerChange);
	}
}
