[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_NORTHClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_NORTH : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_ALIGN_NORTH_INSTRUCTOR_Q_03")
			ShowHint();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent)
			return false;
		
		return m_TutorialComponent.IsPlayerAimingInAngle(0, 7);
	}
};