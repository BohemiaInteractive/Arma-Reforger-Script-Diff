[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_FLAGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_FLAG : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_FLAG");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		// Delayed call so the line doesn't get cut off
		GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 1500, false, "OBSTACLECOURSE_Instructor", 9);
	}
};