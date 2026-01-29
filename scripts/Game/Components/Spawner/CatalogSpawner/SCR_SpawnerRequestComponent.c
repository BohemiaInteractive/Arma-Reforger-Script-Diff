class SCR_SpawnerRequestComponentClass : ScriptGameComponentClass
{
}

//! Used for handling entity spawning requests for SCR_CatalogEntitySpawnerComponent and inherited classes, attached to SCR_PlayerController
class SCR_SpawnerRequestComponent : ScriptComponent
{
	[RplProp()]
	protected int m_iQueuedAIs;
	
	static const float NOTIFICATION_DURATION = 2;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] position
	void RequestPlayerTeleport(vector position)
	{
		Rpc(RPC_DoTeleportPlayer, position); 
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_DoTeleportPlayer(vector position)
	{
		SCR_Global.TeleportLocalPlayer(position, SCR_EPlayerTeleportedReason.BLOCKING_SPAWNER);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add (or substract) AI to quenue
	void AddQueuedAI(int value)
	{
		m_iQueuedAIs += value;
		if (m_iQueuedAIs <= 0)
			m_iQueuedAIs = 0;

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return queued requested AI units count that are currently on the way to rally point and not yet in player's group
	int GetQueuedAIs()
	{
		return m_iQueuedAIs;
	}

	//------------------------------------------------------------------------------------------------
	//! Defender spawner request from SCR_EnableDefendersAction
	//! \param[in] defenderSpawnerComp DefenderSpawner component to be handled
	//! \param[in] enable to enable or disable unit spawning
	//! \param[in] playerID id of player requesting the spawn
	void EnableSpawning(notnull SCR_DefenderSpawnerComponent defenderSpawnerComp, bool enable, int playerID)
	{
		IEntity spawnerOwnerEntity = defenderSpawnerComp.GetOwner();
		if (!spawnerOwnerEntity)
			return;

		RplComponent spawnerRplComp = RplComponent.Cast(spawnerOwnerEntity.FindComponent(RplComponent));
		if (!spawnerRplComp)
			return;

		Rpc(RPC_DoEnableSpawning, spawnerRplComp.Id(), enable, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Performs request on server
	//! \param[in] rplCompId RplComp id of entity with SCR_DefenderSpawnerComponent
	//! \param[in] index item index in User Faction
	//! \param[in] playerID id of player requesting the spawn
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_DoEnableSpawning(RplId defenderSpawnerID, bool enable, int playerID)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(defenderSpawnerID));
		if (!rplComp)
			return;

		SCR_DefenderSpawnerComponent entitySpawnerComp = SCR_DefenderSpawnerComponent.Cast(rplComp.GetEntity().FindComponent(SCR_DefenderSpawnerComponent));
		if (!entitySpawnerComp)
			return;

		entitySpawnerComp.EnableSpawning(enable, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Entity spawn request from SCR_SpawnEntityUserAction
	//! \param[in] index item index in User Action
	//! \param[in] spawnerComponent SpawnerComponent on which should be entity spawned
	//! \param[in] user the user requesting spawn
	//! \param[in] slot on which should be entity spawned
	void RequestCatalogEntitySpawn(int index, notnull SCR_CatalogEntitySpawnerComponent spawnerComponent, IEntity user, SCR_EntitySpawnerSlotComponent slot)
	{
		int userId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (userId == 0)
			return;

		IEntity spawnerOwnerEntity = spawnerComponent.GetOwner();
		if (!spawnerOwnerEntity)
			return;

		RplComponent spawnerRplComp = RplComponent.Cast(spawnerOwnerEntity.FindComponent(RplComponent));
		if (!spawnerRplComp)
			return;

		RplComponent slotRplComp = RplComponent.Cast(slot.GetOwner().FindComponent(RplComponent));
		if (!slotRplComp)
			return;

		Rpc(RPC_DoRequestCatalogSpawn, spawnerRplComp.Id(), index, userId, slotRplComp.Id());
	}

	//------------------------------------------------------------------------------------------------
	//! Performs request on server
	//! \param[in] rplCompId RplComp id of entity with spawner component
	//! \param[in] index item index in User Action
	//! \param[in] userId id of user requesting spawn
	//! \param[in] slotRplId slot RplID OPTIONAL
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_DoRequestCatalogSpawn(RplId rplCompId, int index, int userId, RplId slotRplId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (!rplComp)
			return;

		SCR_CatalogEntitySpawnerComponent entitySpawnerComp = SCR_CatalogEntitySpawnerComponent.Cast(rplComp.GetEntity().FindComponent(SCR_CatalogEntitySpawnerComponent));
		if (!entitySpawnerComp)
			return;

		SCR_EntityCatalogEntry entityEntry = entitySpawnerComp.GetEntryAtIndex(index);
		if (!entityEntry)
			return;

		RplComponent slotRplComp = RplComponent.Cast(Replication.FindItem(slotRplId));
		if (!slotRplComp)
			return;

		SCR_EntitySpawnerSlotComponent slotComp = SCR_EntitySpawnerSlotComponent.Cast(slotRplComp.GetEntity().FindComponent(SCR_EntitySpawnerSlotComponent));
		if (!slotComp)
			return;

		entitySpawnerComp.InitiateSpawn(entityEntry, userId, slotComp);
	}

	//------------------------------------------------------------------------------------------------
	//! Send notification to player
	//! \param[in] msgID
	//! \param[in] assetIndex
	//! \param[in] catalogType
	void SendPlayerFeedback(int msgID, int assetIndex, int catalogType = -1)
	{
		Rpc(RPC_DoPlayerFeedbackImpl, msgID, assetIndex, catalogType);
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification about request result to the requester
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RPC_DoPlayerFeedbackImpl(int msgID, int assetIndex, int catalogType)
	{
		string msg;
		string msg2;

		if (msgID == SCR_EEntityRequestStatus.NOT_ENOUGH_SPACE)
		{
			msg = "#AR-Campaign_DeliveryPointObstructed-UC";
		}

		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!player)
			return;

		SCR_Faction faction = SCR_Faction.Cast(player.GetLocalControlledEntityFaction());
		if (!faction)
			return;

		SCR_EntityCatalog factionCatalog = faction.GetFactionEntityCatalogOfType(catalogType);
		if (!factionCatalog)
			return;

		SCR_EntityCatalogEntry entry = factionCatalog.GetCatalogEntry(assetIndex);
		if (!entry)
			return;

		msg = GetMessageStringForCatalogType(catalogType);

		SCR_EntityCatalogSpawnerData spawnerData = SCR_EntityCatalogSpawnerData.Cast(entry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (spawnerData)
			msg2 = spawnerData.GetOverwriteName();
		
		if (msg2 == string.Empty)
			msg2 = entry.GetEntityName();

		SCR_PopUpNotification.GetInstance().PopupMsg(msg, NOTIFICATION_DURATION, msg2);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] catalogType catalog type from enum.
	//! \return string ID to be used in succesful request feedbacks.
	string GetMessageStringForCatalogType(EEntityCatalogType catalogType)
	{
		switch (catalogType)
		{
			case EEntityCatalogType.VEHICLE:
			{
				return "#AR-Campaign_VehicleReady-UC";
			}
			case EEntityCatalogType.CHARACTER:
			{
				return "#AR-Campaign_UnitReady-UC";
			}
			case EEntityCatalogType.GROUP:
			{
				return "#AR-Campaign_GroupReady-UC";
			}
		}

		//No specific catalogType defined
		return "#AR-Campaign_AssetReady-UC";
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
	}
}
