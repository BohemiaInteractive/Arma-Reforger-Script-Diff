[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MOVEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MOVE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LOOKOUT");
	}
}