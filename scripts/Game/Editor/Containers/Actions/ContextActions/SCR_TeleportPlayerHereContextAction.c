[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TeleportPlayerHereContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (cursorWorldPosition == vector.Zero || SCR_Enum.HasFlag(flags, EEditorContextActionFlags.LIMITED))
			return false;
		
		IEntity controlledEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!controlledEntity)
			return false;
		
		// Check faction entity
		if (SCR_EditableFactionComponent.Cast(hoveredEntity))
			return false;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(controlledEntity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return true;
		
		return damageManager.GetState() != EDamageState.DESTROYED;
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		//~ Do not teleport if hovering over own player as this will cotnrol the player and teleporting player to the same location as it is doesn't really do anything
		if (hoveredEntity && hoveredEntity.GetPlayerID() == SCR_PlayerController.GetLocalPlayerId())
			return false;
		
		return CanBeShown(hoveredEntity, selectedEntities, cursorWorldPosition, flags);
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PlayersManagerEditorComponent playersManager = SCR_PlayersManagerEditorComponent.Cast(SCR_PlayersManagerEditorComponent.GetInstance(SCR_PlayersManagerEditorComponent));
		playersManager.TeleportPlayerToPosition(cursorWorldPosition);
	}
};