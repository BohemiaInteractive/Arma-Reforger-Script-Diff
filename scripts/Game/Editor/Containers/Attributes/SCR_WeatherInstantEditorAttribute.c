/**
Weather Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_WeatherInstantEditorAttribute: SCR_BasePresetsEditorAttribute
{	
	[Attribute("MISSING NAME", desc: "Text shown if weather State has no weather name assigned")]
	protected LocalizedString m_sUnknownWeatherName;
	
	[Attribute("{4B4B51FACB828BF9}UI/Textures/Tasks/TaskIcons/96/Icon_Task_Unknown.edds", desc: "Icon used when weather is unknown")]
	protected ResourceName m_sUnknownWeatherIcon;
	
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
		
		WeatherState currentState = weatherManager.GetCurrentWeatherState();
		return SCR_BaseEditorAttributeVar.CreateInt(currentState.GetStateID());
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(gameMode.GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		int stateIndex = var.GetInt();
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		weatherManager.GetWeatherStatesList(weatherStates);
		
		//Valid state
		if (weatherStates.IsEmpty() || stateIndex >= weatherStates.Count())
			return;
		
		WeatherState weatherToSet = weatherStates[stateIndex];
				
		//Preview Weather
		if (!item)
		{
			weatherManager.SetWeatherStatePreview(true, weatherToSet.GetStateName());
			return;
		}
		
		float transitionTime = 0;
		
		SCR_BaseEditorAttributeVar transitionVar;
		if (manager && manager.GetAttributeVariable(SCR_WeatherInstantTransitionTimeEditorAttribute, transitionVar))
			transitionTime = transitionVar.GetInt();
		
		weatherManager.ForceWeatherTo(true, weatherToSet.GetStateName(), transitionTime / 3600, 0.001, playerID);
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		//Preview weather. Note that if advanced is turned on preview should disapear
		if (setPreview)
		{
			WriteVariable(null, GetVariable(), manager, -1);
		}
		else 
		{
			ChimeraWorld world = GetGame().GetWorld();
			TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
			if (!weatherManager) 
				return;
		
			BaseWeatherStateTransitionManager weatherTransitionManager = weatherManager.GetTransitionManager();
			if (!weatherTransitionManager)
				return;
			
			//Remove preview
			if (weatherTransitionManager.IsPreviewingState())
				weatherManager.SetWeatherStatePreview(false);
		}
	}
	
	protected override void CreatePresets()
	{
		m_aValues.Clear();
		
		SCR_EditorAttributeFloatStringValueHolder value;
		
		array<ref WeatherState> weatherStates = new array<ref WeatherState>;
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager)
			return;
		
		weatherManager.GetWeatherStatesList(weatherStates);
		
		int count = weatherStates.Count();
		string weatherName;
		ResourceName weatherIconPath;
		
		//Create preset list
		for (int i = 0; i < count; i++)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			
			weatherName = weatherStates[i].GetLocalizedName();
			weatherIconPath = weatherStates[i].GetIconPath();
			
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherName))
			{
				weatherName = m_sUnknownWeatherName;
				Print("WeatherState: '" +  weatherStates[i].GetStateID() + "' does not have weather localization name assigned!", LogLevel.ERROR);
			}
			
			
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(weatherIconPath))
			{
				weatherIconPath = m_sUnknownWeatherIcon;
				Print("WeatherState: '" +  weatherStates[i].GetStateID() + "' does not have weather icon assigned!", LogLevel.ERROR);
			}
				
			value.SetName(weatherName);
			value.SetIcon(weatherIconPath);
			value.SetFloatValue(weatherStates[i].GetStateID());
			
			m_aValues.Insert(value);
		}
	}
};