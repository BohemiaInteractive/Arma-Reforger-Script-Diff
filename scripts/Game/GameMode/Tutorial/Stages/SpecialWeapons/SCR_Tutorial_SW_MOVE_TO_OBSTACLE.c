[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_MOVE_TO_OBSTACLEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_MOVE_TO_OBSTACLE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 12);

		RegisterWaypoint("DESTRUCTABLE_OBSTACLE");
	}
};