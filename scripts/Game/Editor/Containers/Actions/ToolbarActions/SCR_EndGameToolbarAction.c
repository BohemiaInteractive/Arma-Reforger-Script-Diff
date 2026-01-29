[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_EndGameToolbarAction : SCR_EditorToolbarAction
{	
	override bool IsServer()
	{
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!Replication.IsRunning())
			return false;
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return false;
		
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.GetInstance();
		if (!respawnComponent)
			return false;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (gameMode.GetState() != SCR_EGameModeState.GAME)
			return false;
		
		return !editorManager.IsLimited();
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!Replication.IsRunning())
			return false;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode.GetState() != SCR_EGameModeState.GAME)
			return false;
		
		return true;
	}
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags, int param = -1)
	{		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		int PlayerID = -1;
		
		if (playerController)
			PlayerID = playerController.GetPlayerId();
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager)
			return;
		
		if (editorManager.IsLimited())
			return;
		
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.GetInstance();
		if (!respawnComponent)
			return;
		
		SCR_AttributesManagerEditorComponent attributeManager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (!attributeManager)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode.GetState() != SCR_EGameModeState.GAME)
			return;
		
		attributeManager.StartEditing(respawnComponent);
	}
};