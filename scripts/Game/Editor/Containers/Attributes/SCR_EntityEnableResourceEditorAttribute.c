[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EntityEnableResourceEditorAttribute : SCR_BaseEditorAttribute
{
	[Attribute(EResourceType.SUPPLIES.ToString(), desc: "Which resource is set enabled/disabled", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EResourceType))]
	protected EResourceType m_eResourceToSetEnabled;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(SCR_EntityHelper.FindComponent(editableEntity.GetOwner(), SCR_ResourceComponent));
		if (!resourceComponent || !resourceComponent.CanResourceTypeEnabledBeChanged(m_eResourceToSetEnabled))
			return null;
		
		//~ Does not use available resources
		float availableResources;
		
		if (!SCR_ResourceSystemHelper.GetAvailableResources(resourceComponent, availableResources))
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateBool(resourceComponent.IsResourceTypeEnabled(m_eResourceToSetEnabled));
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{
		if (isInit)
			manager.SetAttributeEnabled(SCR_EntityEnableResourceEditorAttribute, SCR_ResourceSystemHelper.IsGlobalResourceTypeEnabled(m_eResourceToSetEnabled));
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{						
		if (!var)
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(SCR_EntityHelper.FindComponent(editableEntity.GetOwner(), SCR_ResourceComponent));
		if (!resourceComponent || !resourceComponent.CanResourceTypeEnabledBeChanged(m_eResourceToSetEnabled))
			return;
		
		//~ Does not use available resources
		float availableResources;
		
		if (!SCR_ResourceSystemHelper.GetAvailableResources(resourceComponent, availableResources))
			return;
		
		resourceComponent.SetResourceTypeEnabled(var.GetBool(), m_eResourceToSetEnabled);
	}
};