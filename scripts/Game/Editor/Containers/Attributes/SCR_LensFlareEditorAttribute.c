/**
Lense flare control
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LensFlareEditorAttribute: SCR_BaseFloatValueHolderEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{	
		int count = m_aValues.Count();
		if (count <= 1)
			return null;
		
		SCR_CameraEditorComponent cameraComponent = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));	
		if (!cameraComponent)
			return null;
		
		SCR_ManualCamera camera = cameraComponent.GetCamera();	
		if (!camera)
			return null;
		
		SCR_LensFlareManualCameraComponent lensFlareComponent = SCR_LensFlareManualCameraComponent.Cast(camera.FindCameraComponent(SCR_LensFlareManualCameraComponent));
		if (!lensFlareComponent)
			return null;
		
		for(int i = 0; i < count; i++)
        {
            if ((int)m_aValues[i].GetFloatValue() == lensFlareComponent.GetLensFlareType())
				return SCR_BaseEditorAttributeVar.CreateInt(i);
        }
		
		return null;
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_CameraEditorComponent cameraComponent = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));	
		if (!cameraComponent)
			return;
		
		SCR_ManualCamera camera = cameraComponent.GetCamera();	
		if (!camera)
			return;
		
		SCR_LensFlareManualCameraComponent lensFlareComponent = SCR_LensFlareManualCameraComponent.Cast(camera.FindCameraComponent(SCR_LensFlareManualCameraComponent));
		if (!lensFlareComponent)
			return;
		
		lensFlareComponent.SetLensFlareType((int)m_aValues[var.GetInt()].GetFloatValue());
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		WriteVariable(null, GetVariable(), manager, -1);
	}
};