//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_AttachToEntityToolbarAction : SCR_BaseToggleToolbarAction
{
	/*
	private IEntity m_AttachedEntity;
	private SCR_AttachManualCameraComponent m_AttachCameraComponent;
	
	override bool GetToggled()
	{
		return m_AttachedEntity != null;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (GetToggled())
		{
			return true;
		}
		else
		{
			return GetTargetEntity(hoveredEntity, selectedEntities) != null;
		}
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		IEntity targetEntity = GetTargetEntity(hoveredEntity, selectedEntities);
		
		SCR_AttachManualCameraComponent attachComponent;
		if (GetAttachCameraComponent(attachComponent))
		{
			if (targetEntity && m_AttachedEntity != targetEntity)
			{
				if (!attachComponent.AttachTo(targetEntity))
				{
					targetEntity = null;
				}
			}
			else
			{
				attachComponent.Detach();
			}
			m_AttachedEntity = targetEntity;
		}
		GetOnToggleChanged().Invoke(m_AttachedEntity != null);
	}
	
	IEntity GetTargetEntity(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities)
	{
		if (hoveredEntity != null && hoveredEntity.GetOwner() != null)
		{
			return hoveredEntity.GetOwner();
		}
		else if (selectedEntities.Count() == 1 && selectedEntities[0].GetOwner() != null)
		{
			return selectedEntities[0].GetOwner();
		}
		return null;
	}
	
	bool GetAttachCameraComponent(out SCR_AttachManualCameraComponent attachComponent)
	{
		if (m_AttachCameraComponent)
		{
			attachComponent = m_AttachCameraComponent;
		}
		else
		{
			SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
			if (!cameraManager) return false;
		
			SCR_ManualCamera camera = cameraManager.GetCamera();
			if (!camera) return false;
			
			attachComponent = SCR_AttachManualCameraComponent.Cast(camera.FindCameraComponent(SCR_AttachManualCameraComponent));
			m_AttachCameraComponent = attachComponent;
		}
		return attachComponent != null;
	}
	*/
};