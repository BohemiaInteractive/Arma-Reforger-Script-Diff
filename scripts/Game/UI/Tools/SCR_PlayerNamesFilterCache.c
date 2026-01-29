//------------------------------------------------------------------------------------------
class SCR_PlayerNamesFilterCache
{
	
	private ref map<string, string> m_mUsernames;
	private static ref SCR_PlayerNamesFilterCache s_Instance;
	
	//C O N S T R U C T O R
	//-------------------------------------------------------------------------------------
	private void SCR_PlayerNamesFilterCache()
	{
		s_Instance = this;
		m_mUsernames = new map<string, string>();
		
		//attach callbacks if in a game, otherwise skip callbacks and current players initialization
		SCR_BaseGameMode curMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!curMode)
			return;
		
		curMode.GetOnPlayerConnected().Insert(ProcessOnPlayerConnected);
		curMode.GetOnPlayerDisconnected().Insert(ProcessOnPlayerDisconnected);
		
		PlayerManager pm = GetGame().GetPlayerManager();
		
		if (!pm)
			return;
		
		//initialize stuff and get all current players
		array <int> presentPlayers = {};
		GetGame().GetPlayerManager().GetAllPlayers(presentPlayers);
		
		//filter all current players and store them
		foreach (int player : presentPlayers)
		{
			string cleanName = GetGame().GetPlayerManager().GetPlayerName(player);
			m_mUsernames[cleanName] = FilterName(cleanName);
		}
	}
	
	//P U B L I C    M E T H O D S 
	//-------------------------------------------------------------------------------------
	static SCR_PlayerNamesFilterCache GetInstance()
	{
		if(!s_Instance)
			s_Instance = new SCR_PlayerNamesFilterCache();
		
		return s_Instance;
	}
	
	//-------------------------------------------------------------------------------------
	void Destroy()
	{
		s_Instance = null;
	}
	
	//-------------------------------------------------------------------------------------
	string GetPlayerDisplayName(int playerId)
	{
		string username = GetNameFromManagers(playerId);
		if (!username.IsEmpty())
		{
			SearchAndUpdateNames(username);
			username = m_mUsernames[username];
		}
		return username;
	}
	
	//P R I V A T E     M E T H O D S
	//-------------------------------------------------------------------------------------
	private string FilterName(string name)
	{
		name.Replace("<", "");
		name.Replace(">", "");
		name.Replace("#", "");

		return name;
	}
	
	//-------------------------------------------------------------------------------------
	private void SearchAndUpdateNames(string username)
	{
		if (!m_mUsernames.Contains(username))
			m_mUsernames[username] = FilterName(username);
	}
	
	//-------------------------------------------------------------------------------------
	private string GetNameFromManagers(int playerId)
	{
		PlayerManager pm = GetGame().GetPlayerManager();
		string username;
		
		if (pm)
		{
			username = GetGame().GetPlayerManager().GetPlayerName(playerId);
			if(username.IsEmpty())
			{
				SCR_NotificationsComponent notificationsManager = SCR_NotificationsComponent.GetInstance();
				if (notificationsManager)
					username = notificationsManager.GetPlayerNameFromHistory(playerId);
			}
		}
		
		return username;
	}
	
	//C A L L B A C K S
	//-------------------------------------------------------------------------------------
	private void ProcessOnPlayerConnected(int playerId)
	{
		string username = GetNameFromManagers(playerId);
		if (!username.IsEmpty())
			SearchAndUpdateNames(username);
	}
	
	//-------------------------------------------------------------------------------------
	private void ProcessOnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		m_mUsernames.Remove(GetGame().GetPlayerManager().GetPlayerName(playerId));
	}
}