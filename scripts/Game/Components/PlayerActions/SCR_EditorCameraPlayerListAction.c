class SCR_EditorCameraPlayerListAction : SCR_PlayerListBaseAction
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return false;

		SCR_EditorManagerEntity editorMgr = core.GetEditorManager();
		if (!editorMgr || !editorMgr.IsOpened())
			return false;

		SCR_EditorModeEntity currentlyUsedModeEntity = editorMgr.GetCurrentModeEntity();
		if (!currentlyUsedModeEntity || !currentlyUsedModeEntity.IsOpened())
			return false;

		EEditorMode currentMode = currentlyUsedModeEntity.GetModeType();
		return currentMode == EEditorMode.ADMIN || currentMode == EEditorMode.EDIT;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return false;

		SCR_EditorManagerEntity editorMgr = core.GetEditorManager(userPlayerId);
		if (!editorMgr || !editorMgr.IsOpened())
		{
			if (SCR_PlayerController.s_pLocalPlayerController != userPlayerController)
				return false;

			editorMgr = core.GetEditorManager(); // clients dont fill in an array used by GetEditorManager(userPlayerId) thus one without player id must be used for them
			if (!editorMgr || !editorMgr.IsOpened())
				return false;
		}

		SCR_EditorModeEntity currentlyUsedModeEntity = editorMgr.GetCurrentModeEntity();
		if (!currentlyUsedModeEntity || !currentlyUsedModeEntity.IsOpened())
			return false;

		EEditorMode currentMode = currentlyUsedModeEntity.GetModeType();
		return currentMode == EEditorMode.ADMIN || currentMode == EEditorMode.EDIT;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController)
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorMgr = core.GetEditorManager(userPlayerId);
		SCR_EditorModeEntity currentlyUsedModeEntity = editorMgr.GetCurrentModeEntity();
		IEntity targetCharacter = GetGame().GetPlayerManager().GetPlayerControlledEntity(targetPlayerId);
		if (!targetCharacter)
			return;

		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(currentlyUsedModeEntity.FindComponent(SCR_CameraEditorComponent));
		if (cameraManager)
			cameraManager.TeleportEditorCamera(targetCharacter.GetOrigin());
	}
}
