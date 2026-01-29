[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_FlashlightToolbarAction : SCR_BaseToggleToolbarAction
{
	SCR_LightManualCameraComponent m_FlashLight;
	
	protected void OnFlashlightStateChanged(bool state)
	{
		Toggle(state, state)
	}
	protected void InitFlashlight(SCR_ManualCamera camera)
	{
		SCR_CameraEditorComponent cameraComponent = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));	
		cameraComponent.GetOnCameraCreate().Remove(InitFlashlight);
		
		m_FlashLight = SCR_LightManualCameraComponent.Cast(camera.FindCameraComponent(SCR_LightManualCameraComponent));
		if (!m_FlashLight)
			return;
		
		m_FlashLight.GetOnLightChanged().Insert(OnFlashlightStateChanged);
		OnFlashlightStateChanged(m_FlashLight.IsLightOn());
	}

	override bool IsServer()
	{
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		if (m_FlashLight)
			m_FlashLight.ToggleLight();
	}
	
	override void Track()
	{
		SCR_CameraEditorComponent cameraComponent = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));	
		SCR_ManualCamera camera = cameraComponent.GetCamera();	
		if (cameraComponent && !camera)
			cameraComponent.GetOnCameraCreate().Insert(InitFlashlight);
		else
			InitFlashlight(camera);
	}
	

	override void Untrack()
	{
		if (!m_FlashLight) return;
		m_FlashLight.GetOnLightChanged().Remove(OnFlashlightStateChanged);
	}
};
	
