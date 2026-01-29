[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupplyContainerDecayAttribute : SCR_BaseEditorAttribute
{
	[Attribute()]
	float m_fDefaultDecayTickRate;

	[Attribute()]
	float m_fDefaultDecayValue;

	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return null;

		RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(entity);
		if (!resourceComponent)
			return null;

		SCR_ResourceContainer supplyContainer = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		//:| Dont display Attribute for encapsulated Containers.
		if (!supplyContainer || supplyContainer.IsEncapsulated())
			return null;

		return SCR_BaseEditorAttributeVar.CreateBool(supplyContainer.IsResourceDecayEnabled());
	}

	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		SCR_ResourceContainer supplyContainer = GetResourceContainer(item);

		//:| Dont display Attribute for encapsulated Containers.
		if (!supplyContainer || supplyContainer.IsEncapsulated())
			return;

		bool enabled = var.GetBool();
		supplyContainer.EnableDecay(enabled);

		if (!enabled)
			return;

		if (supplyContainer.GetResourceDecayTickrate() <= 0)
			supplyContainer.SetResourceDecayTickrate(m_fDefaultDecayTickRate);

		if (supplyContainer.GetResourceDecay() <= 0)
			supplyContainer.SetResourceDecay(m_fDefaultDecayValue);
	}
	
	SCR_ResourceContainer GetResourceContainer(Managed item)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity)
			return null;

		GenericEntity entity = editableEntity.GetOwnerScripted();
		if (!entity)
			return null;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(entity);
		if (!resourceComponent)
			return null;

		return resourceComponent.GetContainer(EResourceType.SUPPLIES);
	}
}
