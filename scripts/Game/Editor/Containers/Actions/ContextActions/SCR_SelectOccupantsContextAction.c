[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_SelectOccupantsContextAction : SCR_SelectedEntitiesContextAction
{
	protected bool EntityIsOccupied(GenericEntity vehicleEntity)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(BaseCompartmentManagerComponent));	
		if (!compartmentManager) return false;
		
		array<BaseCompartmentSlot> compartments = new array<BaseCompartmentSlot>;
		compartmentManager.GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot slot : compartments)
		{
			if (slot.GetOccupant() != null || slot.AttachedOccupant() != null)
			{
				return true;
			}
		}
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		return selectedEntity != null && selectedEntity.GetEntityType() == EEditableEntityType.VEHICLE;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		GenericEntity owner = selectedEntity.GetOwner();		
		return owner != null && EntityIsOccupied(owner);
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner)
			return;
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return;
		
		SCR_BaseEditableEntityFilter filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.SELECTED);
		if (!filter)
			return;
		
		array<BaseCompartmentSlot> compartments = new array<BaseCompartmentSlot>;
		compartmentManager.GetCompartments(compartments);
		
		set<SCR_EditableEntityComponent> toSelect = new set<SCR_EditableEntityComponent>();
		foreach	(BaseCompartmentSlot slot: compartments)
		{
			IEntity occupant = slot.GetOccupant();
			if (!occupant)
				occupant = slot.AttachedOccupant();
			
			if (!occupant)
				continue;
			
			SCR_EditableEntityComponent editableOccupant = SCR_EditableEntityComponent.GetEditableEntity(occupant);
			if (editableOccupant)
				toSelect.Insert(editableOccupant);
		}
		
		filter.Replace(toSelect);
	}
};