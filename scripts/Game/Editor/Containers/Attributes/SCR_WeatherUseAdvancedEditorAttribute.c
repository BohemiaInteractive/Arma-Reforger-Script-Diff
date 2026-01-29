//SCR_WeatherUseAdvancedEditorAttribute
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WeatherUseAdvancedEditorAttribute : SCR_BaseEditorAttribute
{	
	/*override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//Disabled for now
		return null;
		
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		SCR_TimeAndWeatherEditorComponent editorWeatherComponent = SCR_TimeAndWeatherEditorComponent.Cast(gameMode.FindComponent(SCR_TimeAndWeatherEditorComponent));
		if (!editorWeatherComponent)
			return null;
	
		return SCR_BaseEditorAttributeVar.CreateBool(editorWeatherComponent.GetUseAdvancedWeather());
	}
	
	//Disable respawn time if respawning is disabled
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{			
		bool useAdvanced = false;
	
		if (var)
	 		useAdvanced = var.GetBool();
		
		manager.SetAttributeEnabled(SCR_WeatherInstantTransitionTimeEditorAttribute, var && !useAdvanced);
		manager.SetAttributeEnabled(SCR_WeatherInstantEditorAttribute, var && !useAdvanced);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_TimeAndWeatherEditorComponent editorWeatherComponent = SCR_TimeAndWeatherEditorComponent.Cast(gameMode.FindComponent(SCR_TimeAndWeatherEditorComponent));
		if (!editorWeatherComponent)
			return;
		
		editorWeatherComponent.SetUseAdvancedWeather(var.GetBool());
		
	}*/
};