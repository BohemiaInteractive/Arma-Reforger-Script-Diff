// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_AutomatedWeatherEditorAttribute : SCR_BaseEditorAttribute
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
		
		BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return null;
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		
		//Get weather states
		weatherManager.GetWeatherStatesList(weatherStates);
		if (weatherStates.IsEmpty()) 
			return null;

		WeatherStateTransitionNode currentStateTransition = weatherTransitionManager.GetCurrentStateTransitionNode();
		
		return SCR_BaseEditorAttributeVar.CreateBool(!currentStateTransition.IsLooping());
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_WeatherInstantEditorAttribute);
		
		manager.SetAttributeEnabled(SCR_WeatherInstantEditorAttribute, var && !var.GetBool());
	}
	
	//---- REFACTOR NOTE START: Hard coded state duration for ForceWeatherTo() is unclear 
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		//~ Ignore previews (!item)
		if (!var || !item) 
			return;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return;
		
		if (!manager)
			return;
		
		WeatherStateTransitionNode currentStateTransition = weatherTransitionManager.GetCurrentStateTransitionNode();
		
		bool setAutomatic = var.GetBool();

		//Remove preview if any was set
		if (weatherTransitionManager.IsPreviewingState())
			weatherManager.SetWeatherStatePreview(false);
		
		//~ Set looping false
		if (setAutomatic)
		{
			weatherManager.ForceWeatherTo(!setAutomatic, string.Empty, 0, 0.001, playerID);
		}
		//~ Set looping true
		else 
		{
			float transitionTime = 0;
			
			SCR_BaseEditorAttributeVar transitionVar;
			if (manager.GetAttributeVariable(SCR_WeatherInstantTransitionTimeEditorAttribute, transitionVar))
				transitionTime = transitionVar.GetInt();
			
			weatherManager.ForceWeatherTo(!setAutomatic, string.Empty, transitionTime / 3600, 0.001, playerID);
		}
	}
	
	//---- REFACTOR NOTE END ----
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!GetVariable())
			return;
		
		bool SetAutomatic = GetVariable().GetBool();
		
		if (SetAutomatic || !setPreview)
		{
			ChimeraWorld world = GetGame().GetWorld();
			TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
			if (!weatherManager) 
				return;
	
			BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			//Remove preview if any was set
			if (weatherTransitionManager.IsPreviewingState())
				weatherManager.SetWeatherStatePreview(false);
		}
		else 
		{
			SCR_BaseEditorAttribute weatherInstantAttribute = manager.GetAttributeRef(SCR_WeatherInstantEditorAttribute);
			if (!weatherInstantAttribute)
				return;
		
			weatherInstantAttribute.PreviewVariable(setPreview, manager);
		}
	}
};