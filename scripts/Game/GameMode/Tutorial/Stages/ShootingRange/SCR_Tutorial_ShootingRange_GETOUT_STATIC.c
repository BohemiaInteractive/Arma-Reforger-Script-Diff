[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_GETOUT_STATICClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_GETOUT_STATIC : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 15);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Player.GetParent())
		{
			ShowHint(1);
			return true;
		}
		
		return false;
	}
};