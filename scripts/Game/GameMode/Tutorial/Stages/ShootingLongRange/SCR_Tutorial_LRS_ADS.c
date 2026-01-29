[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_ADSClass: SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_LRS_ADS : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		CharacterControllerComponent charController = m_Player.GetCharacterController();
		
		if (charController)
			return charController.IsWeaponADS();
		else
			return true;
	}
}