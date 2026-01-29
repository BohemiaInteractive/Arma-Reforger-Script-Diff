[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SetPropBaseBudgetEditorAttribute : SCR_BaseValueListEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity provider = editableEntity.GetOwner();
		if (!provider)
			return null;
		
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent(provider);
		if (!providerComponent)
			return null;
		
		//~ Send over current and max value as max value is used in UI to set slider max.
		return SCR_BaseEditorAttributeVar.CreateFloat((providerComponent.GetCurrentPropValue() / providerComponent.GetMaxBudgetValue(EEditableEntityBudget.PROPS)) * 100);
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return;
		
		IEntity provider = editableEntity.GetOwner();
		if (!provider)
			return;
		
		SCR_CampaignBuildingProviderComponent providerComponent = GetProviderComponent(provider);
		if (!providerComponent)
			return;
		
		providerComponent.SetPropValue(providerComponent.GetMaxBudgetValue(EEditableEntityBudget.PROPS) * (var.GetFloat() * 0.01));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Search for the provider component on given entity.
	SCR_CampaignBuildingProviderComponent GetProviderComponent(notnull IEntity entity)
	{
		array<IEntity> output = {};
		SCR_EntityHelper.GetHierarchyEntityList(entity, output);
		
		foreach (IEntity ent : output)
		{	
			SCR_CampaignBuildingProviderComponent providerComponent = SCR_CampaignBuildingProviderComponent.Cast(ent.FindComponent(SCR_CampaignBuildingProviderComponent));
			if  (providerComponent)
				return providerComponent;
		}

		return null;
	}
};