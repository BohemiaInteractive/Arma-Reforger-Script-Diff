[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_LOCATIONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_LOCATION : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_MapEntity.GetOnMapOpen().Remove(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapOpen().Insert(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 12);
		GetGame().GetCallqueue().CallLater(AddMarker, 60000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_AM_EASTINGS_AND_NORTHINGS_INSTRUCTOR_Q_05")
			PlayNarrativeCharacterStage("NAVIGATION_Instructor", 13);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_TutorialComponent.GetIsMapOpen() && (m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_DRIVE_TO_POSITION_INSTRUCTOR_Q_03" || GetDuration() > 39000);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddMarker()
	{
		CreateMarkerCustom(GetGame().GetWorld().FindEntityByName("WP_ENDING"), SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE, "032-028");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Navigation_LOCATION()
	{
		SCR_TutorialLogic_Navigation logic = SCR_TutorialLogic_Navigation.Cast(m_Logic);
		if (logic)
			GetGame().GetCallqueue().Remove(logic.CheckForNavigationItems);
			
		GetGame().GetCallqueue().Remove(PlayNarrativeCharacterStage);
		GetGame().GetCallqueue().Remove(AddMarker);
	}
};