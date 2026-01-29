[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_MOVE_BACKClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_MOVE_BACK : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_SW_FIRING_POSITION");
		
		if (m_StageInfo.GetStageName() == "PlayerInRange")
			PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 20);
	}
};