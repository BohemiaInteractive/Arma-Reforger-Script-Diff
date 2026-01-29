void ScriptInvokerOnPossessedProxyMethod(int playerID, bool isPossessing, RplId mainEntityID);
typedef func ScriptInvokerOnPossessedProxyMethod;
typedef ScriptInvokerBase<ScriptInvokerOnPossessedProxyMethod> ScriptInvokerOnPossessedProxy;

[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_PossessingManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PossessingManagerComponent : SCR_BaseGameModeComponent
{
	protected ref map<int, RplId> m_MainEntities = new map<int, RplId>();
	protected ref ScriptInvoker Event_OnPossessed;
	protected ref ScriptInvokerOnPossessedProxy Event_OnPossessedProxy;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public methods
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! \return Local instance of the possession manager
	static SCR_PossessingManagerComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return SCR_PossessingManagerComponent.Cast(gameMode.FindComponent(SCR_PossessingManagerComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Event called on server when player possession changes
	ScriptInvoker GetOnPossessed()
	{
		if (!Event_OnPossessed)
			Event_OnPossessed = new ScriptInvoker();

		return Event_OnPossessed;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Event called on proxy when player possession changes
	ScriptInvokerOnPossessedProxy GetOnPossessedProxy()
	{
		if (!Event_OnPossessedProxy)
			Event_OnPossessedProxy = new ScriptInvokerOnPossessedProxy();

		return Event_OnPossessedProxy;
	}

	//------------------------------------------------------------------------------------------------
	//! Get player's main entity.
	//! When not possessing, this will be the same as GetPlayerControlledEntity()
	//! When possessing, this will be player's main entity which was controlled before possessing started
	//! \param[in] iPlayerId
	//! \return Main player entity
	IEntity GetMainEntity(int iPlayerId)
	{
		RplId rplId;
		if (m_MainEntities.Find(iPlayerId, rplId))
		{
			RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(rplId));
			if (rplComponent)
				return rplComponent.GetEntity();
			else
				return null;
		}
		else
		{
			return GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get RplId of player's main entity.
	//! When not possessing, this will be RplId of GetPlayerControlledEntity()
	//! When possessing, this will be RplId of player's main entity which was controlled before possessing started
	//! \param[in] iPlayerId
	//! \return Main player entity's RplId. Returned even if the entity is not streamed in.
	RplId GetMainRplId(int iPlayerId)
	{
		RplId rplId;
		if (m_MainEntities.Find(iPlayerId, rplId))
			return rplId;
		else
			return GetRplId(GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId));
	}

	//------------------------------------------------------------------------------------------------
	//! Get the entity currently possessed by player.
	//! \param[in] iPlayerId
	//! \return Possessed entity. When not possessing, null is returned.
	IEntity GetPossessedEntity(int iPlayerId)
	{
		if (IsPossessing(iPlayerId))
			return GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId);
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get RplId of the entity currently possessed by player.
	//! \param[in] iPlayerId
	//! \return RplId of possessed entity. When not possessing, default RplId is returned.
	RplId GetPossessedRplId(int iPlayerId)
	{
		if (IsPossessing(iPlayerId))
			return GetRplId(GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId));
		else
			return RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID based on main entity, no matter if it's currently controlled or not.
	//! \param[in] entity Evaluated entity
	//! \return Player ID, or 0 if the entity does not belong to any player
	int GetIdFromMainEntity(IEntity entity)
	{
		array<int> players = {};
		for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
		{
			if (GetMainEntity(players[i]) == entity)
				return players[i];
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID based on main entity's RplId, no matter if it's currently controlled or not.
	//! \param[in] rplId Evaluated entity's RplId
	//! \return Player ID, or 0 if the entity does not belong to any player
	int GetIdFromMainRplId(RplId rplId)
	{
		array<int> players = {};
		for (int i = 0, count = GetGame().GetPlayerManager().GetPlayers(players); i < count; i++)
		{
			if (GetMainRplId(players[i]) == rplId)
				return players[i];
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID from directly controlled or main entity.
	//! \param[in] entity Evaluated entity
	//! \return Player ID, or 0 if the entity does not belong to any player
	int GetIdFromControlledEntity(IEntity entity)
	{
		int iPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		if (iPlayerId > 0)
			return iPlayerId;
		else
			return GetIdFromMainEntity(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID from RplId of directly controlled or main entity.
	//! \param[in] rplId Evaluated entity's RplId
	//! \return Player ID, or 0 if the entity does not belong to any player
	int GetIdFromControlledRplId(RplId rplId)
	{
		int iPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(rplId);
		if (iPlayerId > 0)
			return iPlayerId;
		else
			return GetIdFromMainRplId(rplId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] iPlayerId Player ID
	//! \return if given player is currently possessing an entity.
	bool IsPossessing(int iPlayerId)
	{
		return m_MainEntities.Contains(iPlayerId);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] controlledEntity
	//! \param[in] mainEntity
	//! \param[in] isPossessing
	void SetMainEntity(int playerID, IEntity controlledEntity, IEntity mainEntity, bool isPossessing)
	{
		RplId mainEntityID = RplId.Invalid();
		if (mainEntity)
		{
			RplComponent rpl = RplComponent.Cast(mainEntity.FindComponent(RplComponent));
			if (rpl)
				mainEntityID = rpl.Id();
		}

		SetMainEntityBroadcast(playerID, isPossessing, mainEntityID);
		Rpc(SetMainEntityBroadcast, playerID, isPossessing, mainEntityID);
		
		if (Event_OnPossessed)
			Event_OnPossessed.Invoke(playerID, controlledEntity, mainEntity, isPossessing);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Protected methods
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetMainEntityBroadcast(int playerID, bool isPossessing, RplId mainEntityID)
	{	
		if (isPossessing)
			m_MainEntities.Set(playerID, mainEntityID);
		else
			m_MainEntities.Remove(playerID);
		
		if (Event_OnPossessedProxy)
			Event_OnPossessedProxy.Invoke(playerID, isPossessing, mainEntityID);
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Static methods
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Get player's main entity.
	//! When not possessing, this will be the same as GetPlayerControlledEntity()
	//! When possessing, this will be player's main entity which was controlled before possessing started
	//! \param[in] iPlayerId
	//! \return Main player entity
	static IEntity GetPlayerMainEntity(int iPlayerId)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetMainEntity(iPlayerId);
		else
			return GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Get RplId of player's main entity.
	//! When not possessing, this will be RplId of GetPlayerControlledEntity()
	//! When possessing, this will be RplId of player's main entity which was controlled before possessing started
	//! \param[in] iPlayerId
	//! \return Main player entity's RplId. Returned even if the entity is not streamed in.
	static RplId GetPlayerMainRplId(int iPlayerId)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetMainRplId(iPlayerId);
		else
			return possessingManager.GetRplId(GetGame().GetPlayerManager().GetPlayerControlledEntity(iPlayerId));
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID based on main entity, no matter if it is currently controlled or not.
	//! \param[in] entity Evaluated entity
	//! \return Player ID, or 0 if the entity does not belong to any player
	static int GetPlayerIdFromMainEntity(IEntity entity)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetIdFromMainEntity(entity);
		else
			return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID based on main entity, no matter if it's currently controlled or not.
	//! \param[in] rplId Evaluated entity's RplId
	//! \return Player ID, or 0 if the entity does not belong to any player
	static int GetPlayerIdFromMainRplId(RplId rplId)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetIdFromMainRplId(rplId);
		else
			return GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(rplId);
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID from directly controlled or main entity.
	//! \param[in] entity Evaluated entity
	//! \return Player ID, or 0 if the entity does not belong to any player
	static int GetPlayerIdFromControlledEntity(IEntity entity)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetIdFromControlledEntity(entity);
		else
			return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Get player ID from RplId of directly controlled or main entity.
	//! \param[in] rplId Evaluated entity's RplId
	//! \return Player ID, or 0 if the entity does not belong to any player
	static int GetPlayerIdFromControlledEntity(RplId rplId)
	{
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			return possessingManager.GetIdFromControlledRplId(rplId);
		else
			return GetGame().GetPlayerManager().GetPlayerIdFromEntityRplId(rplId);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Overrides
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected override void OnControllableDeleted(IEntity entity)
	{
//		super.OnControllableDeleted(entity);
//		//--- Switch to main entity when the possessed one is deleted (ToDo: Better deletion detection than ChimeraCharacter event?)
//		int playerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
//		if (playerID > 0)
//		{
//			if (IsPossessing(playerID))
//			{
//				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
//				if (playerController)
//					playerController.SetPossessedEntity(null);
//			}
//		}
	}

	//------------------------------------------------------------------------------------------------
	protected RplId GetRplId(IEntity entity)
	{
		if (entity)
		{
			RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));
			if (rpl)
				return rpl.Id();
		}

		return RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		int pid = instigatorContextData.GetVictimPlayerID();
		if (pid > 0)
		{
			SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(pid));
			if (rc)
				rc.NotifyReadyForSpawn_S();
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool HandlePlayerKilled(int playerId, IEntity playerEntity, IEntity killerEntity, notnull Instigator instigator)
	{
		if (playerId > 0)
		{
			//--- Controlled entity
			if (playerEntity != GetMainEntity(playerId))
			{
				//--- Switch to main entity when the possessed one dies
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				if (playerController)
				{
					//--- Open editor, assume it will stop possessing (ToDo: No direct editor reference here)
					SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
					if (core)
					{
						SCR_EditorManagerEntity editorManager = core.GetEditorManager(playerId);
						if (editorManager)
						{
							editorManager.Open();
							// Don't handle kill automatically
							return false;
						}
					}
					
					//--- No editor, stop possessing right now					
					playerController.SetPossessedEntity(null);
					// Don't handle this kill automatically
					return false;
				}
			}
		}
		
		// Main entity, handle kill as usual
		return super.HandlePlayerKilled(playerId, playerEntity, killerEntity, instigator);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		m_MainEntities.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		int mainEntityCount = m_MainEntities.Count();
		writer.WriteInt(mainEntityCount);
		
		for (int i = 0; i < mainEntityCount; i++)
		{
			writer.WriteInt(m_MainEntities.GetKey(i));
			writer.WriteRplId(m_MainEntities.GetElement(i));
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		int mainEntityCount;
		reader.ReadInt(mainEntityCount);
		
		int playerID;
		RplId rplID;
		for (int i = 0; i < mainEntityCount; i++)
		{
			reader.ReadInt(playerID);
			reader.ReadRplId(rplID);
			
			m_MainEntities.Insert(playerID, rplID);
		}
		
		return true;
	}
}
