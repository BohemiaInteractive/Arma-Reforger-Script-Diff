[ComponentEditorProps(category: "GameScripted/ZoneRestriction", description: "")]
class SCR_PlayersRestrictionZoneManagerComponentClass : ScriptComponentClass
{
}

class SCR_PlayersRestrictionZoneManagerComponent : ScriptComponent
{
	//References
	protected PlayerManager m_PlayerManager;
	
	//Registered restriction zones
	protected ref set<SCR_EditorRestrictionZoneEntity> m_aRestrictionZones = new set<SCR_EditorRestrictionZoneEntity>();
	
	//Players data for being in zones
	protected ref map<int, ref SCR_PlayerRestrictionZoneData> m_PlayerRestrictionZoneData = new map<int, ref SCR_PlayerRestrictionZoneData>();
	
	//Update
	protected int m_iPlayerCheckIndex = 0;
	protected const int ZONE_CHECK_PLAYER_BATCH_AMOUNT = 10; //!< The amount of players the manager checks each update. The leftovers will be done next update etc
	
	protected const int ZONE_CHECK_FREQUENCY = 50;

	//============================== ZONE ARRAYS ==============================\\

	//------------------------------------------------------------------------------------------------
	//! Add restriction zone so it can be checked if a player is in/leaves a zone. Server only
	//! \param[in] zone the zone to add
	void AddRestrictionZone(SCR_EditorRestrictionZoneEntity zone)
	{
		if (!zone || !Replication.IsServer() || m_aRestrictionZones.Contains(zone))
			return;
		
		m_aRestrictionZones.Insert(zone);
		
		if (m_aRestrictionZones.Count() == 1)
			GetGame().GetCallqueue().CallLater(ZoneCheckUpdate, ZONE_CHECK_FREQUENCY, true, false, null, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove restriction zone so it no longer checks if a player is in/leaves a zone. Server only
	//! \param[in] zone the zone to remove
	void RemoveRestrictionZone(SCR_EditorRestrictionZoneEntity zone)
	{
		if (!zone || !Replication.IsServer())
			return;
		
		int index = m_aRestrictionZones.Find(zone);
		if (index < 0)
			return;
		
		ZoneMovedOrDeleted(zone);
		
		m_aRestrictionZones.Remove(index);
		
		if (m_aRestrictionZones.IsEmpty())
			GetGame().GetCallqueue().Remove(ZoneCheckUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ZoneCheckUpdate(bool updateAll, SCR_EditorRestrictionZoneEntity removedZone = null, array<int> playerIDZoneMoved = null)
	{	
		array<int> players = {};
		int playerCount = m_PlayerManager.GetPlayers(players);
		
		//If no players do not check
		if (players.IsEmpty())
			return;
	
		IEntity playerEntity;
		SCR_PlayerRestrictionZoneData zoneData;
		
		vector playerEntityPosition;
		vector restrictionZonePosition;
		float distanceSqrXZ;
		
		bool inZone;
		bool inWarningZone;	
		bool inRemovedZone;
		
		int startCheckingIndex;
		int endCheckingIndex;
		
		bool wasTeleported;
		bool ignoreWasTeleported;
		
		//Check if index is greater then player count. If true: Reset
		if (!updateAll)
		{
			if (m_iPlayerCheckIndex >= playerCount)
				m_iPlayerCheckIndex = 0;
		
			//Set the next batch of players to check
			endCheckingIndex = m_iPlayerCheckIndex + ZONE_CHECK_PLAYER_BATCH_AMOUNT;
			
			if (endCheckingIndex > playerCount)
				endCheckingIndex = playerCount;
			
			startCheckingIndex = m_iPlayerCheckIndex;
		}
		else 
		{
			endCheckingIndex = playerCount;
		}
			
		ERestrictionZoneWarningType warningType = 0;
		
		vector zoneCenter;
		
		float warningRadiusSq;
		float zoneRadiusSq;
		float prevDistanceSqrXZ;
		
		//Checks a batch of players
		for (int i = startCheckingIndex; i < endCheckingIndex; i++)
		{			
			inZone = false;
			zoneCenter = vector.Zero;
			inWarningZone = false;
			prevDistanceSqrXZ = -1;
			
			playerEntity = m_PlayerManager.GetPlayerControlledEntity(players[i]);
				
			//Did not find data so add it
			if (!m_PlayerRestrictionZoneData.Find(players[i], zoneData))
			{
				zoneData = new SCR_PlayerRestrictionZoneData(m_PlayerManager.GetPlayerControlledEntity(players[i]));
				m_PlayerRestrictionZoneData.Insert(players[i], zoneData);
			}
			
			//Not the same entity so clear the entity data
			if (zoneData.m_PlayerEntity != playerEntity && playerEntity != null)
				SetPlayerZoneData(players[i], playerEntity, false, false, -1);

			//No entity or dead entity clear the data
			if (playerEntity == null)
			{
				//Clear data
				if (zoneData.m_PlayerEntity != null)
					SetPlayerZoneData(players[i], null, false, false, -1);
				
				continue;
			}
			
			//Get player entity position
			playerEntityPosition = playerEntity.GetOrigin();
			
			//Go over each zone and check if player is in the zone and if it is in the warning zone
			foreach (SCR_EditorRestrictionZoneEntity zone: m_aRestrictionZones)
			{
				if (!zone)
					continue;
				
				restrictionZonePosition = zone.GetOrigin();
				prevDistanceSqrXZ = distanceSqrXZ;
				distanceSqrXZ = vector.DistanceSqXZ(playerEntityPosition, restrictionZonePosition);
				
				if (distanceSqrXZ <= zone.GetRestrictionZoneRadiusSq())
				{
					//If zone was deleted or moved (and player is still in it)
					if (zone == removedZone)
					{
						inRemovedZone = true;
						continue;
					}
					
					//~ Player is in zone
					inZone = true;
									
					//~ Player is in warning zone so show warning UI (Unless the player is safe in any of the other zones)
					if (distanceSqrXZ > zone.GetWarningZoneRadiusSq() && zone.GetWarningZoneRadius() != zone.GetRestrictionZoneRadius())
					{
						inWarningZone = true;
						
						//~ If no zone set or the center of the zone is closer then the prev set zone then set the zone the player will be guided towards
						if (zoneCenter == vector.Zero || distanceSqrXZ < prevDistanceSqrXZ)
						{
							warningType = zone.GetWarningType();	
							zoneCenter = zone.GetOrigin();
							warningRadiusSq = zone.GetWarningZoneRadiusSq();
							zoneRadiusSq = zone.GetRestrictionZoneRadiusSq();
						}
					}	
					//In a zone but not in warning part so never show warning as player is safe
					else if (distanceSqrXZ <= zone.GetWarningZoneRadiusSq())
					{
						inWarningZone = false;
						break;
					}
				}
				//If zone was moved and player in it before but no longer is. 
				else if (zone == removedZone && playerIDZoneMoved && playerIDZoneMoved.Contains(players[i]))
				{
					inRemovedZone = true;
				}
				//If player out of zone, check if was teleported
				else if (!ignoreWasTeleported)
				{
					//Player was teleported
					if (distanceSqrXZ >= zone.GetTeleportedZoneRadiusSq())
					{
						wasTeleported = true;
					}
					//Player walked out so never  check if was teleported
					else 
					{
						wasTeleported = false;
						ignoreWasTeleported = true;
					}
				}
			}
			
			//Player inzone changed
			if (inZone != m_PlayerRestrictionZoneData[players[i]].m_bInZone)
			{
				//If zone was removed by GM and it was the only zone the player was in ignore it
				if (inRemovedZone && !inZone)
				{
					SetPlayerZoneData(players[i], null, false, false, -1);
					continue;
				}
				//Player moved out of zone thus should will be killed
				else if (!inZone)
				{
					//Player was teleported so do not kill
					if (wasTeleported)
					{
						SetPlayerZoneData(players[i], null, false, false, -1);
						continue;
					}
					
					KillPlayerOutOfZone(players[i], playerEntity);
					SetPlayerZoneData(players[i], null, false, false, -1);
					continue;
				}
			}
			
			//Update if not killed
			SetPlayerZoneData(players[i], playerEntity, inZone, inWarningZone, warningType, zoneCenter, warningRadiusSq, zoneRadiusSq);
			continue;		
		}
		
		//Set next batch
		if (!updateAll)
			m_iPlayerCheckIndex = endCheckingIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPlayerZoneData(int playerID, IEntity playerEntity, bool inZone, bool inWarningZone, ERestrictionZoneWarningType warningType, vector zoneCenter = vector.Zero, float warningRadiusSq = -1, float zoneRadiusSq = -1)
	{		
		SCR_PlayerRestrictionZoneData zoneData; 
		
		if (!m_PlayerRestrictionZoneData.Find(playerID, zoneData))
			return;
	
		//~ Left center zone
		if (!inWarningZone && zoneData.m_bInWarningZone)
			ShowWarningUI(playerID, false, -1, false, vector.Zero, -1, -1);
		//~ entered warning zone or zone center point changed
		else if (inWarningZone && (!zoneData.m_bInWarningZone || zoneCenter != zoneData.m_vZoneCenter))
			ShowWarningUI(playerID, true, warningType, zoneCenter != zoneData.m_vZoneCenter, zoneCenter, warningRadiusSq, zoneRadiusSq);
		
		zoneData.m_bInWarningZone = inWarningZone; 
		zoneData.m_PlayerEntity = playerEntity; 
		zoneData.m_bInZone = inZone;
		zoneData.m_vZoneCenter = zoneCenter;
		
		m_PlayerRestrictionZoneData[playerID] = zoneData;
	}
	
	//------------------------------------------------------------------------------------------------
	//Show warning HUD
	protected void ShowWarningUI(int playerID, bool show, ERestrictionZoneWarningType warningIndex, bool centerChanged, vector zoneCenter, float warningRadiusSq, float zoneRadiusSq)
	{
		PlayerController playerController = m_PlayerManager.GetPlayerController(playerID);
		
		if (!playerController)
			return;
		
		SCR_PlayerRestrictionZoneWarningComponent warningComponent = SCR_PlayerRestrictionZoneWarningComponent.Cast(playerController.FindComponent(SCR_PlayerRestrictionZoneWarningComponent));
		if (!warningComponent)
			return;
		
		warningComponent.ShowWarningServer(show, warningIndex, centerChanged, zoneCenter, warningRadiusSq, zoneRadiusSq);
	}
	
	//------------------------------------------------------------------------------------------------
	//Kill the player that walked outside of the zone
	protected void KillPlayerOutOfZone(int playerID, IEntity playerEntity)
	{
		if (!playerEntity)
			return;
		
		DamageManagerComponent damageManager = DamageManagerComponent.Cast(playerEntity.FindComponent(DamageManagerComponent));
		
		if (!damageManager || damageManager.GetState() == EDamageState.DESTROYED)
			return;
		
		damageManager.SetHealthScaled(0);
		SetPlayerZoneData(playerID, null, false, false, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	//A zone was deleted or moved
	protected void ZoneMovedOrDeleted(SCR_EditorRestrictionZoneEntity zone, array<int> playerIDZoneMoved = null)
	{
		if (!zone)
			return;
		
		array<int> players = {};
		m_PlayerManager.GetPlayers(players);
		
		//If no players do not check
		if (players.IsEmpty())
			return;
	
		//Only one zone so just clear data
		if (m_aRestrictionZones.Count() == 1)
		{
			foreach (int playerID: players)
			{
				SetPlayerZoneData(playerID, null, false, false, -1);
			}
		}
		else 
		{
			ZoneCheckUpdate(true, zone, playerIDZoneMoved);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Check if a zone or player was moved
	protected void OnEntityTransformChanged(SCR_EditableEntityComponent editableEntity, vector prevTransfom[4])
	{
		if (m_aRestrictionZones.IsEmpty() || !editableEntity)
			return;
		
		if (editableEntity.GetOwner().Type() == SCR_EditorRestrictionZoneEntity)
		{
			SCR_EditorRestrictionZoneEntity zone = SCR_EditorRestrictionZoneEntity.Cast(editableEntity.GetOwner());
			if (!zone)
				return;
			
			vector playerEntityPosition;
			vector restrictionZonePosition;
			float distanceSqrXZ;
			
			//Get prev zone position
			restrictionZonePosition = prevTransfom[3];
			
			array<int> players = {};
			array<int> playersInZone = {};
			m_PlayerManager.GetPlayers(players);
			IEntity playerEntity;
			
			foreach (int player: players)
			{
				playerEntity = m_PlayerManager.GetPlayerControlledEntity(player);
				if (!playerEntity)
					continue;
				
				playerEntityPosition = playerEntity.GetOrigin();
				distanceSqrXZ = vector.DistanceSqXZ(playerEntityPosition, restrictionZonePosition);
				
				//Player is in moved zone
				if (distanceSqrXZ <= zone.GetRestrictionZoneRadiusSq())
					playersInZone.Insert(player);
			}
			
			ZoneMovedOrDeleted(zone, playersInZone);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Player was disconnected
	protected void OnPlayerDisconnect(int playerID)
	{
		if (m_PlayerRestrictionZoneData.Contains(playerID))
			m_PlayerRestrictionZoneData.Remove(playerID);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (m_aRestrictionZones.IsEmpty())
			return;
		
		if (m_PlayerRestrictionZoneData.Contains(instigatorContextData.GetVictimPlayerID()))
			SetPlayerZoneData(instigatorContextData.GetVictimPlayerID(), null, false, false, -1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		GetGame().GetCallqueue().CallLater(DelayedInit, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//Delayed init so SCR_PlayersManagerEditorComponent can be found
	protected void DelayedInit()
	{
		m_PlayerManager = GetGame().GetPlayerManager();
		
		if (!m_PlayerManager)
			return;
		
		SCR_EditableEntityCore editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!editableEntityCore)
			return;
		
		editableEntityCore.Event_OnEntityTransformChangedServer.Insert(OnEntityTransformChanged);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnect);
			gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (!Replication.IsServer())
			return;
		
		if (!m_aRestrictionZones.IsEmpty())
			GetGame().GetCallqueue().Remove(ZoneCheckUpdate);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
			gameMode.GetOnPlayerKilled().Remove(OnPlayerKilled);
		}
		
		SCR_EditableEntityCore editableEntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (editableEntityCore)
			editableEntityCore.Event_OnEntityTransformChangedServer.Remove(OnEntityTransformChanged);
	}
}

class SCR_PlayerRestrictionZoneData
{	
	IEntity m_PlayerEntity;
	bool m_bInZone;
	bool m_bInWarningZone;
	vector m_vZoneCenter;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] playerEntity
	void SCR_PlayerRestrictionZoneData(IEntity playerEntity)
	{
		m_PlayerEntity = playerEntity;
	}
}
