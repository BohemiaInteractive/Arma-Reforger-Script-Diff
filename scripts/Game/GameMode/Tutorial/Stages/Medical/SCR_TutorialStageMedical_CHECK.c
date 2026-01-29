[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_CHECKClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_CHECK: SCR_BaseTutorialStage
{
	protected SCR_InspectCasualtyWidget m_CasualtyInspectDisplay;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		array<BaseInfoDisplay> infoDisplays = {};
		GetGame().GetPlayerController().GetHUDManagerComponent().GetInfoDisplays(infoDisplays);
		foreach (BaseInfoDisplay baseInfoDisplays : infoDisplays)
		{
			m_CasualtyInspectDisplay = SCR_InspectCasualtyWidget.Cast(baseInfoDisplays);
			if (m_CasualtyInspectDisplay)
				break;
		}
		
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("Victim"));
		PlayNarrativeCharacterStage("Victim", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_WOUNDED_REMOVED_PARTICIPANT_01")
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{		
		if (!m_CasualtyInspectDisplay)
			return false;

		return m_CasualtyInspectDisplay.IsActive();
	}
};