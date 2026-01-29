[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_PlayerSpawnPointManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

void SCR_DeployableSpawnPointBudgetChangedDelegate(SCR_ESpawnPointBudgetType newBudgetType);
typedef func SCR_DeployableSpawnPointBudgetChangedDelegate;
typedef ScriptInvokerBase<SCR_DeployableSpawnPointBudgetChangedDelegate> SCR_SpawnPointBudgetInvoker;

class SCR_PlayerSpawnPointManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_SpawnPointPrefab;
	
	[Attribute("1")]
	protected bool m_bEnablePlayerSpawnPoints;
	
	[Attribute("0", desc: "If true players are able to spawn at stationary command truck vehicles")]
	protected bool m_bRadioVehicleSpawningEnabled;
	
	[Attribute("1", desc: "It allows players to deploy their radiobackpack and create a deployable radio spawn point. It still allows existing deployed radio spawnpoints to be deconstructed by players if this bool is false")]
	protected bool m_bDeployableSpawnPointsEnabled;
	
	[Attribute(defvalue: SCR_ESpawnPointBudgetType.SUPPLIES.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESpawnPointBudgetType), desc: "Budget type for respawning on deployed radios")]
	protected SCR_ESpawnPointBudgetType m_eDeployableSpawnPointBudgetType;
	
	[Attribute("5", desc: "Amount of spawn tickets available for deployed radios")]
	protected int m_iDeployableSpawnPointTicketAmount;
	
	protected ref ScriptInvokerBool m_OnRadioVehicleSpawningChanged;
	protected ref ScriptInvokerBool m_OnSpawnPointDeployingEnabledChanged;
	protected ref SCR_SpawnPointBudgetInvoker m_OnSpawnPointBudgetTypeChanged;
	protected ref ScriptInvokerInt m_OnSpawnPointTicketAmountChanged;
	
	protected ref map<int, SCR_PlayerSpawnPoint> m_SpawnPoints = new map<int, SCR_PlayerSpawnPoint>();
	
	//------------------------------------------------------------------------------------------------
	//! Set status of automatic player spawn points.
	//! When enabled, a spawn point will be created for every connected player, even those who connect later.
	//! When disabled, all existing player spawn points will be deleted.
	//! \param[in] enable True to enable the system
	//! \param[in] notificationPlayerID if not -1 then a notification will be shown to all players that spawning on players is enabled/disabled
	void EnablePlayerSpawnPoints(bool enable, int notificationPlayerID = -1)
	{
		if (enable == m_bEnablePlayerSpawnPoints)
			return;
		
		m_bEnablePlayerSpawnPoints = enable;
		
		if (m_bEnablePlayerSpawnPoints)
		{
			//--- Create spawn points for all connected players
			array<int> playerIDs = {};
			int playerIDCount = GetGame().GetPlayerManager().GetPlayers(playerIDs);
			for (int i; i < playerIDCount; i++)
			{
				AddSpawnPoint(playerIDs[i]);
			}
		}
		else
		{
			//--- Delete all existing spawn points
			for (int i, count = m_SpawnPoints.Count(); i < count; i++)
			{
				delete m_SpawnPoints.Get(i);
			}
			m_SpawnPoints.Clear();
		}
		
		if (notificationPlayerID > 0)
		{
			if (enable)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_PLAYER, notificationPlayerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_PLAYER, notificationPlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the system is enabled, false otherwise
	bool IsPlayerSpawnPointsEnabled()
	{
		return m_bEnablePlayerSpawnPoints;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddSpawnPoint(int playerID)
	{
		if (!m_bEnablePlayerSpawnPoints || !m_SpawnPointPrefab)
			return;
		
		Resource resource = Resource.Load(m_SpawnPointPrefab);
		if (!resource || !resource.IsValid())
		{
			Debug.Error(string.Format("Invalid spawn point resource: %1 in %2::AddSpawnPoint", 
						m_SpawnPointPrefab, Type().ToString()));
			
			return;
		}
		
		SCR_PlayerSpawnPoint spawnPoint = SCR_PlayerSpawnPoint.Cast(GetGame().SpawnEntityPrefab(resource, GetOwner().GetWorld()));
		spawnPoint.SetPlayerID(playerID);
		
		if (!spawnPoint) // Don't register empty entries
		{
			Debug.Error(string.Format("PlayerSpawnPoint for player: %1 could not be created!", playerID));
			return;
		}
		
		m_SpawnPoints.Insert(playerID, spawnPoint);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveSpawnPoint(int playerID)
	{
		SCR_PlayerSpawnPoint spawnPoint;
		if (m_SpawnPoints.Find(playerID, spawnPoint))
		{
			RplComponent.DeleteRplEntity(spawnPoint, false);
			m_SpawnPoints.Remove(playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if Radio vehicle Respawning is enabled for specific respawn vehicles eg: Command Trucks (Server)
	//! \param[in] enable True to enable the system
	//! \param[in] notificationPlayerID if not -1 then a notification will be shown to all players that spawning on stationary vehicles is enabled/disabled
	void EnableRadioVehicleSpawning(bool enable, int notificationPlayerID = -1)
	{
		if (enable == m_bRadioVehicleSpawningEnabled)
			return;
		
		Rpc(RPC_EnableRadioVehicleSpawning, enable);
		RPC_EnableRadioVehicleSpawning(enable);
		
		if (notificationPlayerID > 0)
		{
			if (enable)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLE_RESPAWN_ON_RADIO_VEHICLE, notificationPlayerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLE_RESPAWN_ON_RADIO_VEHICLE, notificationPlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_EnableRadioVehicleSpawning(bool enable)
	{
		m_bRadioVehicleSpawningEnabled = enable;
		
		if (m_OnRadioVehicleSpawningChanged)
			m_OnRadioVehicleSpawningChanged.Invoke(m_bRadioVehicleSpawningEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsRadioVehicleSpawningEnabled()
	{
		return m_bRadioVehicleSpawningEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerBool GetOnRadioVehicleSpawningChanged()
	{
		if (!m_OnRadioVehicleSpawningChanged)
			m_OnRadioVehicleSpawningChanged = new ScriptInvokerBool();
		
		return m_OnRadioVehicleSpawningChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if players are allowed to deploy their radio backpack. (Server only)
	//! \param[in] enable True to enable the system
	//! \param[in] notificationPlayerID if not -1 then a notification will be shown to all players that deployable radio logic is enabled/disabled
	void EnableDeployableSpawnPoints(bool enable, int notificationPlayerID = -1)
	{
		if (enable == m_bDeployableSpawnPointsEnabled)
			return;
		
		Rpc(RPC_EnableDeployableSpawnPoints, enable);
		RPC_EnableDeployableSpawnPoints(enable);
		
		if (notificationPlayerID > 0)
		{
			if (enable)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_ENABLE_DEPLOYABLE_RADIO_SPAWNPOINT, notificationPlayerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_DISABLE_DEPLOYABLE_RADIO_SPAWNPOINT, notificationPlayerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_EnableDeployableSpawnPoints(bool enable)
	{
		m_bDeployableSpawnPointsEnabled = enable;
		
		if (m_OnSpawnPointDeployingEnabledChanged)
			m_OnSpawnPointDeployingEnabledChanged.Invoke(m_bDeployableSpawnPointsEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsDeployingSpawnPointsEnabled()
	{
		return m_bDeployableSpawnPointsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerBool GetOnSpawnPointDeployingEnabledChanged()
	{
		if (!m_OnSpawnPointDeployingEnabledChanged)
			m_OnSpawnPointDeployingEnabledChanged = new ScriptInvokerBool();
		
		return m_OnSpawnPointDeployingEnabledChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set budget type for respawning on deployed radios. (Server only)
	//! \param[in] budgetType SUPPLIES to enable supply usage; SPAWNTICKET for a limited amount of respawns; NONE to disable respawn budget
	//! \param[in] notificationPlayerID if not -1 then a notification will be shown to all players that deployable radio budget type has changed
	void SetDeployableSpawnPointBudgetType(SCR_ESpawnPointBudgetType budgetType, int notificationPlayerID = -1)
	{
		if (budgetType == m_eDeployableSpawnPointBudgetType)
			return;
		
		RPC_SetDeployableSpawnPointBudgetType(budgetType);
		Rpc(RPC_SetDeployableSpawnPointBudgetType, budgetType);
		
		if (m_OnSpawnPointBudgetTypeChanged)
			m_OnSpawnPointBudgetTypeChanged.Invoke(m_eDeployableSpawnPointBudgetType);
		
		if (notificationPlayerID <= 0)
			return;

		if (budgetType == SCR_ESpawnPointBudgetType.NONE)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_NONE, notificationPlayerID);
		else if (budgetType == SCR_ESpawnPointBudgetType.SUPPLIES)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_SUPPLIES, notificationPlayerID);
		else if (budgetType == SCR_ESpawnPointBudgetType.SPAWNTICKET)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_BUDGET_TICKETS, notificationPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetDeployableSpawnPointBudgetType(SCR_ESpawnPointBudgetType budgetType)
	{
		m_eDeployableSpawnPointBudgetType = budgetType;
		
		if (m_OnSpawnPointBudgetTypeChanged)
			m_OnSpawnPointBudgetTypeChanged.Invoke(m_eDeployableSpawnPointBudgetType);
	}
		
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ESpawnPointBudgetType GetDeployableSpawnPointBudgetType()
	{
		return m_eDeployableSpawnPointBudgetType;
	}
		
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_SpawnPointBudgetInvoker GetOnSpawnPointBudgetTypeChanged()
	{
		if (!m_OnSpawnPointBudgetTypeChanged)
			m_OnSpawnPointBudgetTypeChanged = new SCR_SpawnPointBudgetInvoker();
		
		return m_OnSpawnPointBudgetTypeChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set amount of respawn tickets for deployed radios. (Server only)
	//! \param[in] tickets Amount of respawns per deployed radio
	//! \param[in] notificationPlayerID if not -1 then a notification will be shown to all players that deployable radio respawn amount has changed
	void SetDeployableSpawnPointTicketAmount(int tickets, int notificationPlayerID = -1)
	{
		if (tickets == m_iDeployableSpawnPointTicketAmount)
			return;
		
		RPC_SetDeployableSpawnPointTicketAmount(tickets);
		Rpc(RPC_SetDeployableSpawnPointTicketAmount, tickets);
		
		if (notificationPlayerID > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_CHANGED_DEPLOYABLE_RADIO_SPAWNPOINT_TICKET_AMOUNT, notificationPlayerID, tickets);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_SetDeployableSpawnPointTicketAmount(int tickets)
	{
		m_iDeployableSpawnPointTicketAmount = tickets;
		
		if (m_OnSpawnPointTicketAmountChanged)
			m_OnSpawnPointTicketAmountChanged.Invoke(m_iDeployableSpawnPointTicketAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetDeployableSpawnPointTicketAmount()
	{
		return m_iDeployableSpawnPointTicketAmount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerInt GetOnSpawnPointTicketAmountChanged()
	{
		if (!m_OnSpawnPointTicketAmountChanged)
			m_OnSpawnPointTicketAmountChanged = new ScriptInvokerInt();
		
		return m_OnSpawnPointTicketAmountChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		AddSpawnPoint(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		RemoveSpawnPoint(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		int playerId = requestComponent.GetPlayerId();
		SCR_PlayerSpawnPoint playerPoint;
		if (m_SpawnPoints.Find(playerId, playerPoint))
			playerPoint.EnablePoint(playerId, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (!m_pGameMode.IsMaster())
			return;
		
		SCR_PlayerSpawnPoint playerPoint;
		if (m_SpawnPoints.Find(instigatorContextData.GetVictimPlayerID(), playerPoint))
			playerPoint.DisablePoint(instigatorContextData.GetVictimPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
    {	
        writer.WriteBool(m_bRadioVehicleSpawningEnabled); 
        writer.WriteBool(m_bDeployableSpawnPointsEnabled);
		
        return true;
    }
     
	//------------------------------------------------------------------------------------------------
    override bool RplLoad(ScriptBitReader reader)
    {		
		bool radioVehicleSpawningEnabled, deployableSpawnPointsEnabled;
		
        reader.ReadBool(radioVehicleSpawningEnabled);
        reader.ReadBool(deployableSpawnPointsEnabled);

		RPC_EnableRadioVehicleSpawning(radioVehicleSpawningEnabled);
		RPC_EnableDeployableSpawnPoints(deployableSpawnPointsEnabled);
		
        return true;
    }
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		foreach (int pointId, SCR_PlayerSpawnPoint spawnPoint : m_SpawnPoints)
		{
			RplComponent.DeleteRplEntity(spawnPoint, false);
		}
	}
}
