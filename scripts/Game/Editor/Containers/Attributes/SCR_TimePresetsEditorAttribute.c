/**
Daytime Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_TimePresetsEditorAttribute: SCR_BasePresetsEditorAttribute
{		
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(gameMode.GetWorld());
		if (!world)
			return null;
		
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) 
			return null;
		
	
		SCR_UIInfo uiInfo;
		return SCR_BaseEditorAttributeVar.CreateInt(timeManager.GetCurrentDayTimeUIInfo(uiInfo));
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var)
			return;
		
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_DaytimeEditorAttribute);
			return;
		}
			
		int index = var.GetInt();
		
		if (index < 0 || index >= m_aValues.Count())
			return;
		
		//Recreate presets in case date changed
		CreatePresets();
		
		//Set new time
		SCR_BaseEditorAttributeVar newTime = new SCR_BaseEditorAttributeVar();
		newTime.SetFloat(m_aValues[index].GetFloatValue());
		manager.SetAttributeVariable(SCR_DaytimeEditorAttribute, newTime);
	}
	
	//The Time preset doesn't actually set anything other then the slider
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		return;
	}
	
	override void CreatePresets()
	{	
		int year = -1;
		int month = 5;
		int day = 23;
		
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) 
			return;
		
		SCR_AttributesManagerEditorComponent manager = SCR_AttributesManagerEditorComponent.Cast(SCR_AttributesManagerEditorComponent.GetInstance(SCR_AttributesManagerEditorComponent));
		if (manager)
		{
			SCR_DateEditorAttribute dateAttribute = SCR_DateEditorAttribute.Cast(manager.GetAttributeRef(SCR_DateEditorAttribute));
			if (dateAttribute)
			{
				SCR_BaseEditorAttributeVar dateVar = dateAttribute.GetVariable();
				if (dateVar)
				{
					vector date = dateVar.GetVector();
					year = dateAttribute.GetYearByIndex(date[2]);
					month = date[1] +1;
					day = date[0] +1;
				}
			}
		}

		if (year < 0)
			timeManager.GetDate(year, month, day);
		
		m_aValues.Clear();
		
		array<SCR_DayTimeInfoBase> daytimeInfo = new array<SCR_DayTimeInfoBase>;
		timeManager.GetDayTimeInfoArray(daytimeInfo, year, month, day);
		SCR_EditorAttributeFloatStringValueHolder value
		float time;
		
		foreach (SCR_DayTimeInfoBase dayTime: daytimeInfo)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			
			time = dayTime.GetDayTime();
			if (time >= 24)
				time = 0;
			
			value.SetWithUIInfo(dayTime.GetDayTimeUIInfo(), time * 3600);
			m_aValues.Insert(value);
		}
	}
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!manager)
			return;
		
		SCR_BaseEditorAttribute timeAttribute = manager.GetAttributeRef(SCR_DaytimeEditorAttribute);
		if (timeAttribute)
			timeAttribute.PreviewVariable(setPreview, manager);
	}
};