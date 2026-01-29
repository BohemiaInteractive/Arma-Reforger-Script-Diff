[BaseContainerProps(), BaseContainerCustomStringTitleField("Hovered entity is locked for editing (Free Roam Building)")]
class SCR_HoveredEntityIsLockedActionComponent : SCR_AvailableActionCondition
{
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		if (!entity)
			return false;

		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
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
