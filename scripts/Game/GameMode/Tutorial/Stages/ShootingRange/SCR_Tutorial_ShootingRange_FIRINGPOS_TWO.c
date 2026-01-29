[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_FIRINGPOS_TWOClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_FIRINGPOS_TWO : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("WP_FIREPOZ_2", "", "GUNREADY");

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 9);
	}
};