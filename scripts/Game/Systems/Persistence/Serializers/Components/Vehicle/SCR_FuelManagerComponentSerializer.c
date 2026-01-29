class SCR_SerializedFuelNode
{
	int tankId;
	float fuel;
}

class SCR_FuelManagerComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_FuelManagerComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const SCR_FuelManagerComponent fuelManager = SCR_FuelManagerComponent.Cast(component);
		
		array<ref SCR_SerializedFuelNode> saveNodes();
		array<SCR_FuelNode> outScriptedNodes();
		fuelManager.GetScriptedFuelNodesList(outScriptedNodes);
		foreach (SCR_FuelNode fuelNode : outScriptedNodes)
		{
			if (float.AlmostEqual(fuelNode.GetFuel(), fuelNode.GetInitialFuelTankState()))
				continue;
			
			SCR_SerializedFuelNode saveNode();
			saveNode.tankId = fuelNode.GetFuelTankID();
			saveNode.fuel = fuelNode.GetFuel();
			saveNodes.Insert(saveNode);
		}

		if (saveNodes.IsEmpty())
			return ESerializeResult.DEFAULT;
		
		context.WriteValue("version", 1);
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("fuelNodes", saveNodes);
		context.EnableTypeDiscriminator(prev);
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		auto fuelManager = SCR_FuelManagerComponent.Cast(component);
		
		array<SCR_FuelNode> outScriptedNodes();
		fuelManager.GetScriptedFuelNodesList(outScriptedNodes);
		
		int version = -1;
		context.ReadValue("version", version);
		
		array<ref SCR_SerializedFuelNode> saveNodes();
		const bool prev = context.EnableTypeDiscriminator(false);
		context.ReadValue("fuelNodes", saveNodes);
		context.EnableTypeDiscriminator(prev);
		
		foreach (int idx, SCR_SerializedFuelNode savedNode : saveNodes)
		{
			// Try direct idx access
			if (outScriptedNodes.IsIndexValid(idx))
			{
				SCR_FuelNode fuelNode = outScriptedNodes[idx];
				if (fuelNode.GetFuelTankID() == savedNode.tankId)
				{
					fuelNode.SetFuel(savedNode.fuel);
					break;
				}
			}
			
			// Try others and match tank id
			foreach (int innerIdx, SCR_FuelNode fuelNode : outScriptedNodes)
			{
				if (innerIdx == idx)
					continue; // Already tried via direct access
				
				if (fuelNode.GetFuelTankID() == savedNode.tankId)
				{
					fuelNode.SetFuel(savedNode.fuel);
					break;
				}
			}
			
			PrintFormat("Failed to apply fuel save-data. Could not locate tankId:%1 on %2", savedNode.tankId, owner, level: LogLevel.DEBUG);
		}

		return true;
	}
}
