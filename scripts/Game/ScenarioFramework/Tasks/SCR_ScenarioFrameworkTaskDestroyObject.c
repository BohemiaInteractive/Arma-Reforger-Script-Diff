class SCR_TaskDestroyObjectClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_TaskDestroyObject : SCR_ScenarioFrameworkTask
{	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state Checks if object is destroyed, removes damage state change listener, removes engine stop listener, and finishes support entity
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset)
			return;
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
		{
			objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(CheckEngineDrowned);
			
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
		}
				
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);				
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if engine is downed, removes related event handlers, stops vehicle, and finishes support entity task if drowned
	void CheckEngineDrowned()
	{
		if (!m_Asset)
			return;
		
		VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
		if (vehicleController && vehicleController.GetEngineDrowned())
		{
			vehicleController.GetOnEngineStop().Remove(CheckEngineDrowned);
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(CheckEngineDrowned);
			
			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
			if (objectDmgManager)
		 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
			
			m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rehooks task asset, sets up damage state change listener, checks engine stop after 5 seconds if vehicle.
	//! \param[in] object Rehooks task asset, handles damage state changes, checks engine stop after 5 seconds.
	override void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.HookTaskAsset(object);
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		
		if (Vehicle.Cast(m_Asset))
		{
			VehicleControllerComponent vehicleController = VehicleControllerComponent.Cast(m_Asset.FindComponent(VehicleControllerComponent));
			if (vehicleController)
				vehicleController.GetOnEngineStop().Insert(CheckEngineDrowned);
			//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
			
			// Since there is no invoker and no reliable way how to tackle drowned vehicles, in order to make it reliable,
			// We cannot solely rely on GetOnEngineStop because vehicle could have been pushed/moved into the water without started engine.
			SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(CheckEngineDrowned, 5000, true);
			
			//---- REFACTOR NOTE END ----
		}
	}
}
