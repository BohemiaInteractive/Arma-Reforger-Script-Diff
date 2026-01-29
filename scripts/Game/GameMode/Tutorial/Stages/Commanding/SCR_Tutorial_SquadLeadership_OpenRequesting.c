[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_OpenRequestingClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SquadLeadership_OpenRequesting : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("UNIT_REQUESTING_BOARD", "", "CUSTOM").SetOffsetVector("0 1 0");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
}