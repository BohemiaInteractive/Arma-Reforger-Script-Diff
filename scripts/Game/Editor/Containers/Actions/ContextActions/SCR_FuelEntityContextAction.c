//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_FuelEntityContextAction : SCR_SelectedEntitiesContextAction
{
	private bool GetFuelManagerComponent(SCR_EditableEntityComponent entity, out FuelManagerComponent fuelManagerComponent)
	{
		if(!entity)
		{
			return false;
		}
		fuelManagerComponent = FuelManagerComponent.Cast(entity.GetOwner().FindComponent(FuelManagerComponent));
		return fuelManagerComponent != null;
	}
	
	private bool IsAlive(SCR_EditableEntityComponent entity)
	{
		if (!entity)
			return false;
		
		DamageManagerComponent damageComponent = DamageManagerComponent.Cast(entity.GetOwner().FindComponent(DamageManagerComponent));
		if (!damageComponent)
			return true;
		
		return damageComponent.GetState() != EDamageState.DESTROYED;
	}
	
	override bool CanBeShown(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		FuelManagerComponent fuelManagerComponent;
		array<BaseFuelNode> fuelNodes = new array<BaseFuelNode>;
		return GetFuelManagerComponent(selectedEntity, fuelManagerComponent) && fuelManagerComponent.GetFuelNodesList(fuelNodes) > 0;
	}
	
	override bool CanBePerformed(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition, int flags)
	{
		FuelManagerComponent fuelManagerComponent;
		if(IsAlive(selectedEntity) && GetFuelManagerComponent(selectedEntity, fuelManagerComponent) && fuelManagerComponent.CanBeRefueled())
		{
			return true;
		}
		
		return false;
	}	
	
	override void Perform(SCR_EditableEntityComponent selectedEntity, vector cursorWorldPosition)
	{
		FuelManagerComponent fuelManagerComponent;
		if(GetFuelManagerComponent(selectedEntity, fuelManagerComponent))
		{
			array<BaseFuelNode> nodes = {};
			fuelManagerComponent.GetFuelNodesList(nodes);
			SCR_FuelNode fuelNode;
			foreach (BaseFuelNode node : nodes)
			{
				node.SetFuel(node.GetMaxFuel());
			}
		}
	}
};
