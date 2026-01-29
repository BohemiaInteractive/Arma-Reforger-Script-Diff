class SCR_PlayerSupplyAllocationComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_PlayerSupplyAllocationComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		auto supplyAllocation = SCR_PlayerSupplyAllocationComponent.Cast(component);

		const int maxAllocation = supplyAllocation.GetPlayerMilitarySupplyAllocation();
		const int availableAllocation = supplyAllocation.GetPlayerAvailableAllocatedSupplies();

		if (maxAllocation == 0 && availableAllocation == 0)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.Write(maxAllocation);
		context.Write(availableAllocation);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto supplyAllocation = SCR_PlayerSupplyAllocationComponent.Cast(component);

		int version;
		context.Read(version);

		int maxAllocation;
		if (context.Read(maxAllocation))
			supplyAllocation.SetPlayerMilitarySupplyAllocation(maxAllocation);
		
		int availableAllocation;
		if (context.Read(availableAllocation))
			supplyAllocation.AddPlayerAvailableAllocatedSupplies(availableAllocation - supplyAllocation.GetPlayerAvailableAllocatedSupplies());

		return true;
	}
}
