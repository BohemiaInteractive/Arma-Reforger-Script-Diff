/**
Camera particle Attribute for photomode for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_CameraParticleEditorAttribute: SCR_BaseEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!item.IsInherited(SCR_CameraBase)) return null;
		
		SCR_CameraParticlesEditorComponent cameraParticlesManager = SCR_CameraParticlesEditorComponent.Cast(SCR_CameraParticlesEditorComponent.GetInstance(SCR_CameraParticlesEditorComponent));
		if (!cameraParticlesManager) return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(cameraParticlesManager.GetCurrentEffect());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_CameraParticlesEditorComponent cameraParticlesManager = SCR_CameraParticlesEditorComponent.Cast(SCR_CameraParticlesEditorComponent.GetInstance(SCR_CameraParticlesEditorComponent));
		if (!cameraParticlesManager) return;
		
		if (var) cameraParticlesManager.SetCurrentEffect(var.GetInt());
	}
	
	//------------------------------------------------------------------------------------------------
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		WriteVariable(null, GetVariable(), manager, -1);
	}
	
	//---- REFACTOR NOTE START: Loop might not be save? ----
	
	//------------------------------------------------------------------------------------------------
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Clear();
		
		SCR_CameraParticlesEditorComponent cameraParticlesManager = SCR_CameraParticlesEditorComponent.Cast(SCR_CameraParticlesEditorComponent.GetInstance(SCR_CameraParticlesEditorComponent));
		if (!cameraParticlesManager) return 0;
		
		array<SCR_CameraParticleEditor> effects = new array<SCR_CameraParticleEditor>;
		int effectsCount = cameraParticlesManager.GetEffects(effects);
		if (effectsCount == 0) return 0;
		
		for (int i = 0; i < effectsCount; i++)
		{
			outEntries.Insert(new SCR_BaseEditorAttributeEntryText(effects[i].GetDisplayName()));
		}
		return effectsCount;
	}
	
	//---- REFACTOR NOTE END ----
};