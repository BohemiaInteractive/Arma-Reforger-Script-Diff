//
/**
Respawn attribute to enable/disable respawn and respawn time, for getting and setting varriables in Editor Attribute window
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WeatherInstantTransitionTimeEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//~Todo: Disabled for now until instant transition can have time assigned to it
		return null;
		
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		/*SCR_TimeAndWeatherEditorComponent editorWeatherComponent = SCR_TimeAndWeatherEditorComponent.Cast(gameMode.FindComponent(SCR_TimeAndWeatherEditorComponent));
		if (!editorWeatherComponent)
			return null;*/
		
		//~Todo: Not implemented
		return null;
		//return SCR_BaseEditorAttributeVar.CreateInt(editorWeatherComponent.GetWeatherInstantTransitionTime());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		/*SCR_TimeAndWeatherEditorComponent editorWeatherComponent = SCR_TimeAndWeatherEditorComponent.Cast(gameMode.FindComponent(SCR_TimeAndWeatherEditorComponent));
		if (!editorWeatherComponent)
			return;*/
		
		//~Todo: Not implemented
		//editorWeatherComponent.SetWeatherInstantTransitionTime(var.GetInt());
	}	
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeEntryTimeSlider(1, false));
		return super.GetEntries(outEntries);
	}
};
