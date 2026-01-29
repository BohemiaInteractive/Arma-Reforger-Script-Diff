[BaseContainerProps()]
class SCR_CustomTriggerConditionsCheckVehicleEngineRunning : SCR_CustomTriggerConditions
{
	[Attribute(desc: "Checked Vehicle Getter")]
	ref SCR_ScenarioFrameworkGet m_VehicleGetter;
	
	[Attribute(defvalue: "1")]
	bool m_bIsEngineOn;
	
	//------------------------------------------------------------------------------------------------
	//! Performs specified logic checks when called
	//! \param[in] trigger
	override bool Init(IEntity entity)
	{
		if (m_bIsEngineOn)
			return CheckEnginesRunning(entity);
		
		return !CheckEnginesRunning(entity);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckEnginesRunning(IEntity entity)
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

		if (vehicles.IsEmpty())
			return false;
		
		VehicleControllerComponent vehicleController;
		foreach (Vehicle veh : vehicles)
		{
			vehicleController = VehicleControllerComponent.Cast(veh.FindComponent(VehicleControllerComponent));
			if (!vehicleController)
				continue;
			
			if (vehicleController.IsEngineOn())
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected IEntity ValidateInputEntity(SCR_ScenarioFrameworkGet getter)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework Action: Issue with Getter detected for Action %1.", this, level: LogLevel.ERROR);
			return null;
		}
		
		IEntity ent = entityWrapper.GetValue();
		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(ent.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (layerBase)
			return layerBase.GetSpawnedEntity();

		return entityWrapper.GetValue();
	}
}