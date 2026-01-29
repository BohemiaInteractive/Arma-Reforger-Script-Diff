[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Exit_BuildingClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_Seizing_Exit_Building : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !IsBuildingModeOpen();
	}
}