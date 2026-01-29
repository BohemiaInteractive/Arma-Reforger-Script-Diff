/**
Daytime Attribute for getting and setting varriables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_DaytimeEditorAttribute: SCR_BaseValueListEditorAttribute
{		
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		GenericEntity ent = GenericEntity.Cast(item);
		ChimeraWorld world = ent.GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) return null;
	
		return SCR_BaseEditorAttributeVar.CreateFloat(timeManager.GetTimeOfTheDay() * 3600);
	}
	
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (!var || isInit)
			return;
		
		//If slider moved set all deselected
		manager.SetAttributeSelected(SCR_TimePresetsEditorAttribute, false, -1);
	}
	
	//---- REFACTOR NOTE START: Hardcoded numbers 
	
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
		
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return;
		
		BaseWeatherStateTransitionManager weatherTransitionManager = timeManager.GetTransitionManager();
		if (!weatherTransitionManager)
			return;
		
		float daytime = var.GetFloat() / 3600;
		
		//Preview
		if (!item)
		{
			if (!manager)
				return;
			
			SCR_DateEditorAttribute dateAttribute = SCR_DateEditorAttribute.Cast(manager.GetAttributeRef(SCR_DateEditorAttribute));
			if (!dateAttribute)
				return;
			
			SCR_BaseEditorAttributeVar dateVar = dateAttribute.GetVariable();
			if (!dateVar)
				return;
			
			vector date = dateVar.GetVector();
			timeManager.SetDateTimePreview(true, dateAttribute.GetYearByIndex(date[2]), date[1] +1, date[0] +1, daytime / 24);
			
			return;
		}
		
		if (weatherTransitionManager.IsPreviewingDateTime())
			timeManager.SetDateTimePreview(false);
		
		timeManager.SetTimeOfTheDay(daytime);
		
		string time = SCR_FormatHelper.GetTimeFormatting(var.GetFloat(), (ETimeFormatParam.DAYS | ETimeFormatParam.SECONDS));
		
		//Notification
		if (item)
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_TIME_CHANGED, playerID,  time[0].ToInt(), time[1].ToInt(), time[3].ToInt(), time[4].ToInt());
	}
	
	//---- REFACTOR NOTE END ----
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
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
			if (weatherTransitionManager.IsPreviewingDateTime())
				weatherManager.SetDateTimePreview(false);
			
			float timeofDay = weatherManager.GetTimeOfTheDay();
			weatherManager.SetTimeOfTheDay(timeofDay, true);
		}
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		outEntries.Insert(new SCR_BaseEditorAttributeEntryTimeSlider(1, true));
		return super.GetEntries(outEntries);
	}
};
