[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_EnterBuilding_1Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_EnterBuilding_1 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_TutorialLogic_CombatEngineering logic = SCR_TutorialLogic_CombatEngineering.Cast(m_Logic);
		if (logic)
			logic.HandleArlevilleSupplies(true);
		
		RegisterWaypoint("VEHICLE_REQUESTING_BOARD", "", "CUSTOM").SetOffsetVector("0 1 0");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
}