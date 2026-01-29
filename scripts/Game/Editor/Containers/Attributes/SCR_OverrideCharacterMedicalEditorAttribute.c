/**
Medical attribute if it should override certain medical values
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_OverrideCharacterMedicalEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return null;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!characterDamageManager) 
			return null;
		
		if (characterDamageManager.GetState() == EDamageState.DESTROYED)  
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(characterDamageManager.GetOverrideCharacterMedicalGMAttribute());
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_CharRegenEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_CharBleedingEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_CharUnconsciousnessEditorAttribute);
		}	
		
		bool isOverridden = var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_CharRegenEditorAttribute, isOverridden);
		manager.SetAttributeEnabled(SCR_CharBleedingEditorAttribute, isOverridden);
		manager.SetAttributeEnabled(SCR_CharUnconsciousnessEditorAttribute, isOverridden);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
			if (!editableEntity) 
				return;
		
		IEntity owner = editableEntity.GetOwner();
		if (!owner) 
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!characterDamageManager || characterDamageManager.GetState() == EDamageState.DESTROYED) 
			return;
		
		bool value = var.GetBool();

		if (!value)
		{
			SCR_GameModeHealthSettings gameModeHealthSettings = SCR_GameModeHealthSettings.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeHealthSettings));
			if (!gameModeHealthSettings) 
				return;
			
			characterDamageManager.SetBleedingScale(gameModeHealthSettings.GetBleedingScale(), false);
			characterDamageManager.SetRegenScale(gameModeHealthSettings.GetRegenScale(), false);
			characterDamageManager.SetPermitUnconsciousness(gameModeHealthSettings.IsUnconsciousnessPermitted(), true);
		}
		
		characterDamageManager.SetOverrideCharacterMedicalGMAttribute(value);
	}
};