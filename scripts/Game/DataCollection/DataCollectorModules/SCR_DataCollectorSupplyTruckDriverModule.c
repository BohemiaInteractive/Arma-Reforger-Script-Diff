[BaseContainerProps()]
class SCR_DataCollectorSupplyTruckDriverModule : SCR_DataCollectorModule
{	
	/* Currently unused
	protected ref map<int, IEntity> m_mTrackedDrivers = new map<int, IEntity>();
	
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;
		
		IEntity playerEntity = compartment.GetOccupant();
		if (!playerEntity)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(playerEntity);
		if (playerID == 0) // Non-player character
			return;
		
		SlotManagerComponent slotManagerComponent = SlotManagerComponent.Cast(targetEntity.FindComponent(SlotManagerComponent));
		if (!slotManagerComponent)
			return;
		
		array<EntitySlotInfo> infos = {};
		
		for (int i = 0, count = slotManagerComponent.GetSlotInfos(infos); i < count; i++)
		{
			IEntity attachedEntity = infos[i].GetAttachedEntity();
			if (!attachedEntity)
				continue;
			
			SCR_CampaignSuppliesComponent suppliesComponent = SCR_CampaignSuppliesComponent.Cast(attachedEntity.FindComponent(SCR_CampaignSuppliesComponent));
			if (suppliesComponent)
			{
				//Player playerID is a driver of a vehicle with supplies
				m_mTrackedDrivers.Insert(playerID, playerEntity);
				return;
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCompartmentLeft(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		BaseCompartmentSlot compartment = manager.FindCompartment(slotID, mgrID);
		if (!compartment)
			return;
		
		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(compartment.GetOccupant());
		if (playerID == 0) // Non-player character
			return;
		
		m_mTrackedDrivers.Remove(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void AddInvokers(IEntity player, bool spawned = true)
	{
		super.AddInvokers(player, spawned);
		if (!player)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
			
		compartmentAccessComponent.GetOnCompartmentEntered().Insert(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Insert(OnCompartmentLeft);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void RemoveInvokers(IEntity player)
	{
		super.RemoveInvokers(player);
		if (!player)
			return;
		
		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(player.FindComponent(SCR_CompartmentAccessComponent));
		
		if (!compartmentAccessComponent)
			return;
		
		compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnCompartmentEntered);
		compartmentAccessComponent.GetOnCompartmentLeft().Remove(OnCompartmentLeft);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		if (!controlledEntity)	
			controlledEntity = 	m_mTrackedDrivers.Get(playerID);
		
		super.OnPlayerDisconnected(playerID, controlledEntity);
		m_mTrackedDrivers.Remove(playerID);
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
		
		int playerID;
		if (from)
		{
			playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(from);
			m_mTrackedDrivers.Remove(playerID);
		}
			
		if (to)
		{
			playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(to);
			m_mTrackedDrivers.Insert(playerID, to);
		}
	}
#endif
	
	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeTick)
	{
		//If there's no data collector, do nothing
		if (!GetGame().GetDataCollector())
			return;
		
		m_fTimeSinceUpdate += timeTick;
		
		if (m_fTimeSinceUpdate<m_fTimeToUpdate)
			return;
				
		IEntity playerEntity;
		IEntity vehicle;
		Physics physics;
		float distanceTraveled;
		int playerId;
		SCR_PlayerData playerData;
				
		for (int i = m_mTrackedDrivers.Count() - 1; i >= 0; i--)
		{
			playerEntity = m_mTrackedDrivers.GetElement(i);
			vehicle = CompartmentAccessComponent.GetVehicleIn(playerEntity);
			if (!vehicle)
				continue;
			
			physics = vehicle.GetPhysics();
			if (!physics)
				continue;
			
			distanceTraveled = physics.GetVelocity().Length() * timeTick;
			playerId = m_mTrackedDrivers.GetKey(i);
			
			playerData = GetGame().GetDataCollector().GetPlayerData(playerId);
			playerData.AddTraveledDistanceSupplyVehicle(distanceTraveled);
			playerData.AddTraveledTimeSupplyVehicle(timeTick);
			
			//DEBUG display
			if (m_StatsVisualization)
			{
				m_StatsVisualization.Get(SCR_ESupplyTruckDriverModuleStats.METERSDRIVENSUPPLYVEHICLE).SetText(playerData.GetCurrentTraveledDistanceSupplyVehicle().ToString());
				m_StatsVisualization.Get(SCR_ESupplyTruckDriverModuleStats.TIMEDRIVENSUPPLYVEHICLE).SetText(playerData.GetCurrentTraveledTimeSupplyVehicle().ToString());
			}
		}
		m_fTimeSinceUpdate = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override void CreateVisualization()
	{
		super.CreateVisualization();
		if (!m_StatsVisualization)
			return;
		
		CreateEntry("Meters driven w Supply Vehicle: ", 0, SCR_ESupplyTruckDriverModuleStats.METERSDRIVENSUPPLYVEHICLE);
		CreateEntry("Time driving Supply Vehicle: ", 0, SCR_ESupplyTruckDriverModuleStats.TIMEDRIVENSUPPLYVEHICLE);
	}
	*/
};
/*
enum SCR_ESupplyTruckDriverModuleStats
{
	METERSDRIVENSUPPLYVEHICLE,
	TIMEDRIVENSUPPLYVEHICLE
};
*/