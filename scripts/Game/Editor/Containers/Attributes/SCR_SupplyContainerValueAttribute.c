[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupplyContainerValueAttribute : SCR_BaseValueListEditorAttribute
{
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return null;

		SCR_ResourceComponent resourceComponent;
		SCR_ResourceContainer supplyContainer = GetContainer(entity, resourceComponent);
		if (!supplyContainer)
			return null;

		float currentValue = supplyContainer.GetResourceValue();
		float maxValue = supplyContainer.GetMaxResourceValue();

		if (maxValue == 0)
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat((currentValue / maxValue) * 100);
	}

	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return;

		SCR_ResourceComponent resourceComponent;
		SCR_ResourceContainer supplyContainer = GetContainer(entity, resourceComponent);
		if (!supplyContainer)
			return;

		float percentage = var.GetFloat();
		float maxValue = supplyContainer.GetMaxResourceValue();
		float newValue = maxValue * (percentage / 100);

		supplyContainer.SetResourceValue(newValue);
	}

	SCR_ResourceContainer GetContainer(notnull IEntity entity, out SCR_ResourceComponent component)
	{
		component = SCR_ResourceComponent.Cast(entity.FindComponent(SCR_ResourceComponent));
		SCR_ResourceContainer container;

		if (component)
		{
			container = component.GetContainer(EResourceType.SUPPLIES);
			if (container)
				return container;
		}

		IEntity iteratedEntity = entity.GetChildren();
		while (iteratedEntity)
		{
			component = SCR_ResourceComponent.Cast(iteratedEntity.FindComponent(SCR_ResourceComponent));
			if (component)
			{
				container = component.GetContainer(EResourceType.SUPPLIES);
				if (container)
					return container;
			}

			iteratedEntity = iteratedEntity.GetSibling();
		}

		return null;
	}
}