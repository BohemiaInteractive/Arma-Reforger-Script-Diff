[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_TeleportCameraToPlayerContextAction : SCR_BaseContextAction
{
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return SCR_PlayerController.GetLocalControlledEntity() != null;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
		if (!playerEntity)
			return;
		
		vector teleportPosition = playerEntity.GetOrigin();		
		
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		if (cameraManager)
		{
			SCR_ManualCamera camera = cameraManager.GetCamera();
			if (camera)
			{
				SCR_TeleportToCursorManualCameraComponent teleportComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				if (teleportComponent)
				{
					teleportComponent.TeleportCamera(teleportPosition, true, true, true);
				}
			}
		}
	}
};