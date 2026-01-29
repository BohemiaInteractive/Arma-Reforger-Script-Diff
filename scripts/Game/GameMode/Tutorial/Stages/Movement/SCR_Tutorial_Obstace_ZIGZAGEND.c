[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_ZIGZAGENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_ZIGZAGEND : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_ZIGZAG_3");
	}
};