[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_ISLANDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_ISLAND : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 7);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent)
			return false;
		
		return m_TutorialComponent.IsPlayerAimingInAngle(240, 5);
	}
};