//Baseclass that is supposed to be extended and filled with custom conditions
[BaseContainerProps()]
class SCR_CustomTriggerConditionsCheckVehicleForAI : SCR_CustomTriggerConditions
{
	[Attribute(desc: "AI or Group to check for.")]
	ref SCR_ScenarioFrameworkGet m_AIGetter;

	[Attribute(desc: "Checked Vehicle Getter")]
	ref SCR_ScenarioFrameworkGet m_VehicleGetter;

	//------------------------------------------------------------------------------------------------
	//! Performs specified logic checks when called
	//! \param[in] trigger
	override bool Init(IEntity entity)
	{
		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		if (!trigger)
			return false;

		array<IEntity> entities = {};
		trigger.GetEntitiesInside(entities);

		if (!entities || entities.IsEmpty())
			return false;

		//preparing vehicles
		array<Vehicle> vehicles = {};
		
		Vehicle vehicleEnt;
		if (m_VehicleGetter)
		{
			vehicleEnt = Vehicle.Cast(ValidateInputEntity(m_VehicleGetter));
			if (!vehicleEnt)
				return false;
			
			vehicles.Insert(vehicleEnt);
		}
		else
		{
			foreach (IEntity ent : entities)
			{
				vehicleEnt = Vehicle.Cast(ent);
				if (vehicleEnt)
					vehicles.Insert(vehicleEnt);
			}
		}

		//preparing characters
		array<ChimeraCharacter> characters = {};

		if (m_AIGetter)
		{
			IEntity aiEnt = ValidateInputEntity(m_AIGetter);
			if (!aiEnt)
				return false;
			
			AIGroup group = AIGroup.Cast(aiEnt);
			if (group)
			{
				array<AIAgent> agents = {};
				group.GetAgents(agents);

				foreach (AIAgent agent : agents)
				{
					characters.Insert(ChimeraCharacter.Cast(agent.GetControlledEntity()));
				}
			}
			else
			{
				ChimeraCharacter char = ChimeraCharacter.Cast(aiEnt);
				if (!char)
					return false;
				
				characters.Insert(char);
			}
		}
		else
		{
			ChimeraCharacter character;
			foreach (IEntity ent : entities)
			{
				character = ChimeraCharacter.Cast(ent);
				if (character)
					characters.Insert(character);
			}
		}

		if (vehicles.IsEmpty() || characters.IsEmpty())
			return false;
		
		IEntity parent;
		foreach (ChimeraCharacter char : characters)
		{
			parent = SCR_EntityHelper.GetMainParent(char);
			if (!parent)
				return false;
			
			foreach (Vehicle veh : vehicles)
			{
				if (veh == parent)
					break;
				
				return false;
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity ValidateInputEntity(SCR_ScenarioFrameworkGet getter)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			
			return null;
		}
		
		IEntity ent = entityWrapper.GetValue();
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(ent.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (layerBase)
			return layerBase.GetSpawnedEntity();

		return entityWrapper.GetValue();
	}
}
