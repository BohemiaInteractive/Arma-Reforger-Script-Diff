[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_LayerContextAction : SCR_BaseEditorAction
{
	[Attribute(desc: "Enable to enter the layer, disable to leave the layer")]
	protected bool m_bToEnter;
	
	[Attribute(desc: "When enable, it will not move just one layer up, but all the way to root. And the same when moving down.")]
	protected bool m_bToExtreme;
	
	[Attribute()]
	protected bool m_bIsHidden;
	
	protected bool CanExit(SCR_LayersEditorComponent layersManager)
	{
		return layersManager.GetCurrentLayer() //--- Not in root
			&& (!m_bToExtreme || layersManager.GetCurrentLayer().GetParentEntity()); //--- When parent is already in the root, don't show the extreme version
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (m_bIsHidden)
			return false;
		
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layersManager || !layersManager.IsEditingLayersEnabled())
			return false;
		
		if (m_bToEnter && !hoveredEntity)
			return false;
		
		if (m_bToEnter)
			return hoveredEntity.CanEnterLayer(layersManager);
		else
			return !hoveredEntity && CanExit(layersManager); //--- Context menu action to return is shown only when not hovering over any entity
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent));
		if (!layersManager || !layersManager.IsEditingLayersEnabled())
			return false;
		
		if (m_bToEnter && !hoveredEntity)
			return false;
		
		if (m_bToEnter)
			return hoveredEntity.CanEnterLayer(layersManager);
		else
			return CanExit(layersManager);
	}	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_LayersEditorComponent layersManager = SCR_LayersEditorComponent.Cast(SCR_BaseEditorComponent.GetInstance(SCR_LayersEditorComponent, true));
		if (!layersManager)
			return;
		
		if (m_bToEnter)
		{
			if (m_bToExtreme)
				//--- Make entity's parent current layer, so the entity icon is directly visible
				layersManager.SetCurrentLayer(hoveredEntity.GetParentEntity());
			else
				//--- Go one layer down
				layersManager.ToggleCurrentLayer(hoveredEntity);
		}
		else
		{
			if (m_bToExtreme)
				//--- Move to root
				layersManager.SetCurrentLayer(null);
			else
				//--- Go one layer up
				layersManager.SetCurrentLayerToParent();
		}
	}
	override bool IsServer()
	{
		return false;
	}
};