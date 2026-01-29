class SCR_SalvageVehicleSlotInfo : EntitySlotInfo
{
	[Attribute("{FD4C5E5CD5C4051A}Prefabs/Vehicles/Core/VehicleSalvageActionEntity.et", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", desc: "The entity that is spawned and attached to the vehicle when it is destroyed.")]
	protected ResourceName m_sSalvageActionEntity;
	
	protected bool m_bIsListeningToAdditionalSettingsChanged;
	
	//------------------------------------------------------------------------------------------------
	// On vehicle destroyed, spawn the vehicle salvage action
	void OnDamageStateChanged(EDamageState state)
	{
		// Check if setting is enabled
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings)
			return;
		
		if (state != EDamageState.DESTROYED)
			return;
		
		if (!additionalGameSettings.IsVehicleSalvageEnabled())
		{
			m_bIsListeningToAdditionalSettingsChanged = true;
			additionalGameSettings.GetOnChangeAdditionalSettingsInvoker().Insert(OnAdditionalSettingsChanged);
			return;
		}
		
		SpawnSalvageEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAdditionalSettingsChanged()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner());
		if (!vehicle)
			return;
		
		SCR_DamageManagerComponent damageManager = vehicle.GetDamageManager();
		if (!damageManager)
			return;
		
		if (damageManager.GetState() != EDamageState.DESTROYED)
			return;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!additionalGameSettings || !additionalGameSettings.IsVehicleSalvageEnabled())
			return;
		
		m_bIsListeningToAdditionalSettingsChanged = false;
		additionalGameSettings.GetOnChangeAdditionalSettingsInvoker().Remove(OnAdditionalSettingsChanged);		
		
		SpawnSalvageEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SpawnSalvageEntity()
	{
		Resource resource = Resource.Load(m_sSalvageActionEntity);
		if (!resource.IsValid())
			return;
		
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(resource);
		
		AttachEntity(spawnedEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	void DelayedInit()
	{
		Vehicle vehicle = Vehicle.Cast(GetOwner());
		if (!vehicle)
			return;
		
		SCR_DamageManagerComponent damageManager = vehicle.GetDamageManager();
		if (!damageManager)
			return;
		
		damageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SalvageVehicleSlotInfo()
	{			
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if ((gameMode && !gameMode.IsMaster()) || (!gameMode && Replication.IsClient()))
			return;
		
		// This is so the vehicle has time to get created
		GetGame().GetCallqueue().CallLater(DelayedInit);
	}
	
	void ~SCR_SalvageVehicleSlotInfo()
	{	
		if (!m_bIsListeningToAdditionalSettingsChanged)
			return;
		
		SCR_AdditionalGameModeSettingsComponent additionalGameSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (additionalGameSettings)
		{
			m_bIsListeningToAdditionalSettingsChanged = false;
			additionalGameSettings.GetOnChangeAdditionalSettingsInvoker().Remove(OnAdditionalSettingsChanged);
		}		
	}
}
