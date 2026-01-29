[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_DRIVEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_DRIVE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_Waypoint waypoint = RegisterWaypoint("WP_ENDING", "", "GETOUT");
		waypoint.m_iMaximumDrawDistance = 100;
		
		CreateMarkerPath("NAVIGATION_PATH_DOTS");
	}
};