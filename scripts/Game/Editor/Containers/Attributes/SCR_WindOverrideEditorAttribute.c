/**
Weather attribute if it should override certain weather values
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WindOverrideEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(weatherManager.IsWindSpeedOverridden() || weatherManager.IsWindDirectionOverridden());
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_WindSpeedEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_WindDirectionEditorAttribute);
		}	
				
		bool overrideWeather = var && var.GetBool();
		
		manager.SetAttributeEnabled(SCR_WindSpeedEditorAttribute, overrideWeather);
		manager.SetAttributeEnabled(SCR_WindDirectionEditorAttribute, overrideWeather);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();

		//Is preview
		if (!item)
		{
			if (!weatherTransitionManager)
				return;
			
			//Has override so show preview
			if (var.GetBool())
			{
				//Set wind override preview
				SCR_BaseEditorAttributeVar windSpeedVar;
				if (!manager.GetAttributeVariable(SCR_WindSpeedEditorAttribute, windSpeedVar))
					return;
				
				SCR_BaseEditorAttributeVar windDirectionVar;
				if (!manager.GetAttributeVariable(SCR_WindDirectionEditorAttribute, windDirectionVar))
					return;		
			
				SCR_WindDirectionInfo windDirectionInfo;
				if (!weatherManager.GetWindDirectionInfoFromIndex(windDirectionVar.GetInt(), windDirectionInfo))
					return;
				
				weatherManager.SetWindPreview(true, windSpeedVar.GetFloat(),  windDirectionInfo.GetWindDirectionValue());
			}
			//Override false so disable preview
			else if (weatherTransitionManager.IsPreviewingWind())
			{
				weatherManager.SetWindPreview(false);
			}
			
			return;
		}
		
		//Cancel preview
		if (weatherTransitionManager && weatherTransitionManager.IsPreviewingWind())
			weatherManager.SetWindPreview(false);
		
		weatherManager.DelayedSetWindOverride(var.GetBool(), playerID);
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		//Set preview (Also called from SCR_WindSpeedEditorAttribute and SCR_WindDirectionEditorAttribute)
		if (setPreview)
		{
			WriteVariable(null, GetVariable(), manager, -1);
		}
		//Cancel preview (On reset)
		else
		{
			ChimeraWorld world = GetGame().GetWorld();
			TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
			if (!weatherManager) 
				return;
			
			BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			if (weatherTransitionManager.IsPreviewingWind())
				weatherManager.SetWindPreview(false);
		}
	}
};