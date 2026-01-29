[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_Enter_BuildingClass : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_Seizing_Enter_Building : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("SEIZING_BuildingService", "", "CUSTOM").SetOffsetVector("0 1 0");
		
		IEntity ent = GetGame().GetWorld().FindEntityByName("LIGHTHOUSE_SUPPLIES");
		if (!ent)
			return;

		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(ent);
		if (!resComp)
			return;

		SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
		if (resourceContainer)
			resourceContainer.IncreaseResourceValue(400);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return IsBuildingModeOpen();
	}
}