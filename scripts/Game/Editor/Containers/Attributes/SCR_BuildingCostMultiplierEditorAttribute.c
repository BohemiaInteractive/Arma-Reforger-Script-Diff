/*
The multiplier of building cost
*/
/*[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_BuildingCostMultiplierEditorAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		SCR_CampaignBuildingManagerEntity buildingManagerEnt = SCR_CampaignBuildingManagerEntity.GetInstance();
		if (!buildingManagerEnt) return null;
		
		float value = buildingManagerEnt.GetCostMultiplier();
		
		return SCR_BaseEditorAttributeVar.CreateFloat(value);
	}
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		
		SCR_CampaignBuildingManagerEntity buildingManagerEnt = SCR_CampaignBuildingManagerEntity.GetInstance();
		if (!buildingManagerEnt) return;
		
		float value = var.GetFloat();		
		buildingManagerEnt.SetCostMultiplier(value);
	}
};*/