[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_CloseRequestingClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_SquadLeadership_CloseRequesting : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !IsBuildingModeOpen();
	}
}