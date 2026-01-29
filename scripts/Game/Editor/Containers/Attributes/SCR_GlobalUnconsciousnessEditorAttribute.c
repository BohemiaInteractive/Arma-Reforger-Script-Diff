[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GlobalUnconsciousnessEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		//If opened in global attributes
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return null;
		
		SCR_GameModeHealthSettings gameModeHealthSettings = SCR_GameModeHealthSettings.Cast(gamemode.FindComponent(SCR_GameModeHealthSettings));
		if (!gameModeHealthSettings)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(gameModeHealthSettings.IsUnconsciousnessPermitted());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return;
		
		SCR_GameModeHealthSettings gameModeHealthSettings = SCR_GameModeHealthSettings.Cast(gamemode.FindComponent(SCR_GameModeHealthSettings));
		if (!gameModeHealthSettings)
			return;
		
		bool value = var.GetBool();

		//Notification
		if (item && value != gameModeHealthSettings.IsUnconsciousnessPermitted())
		{
			if (value)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_UNCONSCIOUSNESS_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_UNCONSCIOUSNESS_DISABLED, playerID);
		}
		
		gameModeHealthSettings.SetUnconsciousnessPermitted(value);
		
		//Neutralize character if unconsciousness is disabled and character is already unconscious
		if (value)
			return;
		
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();
		if (!characters)
			return;
			
		foreach (SCR_ChimeraCharacter character: characters)
		{
			SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
			if (!characterDamageManager) 
				continue;
			
			CharacterControllerComponent controller = character.GetCharacterController();
			if (!controller) 
				continue;
			
			ECharacterLifeState lifeState = controller.GetLifeState();
			if (lifeState == ECharacterLifeState.DEAD)
				continue;
			
			if (!characterDamageManager.IsDamageHandlingEnabled())
				continue;

			if (lifeState == ECharacterLifeState.INCAPACITATED)
				characterDamageManager.Kill(Instigator.CreateInstigatorGM());
		}
	}
};