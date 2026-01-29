/**
Entity Fuel Attribute for getting and setting variables in Editor Attribute window
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FuelEditorAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	protected SCR_EFuelNodeTypeFlag GetFuelNodeFlags()
	{
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_EFuelNodeTypeFlag GetFuelNodeIgnoreFlags()
	{
		return SCR_EFuelNodeTypeFlag.IS_FUEL_STORAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity) 
			return null;
		
		IEntity owner =  editableEntity.GetOwner();
		if (!owner) 
			return null;
		
		//Don't show if destroyed
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
			return null;
		
		//~ Get all fuel managers
		array<SCR_FuelManagerComponent> fuelManagers = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(owner, fuelManagers);
		if (fuelManagers.IsEmpty()) 
			return null;
		
		float totalFuel, maxFuel, fuelPercentage;
		SCR_FuelManagerComponent.GetTotalValuesOfFuelNodesOfFuelManagers(fuelManagers, totalFuel, maxFuel, fuelPercentage,  GetFuelNodeFlags(), GetFuelNodeIgnoreFlags());
		if (maxFuel <= 0)
			return null;

		return SCR_BaseEditorAttributeVar.CreateFloat(Math.Round(fuelPercentage * 100));
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		IEntity owner =  editableEntity.GetOwner();
		if (!owner)
			return;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));
		if (damageComponent && damageComponent.GetState() == EDamageState.DESTROYED)
			return;

		array<SCR_FuelManagerComponent> fuelManagers = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(owner, fuelManagers);
		if (fuelManagers.IsEmpty()) 
			return;
				
		SCR_FuelManagerComponent.SetTotalFuelPercentageOfFuelManagers(fuelManagers, var.GetFloat() * 0.01, GetFuelNodeFlags(), GetFuelNodeIgnoreFlags());
	}
};
