[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_NightModeEditorAttribute : SCR_BaseEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return null;
		
		SCR_NightModeGameModeComponent nightModeComponent = SCR_NightModeGameModeComponent.Cast(gamemode.FindComponent(SCR_NightModeGameModeComponent));
		if (!nightModeComponent || !nightModeComponent.IsGlobalNightModeAllowed() || !nightModeComponent.CanEnableNightMode())
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(nightModeComponent.IsGlobalNightModeEnabled());
	}

	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gamemode = BaseGameMode.Cast(item);
		if (!gamemode)
			return;
		
		SCR_NightModeGameModeComponent nightModeComponent = SCR_NightModeGameModeComponent.Cast(gamemode.FindComponent(SCR_NightModeGameModeComponent));
		if (!nightModeComponent || !nightModeComponent.IsGlobalNightModeAllowed() || !nightModeComponent.CanEnableNightMode())
			return;
		
		nightModeComponent.EnableGlobalNightMode(var.GetBool(), playerID);
	}
};