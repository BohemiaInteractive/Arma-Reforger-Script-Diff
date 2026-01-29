class SCR_TaskDefendClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_TaskDefend : SCR_ScenarioFrameworkTask
{	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state Checks if object is destroyed, removes damage state change listener, removes engine stop listener, and fails support entity task
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
				
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.FAILED);			
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if engine is downed, removes related event handlers, and fails support entity task if engine is downed
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
			
			m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.FAILED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupEmpty(AIGroup group)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(group);
		if (!aiGroup)
			return;
		
		aiGroup.GetOnEmpty().Remove(OnGroupEmpty);
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.FAILED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rehooks task asset, sets up damage state change listener, checks engine stop after 5 seconds if vehicle.
	//! \param[in] object to be linked to this task.
	override void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.HookTaskAsset(object);
		
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Asset);
		if (!group)
		{
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
		else
		{
			group.GetOnEmpty().Insert(OnGroupEmpty);
		}	
	}
}
