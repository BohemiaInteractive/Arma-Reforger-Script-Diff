[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_ZIGZAGSTARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_ZIGZAGSTART : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_ZIGZAG_1");
		
		m_TutorialComponent.SetSavedTime(GetGame().GetWorld().GetWorldTime());

		ChimeraWorld world = GetGame().GetWorld();
			
		if (world)
			world.GetMusicManager().Play("SOUND_OBSTACLE_COURSE");
	}
};