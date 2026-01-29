[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginOnDestroyEvent : SCR_ScenarioFrameworkPlugin
{
	[Attribute(UIWidgets.Auto, desc: "What to do once object gets destroyed", category: "OnDestroy")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActionsOnDestroy;

	IEntity m_Asset;

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework layer, registers damage state change event, checks for vehicle engine stop event, and schedules engine drowning action
	//! \param[in] object of layer base from which entity is further retrieved
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		IEntity entity = object.GetSpawnedEntity();
		if (!entity)
		{
			Print("ScenarioFramework: No entity was spawned by layer to add OnDestroy event to", LogLevel.ERROR);
			return;
		}

		m_Asset = entity;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Asset);
		if (!group)
		{
			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
			if (objectDmgManager)
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
			else
				PrintFormat("ScenarioFramework: Registering OnDestroy of entity %1 failed! The entity doesn't have damage manager", entity, LogLevel.ERROR);
	
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
	
	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnDestroy)
		{
			action.OnActivate(m_Asset);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state OnObjectDamage state represents the current damage state of an object in the game, used to determine if an object has been
	void OnObjectDamage(EDamageState state)
	{
		// Here you can debug specific Plugin instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Plugin Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkPluginOnDestroyEvent.OnObjectDamage] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
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

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if engine is downed, removes related event handlers, and activates actions on destroy if engine is drown
	void CheckEngineDrowned()
	{
		// Here you can debug specific Plugin instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Plugin Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkPluginOnDestroyEvent.CheckEngineDrowned] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
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

			OnActivate();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupEmpty(AIGroup group)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(group);
		if (!aiGroup)
			return;
		
		aiGroup.GetOnEmpty().Remove(OnGroupEmpty);
		OnActivate();
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetActions()
	{
		return m_aActionsOnDestroy;
	}
}