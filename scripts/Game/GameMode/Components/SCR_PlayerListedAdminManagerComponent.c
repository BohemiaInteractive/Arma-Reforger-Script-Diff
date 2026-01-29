void SCR_PlayerAdminManager_OnListedAdminsChanged(notnull array<int> adminPlayers);
typedef func SCR_PlayerAdminManager_OnActiveListedAdminsChanged;

[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_PlayerListedAdminManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_PlayerListedAdminManagerComponent : SCR_BaseGameModeComponent
{
	protected static SCR_PlayerListedAdminManagerComponent m_Instance;
	
	[RplProp(onRplName: "OnListedAdminsChanged")]
	protected ref array<int> m_aAdminPlayers = {};
	
	protected ref ScriptInvokerBase<SCR_PlayerAdminManager_OnActiveListedAdminsChanged> m_OnActiveListedAdminsChanged;
	
	//------------------------------------------------------------------------------------------------
	//! return Get Instance of SCR_PlayerListedAdminManagerComponent
	static SCR_PlayerListedAdminManagerComponent GetInstance()
	{
		return m_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int playerId)
	{
		//~ Master only
		if (!GetGameMode().IsMaster())
			return;
		
		//~ Player on the admin list?
		BackendApi backendApi = GetGame().GetBackendApi();
		if (!backendApi || !backendApi.IsListedServerAdmin(playerId) || m_aAdminPlayers.Contains(playerId))
			return;
		
		//~ Add player to the active listed admins
		m_aAdminPlayers.Insert(playerId);
		
		Replication.BumpMe();
		
		OnListedAdminsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		//~ Master only
		if (!GetGameMode().IsMaster())
			return;
		
		//~ Player on the admin list?
		BackendApi backendApi = GetGame().GetBackendApi();
		if (!backendApi || !backendApi.IsListedServerAdmin(playerId) || !m_aAdminPlayers.Contains(playerId))
			return;
		
		//~ Remove player from the active listed admins
		m_aAdminPlayers.RemoveItem(playerId);
		
		Replication.BumpMe();
		
		OnListedAdminsChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	//! return True if the player ID is in the active listed admins. Meaning that the player that joined can login as admin even if the player is not yet logged in
	bool IsPlayerOnAdminList(int playerId)
	{
		return m_aAdminPlayers.Contains(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! param[out] Get a list of all players that are  listed admins currently in the server. Both those are logged in and those who are not
	//! return Count of currently active listed admins
	int GetActiveListedAdmins(out notnull array<int> activeListedAdmins)
	{
		foreach (int admin : m_aAdminPlayers)
		{
			activeListedAdmins.Insert(admin);
		}
		
		return activeListedAdmins.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on player connect or disconnect and that player is a listed admin for the server
	protected void OnListedAdminsChanged()
	{
		if (m_OnActiveListedAdminsChanged)
			m_OnActiveListedAdminsChanged.Invoke(m_aAdminPlayers);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Script invoker when active listed admins changed aka: A player connected or left that is a listed admin of the server
	ScriptInvokerBase<SCR_PlayerAdminManager_OnActiveListedAdminsChanged> GetOnActiveListedAdminsChanged()
	{
		//~ Create script invoker if it doesn't already exists
		if (!m_OnActiveListedAdminsChanged)
			m_OnActiveListedAdminsChanged = new ScriptInvokerBase<SCR_PlayerAdminManager_OnActiveListedAdminsChanged>();
		
		return m_OnActiveListedAdminsChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PlayerListedAdminManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		//~ Instance already exists
		if (m_Instance)
		{
			Print("'SCR_PlayerListedAdminManagerComponent' Multiple instances of the 'SCR_PlayerListedAdminManagerComponent' exist in the world!", LogLevel.WARNING);
			return;
		}
		
		m_Instance = this;
	}
}