[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_ENDClass: SCR_BaseTutorialStageClass
{
};


class SCR_Tutorial_Navigation_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 2);
		GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 3000, false, "VEHICLEMAINTENANCE_Instructor", 3);
		
		int finalTimeMS = Math.Floor(GetGame().GetWorld().GetWorldTime() - m_TutorialComponent.GetSavedTime());
		
		int msg;

		if (finalTimeMS < 140000)
			msg = 4;
		else if (finalTimeMS < 200000)
			msg = 5;
		else
			msg = 6;
		
		GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 8000, false, "VEHICLEMAINTENANCE_Instructor", msg);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TIME_TRIAL_GOOD_INSTRUCTOR_R_01" || m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TIME_TRIAL_OKAY_INSTRUCTOR_R_01" || m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TIME_TRIAL_POOR_INSTRUCTOR_R_03" || GetDuration() > 25000;
	}
};