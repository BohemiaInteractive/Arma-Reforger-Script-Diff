[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SetSuppliesBaseEditorAttribute : SCR_BaseValueListEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		if (!CanDisplay(editableEntity))
			return null;
		
		/*SCR_CampaignSuppliesComponent supplyComponent = SCR_CampaignSuppliesComponent.GetSuppliesComponent(editableEntity.GetOwner());
		if (!supplyComponent) 
			return null;*/
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (!base)
			return null;
		
		if (base.GetSuppliesMax() <= 0)
			return null;
		
		//~ Send over current and max value as max value is used in UI to set slider max
		return SCR_BaseEditorAttributeVar.CreateVector(Vector(base.GetSupplies(), base.GetSuppliesMax(), 0));
	}
	
	protected bool CanDisplay(SCR_EditableEntityComponent editableEntity)
	{
		//~ Ignore if vehicle
		return editableEntity.GetEntityType() != EEditableEntityType.VEHICLE;
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		/*SCR_CampaignSuppliesComponent supplyComponent = SCR_CampaignSuppliesComponent.GetSuppliesComponent(editableEntity.GetOwner());
		if (!supplyComponent) 
			return;*/
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (!base)
			return;
		
		vector currentAndMaxSupplies = var.GetVector();
		base.SetSupplies(currentAndMaxSupplies[0]);
	}
};