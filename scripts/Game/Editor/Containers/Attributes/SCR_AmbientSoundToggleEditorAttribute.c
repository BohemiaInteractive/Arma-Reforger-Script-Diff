/**
Toggle Ambient Sound Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AmbientSoundToggleEditorAttribute : SCR_BaseEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return null;
		
		MusicManager musicManager = world.GetMusicManager();
		if (!musicManager) 
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(!musicManager.ServerIsCategoryMuted(MusicCategory.Ambient));

	}	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		ChimeraWorld world = GetGame().GetWorld();

		if (!world)
			return;
		
		MusicManager musicManager = world.GetMusicManager();
		if (!musicManager) 
			return;
		
		if (var.GetBool())
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLED_AMBIENT_MUSIC, playerID);
		else 
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLED_AMBIENT_MUSIC, playerID);
		
		musicManager.RequestServerMuteCategory(MusicCategory.Ambient, !var.GetBool());
	}
};