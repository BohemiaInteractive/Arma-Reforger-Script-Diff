[EntityEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerSpawnPointClass : SCR_SpawnPointClass
{
}

class SCR_PlayerSpawnPoint : SCR_SpawnPoint
{
	[Attribute("1", desc: "How often will the spawn's position be updated to match assigned player's position (in seconds).", category: "Player Spawn Point")]
	protected float m_fUpdateInterval;
	
	[Attribute(desc: "Spawn point visualization. Original 'Info' attribute will be ignored.", category: "Player Spawn Point")]
	protected ref SCR_PlayerUIInfo m_PlayerInfo;
	
	[RplProp(onRplName: "OnSetPlayerID")]
	protected int m_iPlayerID;
	
	[RplProp()]
	protected bool m_bIsActive;
	
	protected Faction m_CachedFaction;
	protected IEntity m_TargetPlayer;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetTargetPlayer()
	{
		return m_TargetPlayer;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Assign player ID to this respawn point.
	//! It will then present itself as the player, and spawning on it will actually spawn the new player on position of assignd player.
	//! \param[in] playerID Target player ID
	void SetPlayerID(int playerID)
	{
		if (playerID == m_iPlayerID || !Replication.IsServer())
			return;
		
		//--- Set and broadcast new player ID
		m_iPlayerID = playerID;
		OnSetPlayerID();
		Replication.BumpMe();
		
		IEntity player = SCR_PossessingManagerComponent.GetPlayerMainEntity(m_iPlayerID);
		if (player)
			EnablePoint(m_iPlayerID, player);
		else
			DisablePoint(m_iPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get ID of the player this spawn point is assigned to.
	//! \return Target player ID
	int GetPlayerID()
	{
		return m_iPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSetPlayerID()
	{
		//--- Link player info
		if (!m_PlayerInfo)
			m_PlayerInfo = new SCR_PlayerUIInfo();
		
		m_PlayerInfo.SetPlayerID(m_iPlayerID);
		LinkInfo(m_PlayerInfo);
	}

	//------------------------------------------------------------------------------------------------
	protected override string GetSpawnPointName()
	{
		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] playerEntity
	void EnablePoint(int playerId, IEntity playerEntity)
	{
		if (playerId != m_iPlayerID)
		{
			Print(string.Format("Couldn't enable point, mismatching playerId(s), expected: %1, got: %2", m_iPlayerID, playerId), LogLevel.WARNING);
			return;
		}
		
		m_CachedFaction = SCR_FactionManager.SGetPlayerFaction(m_iPlayerID);
		m_TargetPlayer = playerEntity;
		ActivateSpawnPoint();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	void DisablePoint(int playerId)
	{
		if (playerId != m_iPlayerID)
		{
			Print(string.Format("Couldn't disable point, mismatching playerId(s), expected: %1, got: %2", m_iPlayerID, playerId), LogLevel.WARNING);
			return;
		}
		
		DeactivateSpawnPoint();
		
		m_CachedFaction = null;
		m_TargetPlayer = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActivateSpawnPoint()
	{
		SetFaction(m_CachedFaction);
		
		//--- Periodically refresh spawn's position
		//--- Clients cannot access another player's entity directly, because it may not be streamed for them
		ClearFlags(EntityFlags.STATIC, false);
		GetGame().GetCallqueue().CallLater(UpdateSpawnPos, m_fUpdateInterval * 1000, true);
		
		m_bIsActive = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsSpawnPointActive()
	{
		return m_bIsActive;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DeactivateSpawnPoint()
	{
		SetFaction(null);
		
		//--- Stop periodic refresh
		SetFlags(EntityFlags.STATIC, false);
		GetGame().GetCallqueue().Remove(UpdateSpawnPos);
		
		m_bIsActive = false;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSpawnPos()
	{
		if (!m_TargetPlayer)
			return;
		
		vector pos = m_TargetPlayer.GetOrigin();
		UpdateSpawnPosBroadcast(pos);
		Rpc(UpdateSpawnPosBroadcast, pos);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void UpdateSpawnPosBroadcast(vector pos)
	{
		SetOrigin(pos);
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetPositionAndRotation(out vector pos, out vector rot)
	{
		super.GetPositionAndRotation(pos, rot);
		
		if (m_TargetPlayer)
		{
			SCR_CompartmentAccessComponent compartmentAccessTarget = SCR_CompartmentAccessComponent.Cast(m_TargetPlayer.FindComponent(SCR_CompartmentAccessComponent));
			IEntity vehicle = compartmentAccessTarget.GetVehicle();
			if (vehicle)
				rot = vehicle.GetAngles();
			else
				rot = m_TargetPlayer.GetAngles();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected Vehicle GetTargetVehicle()
	{
		if (!m_TargetPlayer)
			return null;

		SCR_CompartmentAccessComponent compartmentAccessTarget = SCR_CompartmentAccessComponent.Cast(m_TargetPlayer.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessTarget)
			return null;
		
		return Vehicle.Cast(compartmentAccessTarget.GetVehicle());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		//~ Check if super fails or not if the spawnpoint can be reserved
		if (!super.CanReserveFor_S(playerId))
			return false; 
		
		Vehicle targetVehicle = GetTargetVehicle();
		if (targetVehicle)
		{
			// See if there are any slots left
			BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(targetVehicle.FindComponent(BaseCompartmentManagerComponent));
			array<BaseCompartmentSlot> compartments = {};
			int count = compartmentManager.GetCompartments(compartments);
			for (int i = 0; i < count; i++)
			{
				BaseCompartmentSlot slot = compartments[i];
				if (!slot.IsOccupied() && !slot.IsReserved())
					return true;
			}
			
			// No slots available
			result = SCR_ESpawnResult.NOT_ALLOWED_VEHICLE_FULL;
			return false;
		}
		
		// No vehicle and all other tests passed
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool PrepareSpawnedEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::PrepareSpawnedEntity_S(playerId: %2, data: %3, entity: %4)", Type().ToString(),
					requestComponent.GetPlayerId(), data, entity), LogLevel.NORMAL);
		#endif
		
		//~ Check if can prepare spawn from super
		if (!super.PrepareSpawnedEntity_S(requestComponent, data, entity))
			return false;
		
		// Spawned entity must have a compartment access component
		SCR_CompartmentAccessComponent compartmentAccessPlayer = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccessPlayer)
			return false;
		
		// Spawning on vehicle; resolve vehicle logic
		Vehicle targetVehicle = GetTargetVehicle();		
		if (targetVehicle)
			return PrepareSpawnedEntityForVehicle_S(requestComponent, data, entity, targetVehicle);
		
		// All checks succeed
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool PrepareSpawnedEntityForVehicle_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity, Vehicle vehicle)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::PrepareSpawnedEntityForVehicle_S(playerId: %2, data: %3, entity: %4, vehicle: %5)", Type().ToString(),
					requestComponent.GetPlayerId(),	data, entity, vehicle), LogLevel.NORMAL);
		#endif
		
		SCR_CompartmentAccessComponent accessComponent = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		
		array<BaseCompartmentSlot> compartments = {};
		int count = compartmentManager.GetCompartments(compartments);
		for (int i = 0; i < count; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			if (!slot.IsOccupied() && (!slot.IsReserved() || slot.IsReservedBy(entity)))
			{
				ChimeraWorld world = GetGame().GetWorld();
				return accessComponent.GetInVehicle(vehicle, slot, true, -1, ECloseDoorAfterActions.INVALID, world.IsGameTimePaused());
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanFinalizeSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!super.CanFinalizeSpawn_S(requestComponent, data, entity))
			return false;
		
		// Do not finalize if character is in the middle of entering a vehicle, leave that to be resolved on the authority first
		SCR_CompartmentAccessComponent accessComponent = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		if (accessComponent && accessComponent.IsGettingIn())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFinalizeSpawnDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_PlayerSpawnPoint()
	{
		GetGame().GetCallqueue().Remove(UpdateSpawnPos);
	}
}
