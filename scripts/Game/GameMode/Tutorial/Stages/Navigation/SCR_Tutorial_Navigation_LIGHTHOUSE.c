[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_LIGHTHOUSEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_LIGHTHOUSE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent)
			return false;
		
		return m_TutorialComponent.IsPlayerAimingInAngle(220, 5);
	}
};