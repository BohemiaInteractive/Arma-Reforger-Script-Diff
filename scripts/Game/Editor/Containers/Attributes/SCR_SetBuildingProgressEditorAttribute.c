[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SetBuildingProgressEditorAttribute : SCR_BaseValueListEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		SCR_CampaignBuildingLayoutComponent layoutComponent = GetLayoutComponent(editableEntity);
		if (!layoutComponent)
			return null;
		
		if (layoutComponent.GetToBuildValue() <= 0)
			return null;
		
		//~ Send over current and max value as max value is used in UI to set slider max.
		return SCR_BaseEditorAttributeVar.CreateFloat((layoutComponent.GetCurrentBuildValue() / layoutComponent.GetToBuildValue()) * 100);
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_CampaignBuildingLayoutComponent GetLayoutComponent(notnull SCR_EditableEntityComponent editableEntity)
	{
		IEntity child = editableEntity.GetOwner().GetChildren();
		SCR_CampaignBuildingLayoutComponent layoutComponent;
		while (child)
		{
			// The preview exist at this moment but will be deleted, skip it.
			layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(child.FindComponent(SCR_CampaignBuildingLayoutComponent));
			if (layoutComponent)
				break;
						
			child = child.GetSibling();
		}
		
		return layoutComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		SCR_CampaignBuildingLayoutComponent layoutComponent = GetLayoutComponent(editableEntity);
		if (!layoutComponent) 
			return;
		
		layoutComponent.SetBuildingValue(layoutComponent.GetToBuildValue() * (var.GetFloat() * 0.01));
	}
};