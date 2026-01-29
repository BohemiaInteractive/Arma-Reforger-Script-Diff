//
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_DisembarkContextAction : SCR_SelectedEntitiesContextAction
{
	protected bool IsCharacterInVehicle(SCR_EditableEntityComponent selectedEntity)
	{
		SCR_EditableCharacterComponent character = SCR_EditableCharacterComponent.Cast(selectedEntity);
		if (character && character.GetPlayerID() <= 0)
		{
			return character.GetVehicle();
		}
		return false;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;
		
		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner) 
			return false;		
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));	
		if (compartmentManager)
		{
			return true;
		}
		else
		{
			return IsCharacterInVehicle(selectedEntity);
		}
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		if (!selectedEntity)
			return false;

		GenericEntity owner = selectedEntity.GetOwner();
		if (!owner) 
			return false;
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));	
		if (compartmentManager)
		{
			array<CompartmentAccessComponent> crewCompartmentAccess = new array<CompartmentAccessComponent>;
			selectedEntity.GetCrew(crewCompartmentAccess);
		
			return !crewCompartmentAccess.IsEmpty();
		}
		else 
		{
			return IsCharacterInVehicle(selectedEntity);
		}
	}
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		if (!selectedEntity)
			return;
		
		array<CompartmentAccessComponent> crewCompartmentAccess = new array<CompartmentAccessComponent>;
		SCR_EditableCharacterComponent character = SCR_EditableCharacterComponent.Cast(selectedEntity);
		
		//~Todo: Bug: If All characters of the same group are selected (But not the group) then disembarking does not unregister the vehicle of the group
		//If character selected check if vehicle is still in use by another group memeber before removing it from group
		if (character && character.GetPlayerID() <= 0)
		{
			selectedEntity.GetCrew(crewCompartmentAccess, false);
			
			//Unregister vehicle
			character.RemoveUsableVehicle(character.GetVehicle().GetOwner());
			
			//Get out
			foreach (CompartmentAccessComponent compartmentAccess: crewCompartmentAccess)
			{
				if (compartmentAccess.CanGetOutVehicleViaDoor(-1))				
					compartmentAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
			}
			
			return;
		}
		
		//If vehicle is selected make sure to instantly remove itself from characters
		SCR_EditableVehicleComponent vehicle = SCR_EditableVehicleComponent.Cast(selectedEntity);		
		if (vehicle)
		{
			selectedEntity.GetCrew(crewCompartmentAccess, false);
			foreach (CompartmentAccessComponent compartmentAccess: crewCompartmentAccess)
			{
				character = SCR_EditableCharacterComponent.Cast(compartmentAccess.GetOwner().FindComponent(SCR_EditableCharacterComponent));
				
				//~ Ignore Players
				if (!character || character.GetPlayerID() > 0)
					continue
				
				//~Unregister vehicle
				character.RemoveUsableVehicle(vehicle.GetOwner(), false);
				
				if (compartmentAccess.CanGetOutVehicleViaDoor(-1))				
					compartmentAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
			}
			
			return;		
		}
	}
};
