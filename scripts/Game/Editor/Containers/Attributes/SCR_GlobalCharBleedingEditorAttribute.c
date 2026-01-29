[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GlobalCharBleedingEditorAttribute : SCR_BaseValueListEditorAttribute

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
		
		return SCR_BaseEditorAttributeVar.CreateFloat(gameModeHealthSettings.GetBleedingScale());
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
		
		gameModeHealthSettings.SetBleedingScale(var.GetFloat());
	}
};