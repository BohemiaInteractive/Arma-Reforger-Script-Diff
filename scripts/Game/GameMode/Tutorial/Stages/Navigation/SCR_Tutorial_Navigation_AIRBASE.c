[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_AIRBASEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_AIRBASE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 8);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent)
			return false;
		
		return m_TutorialComponent.IsPlayerAimingInAngle(295, 5);
	}
};