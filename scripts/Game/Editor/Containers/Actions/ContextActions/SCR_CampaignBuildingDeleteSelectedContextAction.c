//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_CampaignBuildingDeleteSelectedContextAction : SCR_DeleteSelectedContextAction
{
	//------------------------------------------------------------------------------------------------
	override int GetParam()
	{
		return GetGame().GetPlayerController().GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;

		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(selectedEntity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent || compositionComponent.IsInteractionLocked())
			return false;

		return super.CanBeShown(selectedEntity, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity || !CanBeDeleted(selectedEntity))
			return false;

		return super.CanBePerformed(selectedEntity, cursorWorldPosition, flags);
	}

	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{
		if (!hoveredEntity || !CanBeDeleted(hoveredEntity))
			return;

		super.Perform(hoveredEntity, cursorWorldPosition);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanBeDeleted(notnull SCR_EditableEntityComponent hoveredEntity)
	{
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(hoveredEntity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!compositionComponent)
			return false;

		if (compositionComponent.IsInteractionLocked())
		{
			SendNotification();
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SendNotification()
	{
		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_COMPOSITION_UNDER_CONSTRUCTION);
	}
}
