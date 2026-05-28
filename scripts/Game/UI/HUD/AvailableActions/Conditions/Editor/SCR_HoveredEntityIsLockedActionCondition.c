[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity is locked for editing (Free Roam Building)")]
class SCR_HoveredEntityIsLockedActionCondition : SCR_AvailableActionCondition
{
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SCR_EntitiesManagerEditorComponent entitiesManager = data.GetEditorEntitiesManagerComponent();
		if (!entitiesManager)
			return false;

		SCR_BaseEditableEntityFilter filter = entitiesManager.GetFilter(EEditableEntityState.HOVER);
		if (!filter)
			return false;

		SCR_EditableEntityComponent entity = filter.GetFirstEntity();
		if (!entity)
			return false;

		SCR_EditorManagerEntity editorManager = data.GetEditorManager();
		if (!editorManager)
			return false;

		if (editorManager.GetCurrentMode() != EEditorMode.BUILDING)
			return true;
		
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return true;

		return GetReturnResult(compositionComponent.IsInteractionLocked());
	}
}
