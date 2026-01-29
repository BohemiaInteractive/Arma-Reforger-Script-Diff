[EntityEditorProps(category: "GameScripted/FiringRange", description: "Handles score and MP synchronization. Only one per world.", color: "0 0 255 255")]
class SCR_FiringRangeManagerClass: ScriptedGameTriggerEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_FiringRangeManager : ScriptedGameTriggerEntity
{
	//! Network entity
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Entity to spawn as communication entity.", "et")]
	private ResourceName m_ComEnt;
	
	protected Widget m_wRoot;
	protected Widget m_wTable
	private SCR_FiringRangeScoringComponent m_ScoringSystem;
	private ref array<int> m_aPlayerScores = new array<int>;
	private ref array<int> m_aPlayerScoresMax = new array<int>;
	private ref array<string> m_aPlayerNames = new array<string>;
	private ref array<SCR_PlayerScoreInfoFiringRange> m_aAllPlayersInfo = new array<SCR_PlayerScoreInfoFiringRange>;
	private ref array<Widget> m_aPlayerWidgets = new array<Widget>;
	[RplProp()]	
	private ref array<int> m_aPlayersInArea = new array<int>;
	
	// array contains all firing line controllers
	private ref array<IEntity> m_aFiringLineControllers = new array<IEntity>;

	private int m_iLocalPlayerID = -1;
	private int m_iLastSelectedTargetId = -1;
	private int m_iSelectedIndex = -1;
	protected static SCR_FiringRangeManager s_FiringRangeManagerMain;
	
	protected SCR_FiringRangeManager m_FiringRangeManagerInstance;
	
	// RPL component
	private RplComponent m_RplComponent;
	
	private int m_iPlayersInGameCount = -1;
	private int m_iPlayersInAreaCount = -1;  	
	
	//------------------------------------------------------------------------------------------------
	//-------------------------------------- SCORE PART START ----------------------------------------
	//------------------------------------------------------------------------------------------------
	void ClearPlayerScore(int playerID)
	{		
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("ClearPlayerScore()"))
			return;
		#endif
		
		if (!m_ScoringSystem)
			return;

		m_ScoringSystem.ClearScore(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void CountPlayerScore(int scoringPlayer,int scorePoints)
	{
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("CountPlayerScore()"))
			return;
		#endif
		
		if (!m_ScoringSystem)
			return;
		
		m_ScoringSystem.AddScore(scoringPlayer,scorePoints);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerScoreMax(int playerID, float maxScore)
	
	{	
		#ifdef ENABLE_DIAG
		if (!CheckMasterOnlyMethod("SetPlayerScoreMax()"))
			return;
		#endif
		
		if (!m_ScoringSystem)
			return;
		m_ScoringSystem.SetScoreMax(playerID,maxScore);
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateScoreboardData()
	{
		if (!m_ScoringSystem)
			return;
		
		if (m_aPlayerWidgets.IsEmpty())
			return;
		
		if (m_aPlayerWidgets.Count() != m_aAllPlayersInfo.Count())
			return;
		
		foreach (int i, SCR_PlayerScoreInfoFiringRange info : m_aAllPlayersInfo)
		{
			TextWidget nameWidget = TextWidget.Cast(m_aPlayerWidgets[i].FindAnyWidget("Name"));
			if (nameWidget)
				nameWidget.SetText(info.GetName());

			TextWidget scoreWidget = TextWidget.Cast(m_aPlayerWidgets[i].FindAnyWidget("Score"));
			if (scoreWidget)
				scoreWidget.SetText(info.GetScore(m_ScoringSystem).ToString());


			TextWidget maxScoreWidget = TextWidget.Cast(m_aPlayerWidgets[i].FindAnyWidget("MaxScore"));
			if (maxScoreWidget)
				maxScoreWidget.SetText(info.GetScoreMax().ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	int GetHighScoreInfo(array<SCR_PlayerScoreInfoFiringRange> infos)
	{
		int highScore = int.MIN;
		int highIndex = -1;
		
		foreach (int i, SCR_PlayerScoreInfoFiringRange info : infos)
		{
			if (info)
			{
				int score = info.GetScore();
				if (score > highScore)
				{
					highScore = score;
					highIndex = i;
				}
			}
		}
		
		return highIndex;
	}
		
	//------------------------------------------------------------------------------------------------
	bool ScoreChanged()
	{
		int count = m_aAllPlayersInfo.Count();
		if (count != m_aPlayerScores.Count())
		{
			// Update faction array
			m_aPlayerScores.Clear();
			foreach (SCR_PlayerScoreInfoFiringRange info : m_aAllPlayersInfo)
			{
				int score = -1000;
				if (info)
				{
					score = info.GetScore();
				}
				
				m_aPlayerScores.Insert(score);
			}
			
			return true;
		}
		
		bool changed = false;
		for (int i = 0; i < count; i++)
		{
			SCR_PlayerScoreInfoFiringRange info = m_aAllPlayersInfo.Get(i);
			if (!info)
				continue;
			
			int score = info.GetScore();
			if (score != m_aPlayerScores.Get(i))
			{
				m_aPlayerScores.Set(i, score);
				changed = true;
			}
		}
		
		return changed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool ScoreMaxChanged()
	{
		int count = m_aAllPlayersInfo.Count();
		if (count != m_aPlayerScoresMax.Count())
		{
			// Update faction array
			m_aPlayerScoresMax.Clear();
			foreach (SCR_PlayerScoreInfoFiringRange info : m_aAllPlayersInfo)
			{
				int scoreMax = -1000;
				if (info)
				{
					scoreMax = info.GetScoreMax();
				}
				
				m_aPlayerScoresMax.Insert(scoreMax);
			}
			
			return true;
		}
		
		bool changed = false;
		for (int i = 0; i < count; i++)
		{
			SCR_PlayerScoreInfoFiringRange info = m_aAllPlayersInfo.Get(i);
			if (!info)
				continue;
			
			int scoreMax = info.GetScoreMax();
			if (scoreMax != m_aPlayerScoresMax.Get(i))
			{
				m_aPlayerScoresMax.Set(i, scoreMax);
				changed = true;
			}
		}
		
		return changed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool NameChanged()
	{
		int count = m_aAllPlayersInfo.Count();
		if (count != m_aPlayerNames.Count())
		{
			// Update faction array
			m_aPlayerNames.Clear();
			foreach (SCR_PlayerScoreInfoFiringRange info : m_aAllPlayersInfo)
			{
				string name = "";
				if (info)
					name = info.GetName();
				
				m_aPlayerNames.Insert(name);
			}
			
			return true;
		}
		
		bool changed = false;
		for (int i = 0; i < count; i++)
		{
			SCR_PlayerScoreInfoFiringRange info = m_aAllPlayersInfo.Get(i);
			if (!info)
				continue;

			string name = info.GetName();
			if (name != m_aPlayerNames.Get(i))
			{
				m_aPlayerNames.Set(i, name);
				changed = true;
			}
		}
		
		return changed;
	}
		
	//------------------------------------------------------------------------------------------------
	// Check if the count of players in game has changed.
	bool PlayerCountChangedInArea()
	{
		// Check if the array exists
		if (!m_aPlayersInArea)
			return false;
		
		// Count how many players is in game and compare it to older value
		int currentPlayersCountInArea = m_aPlayersInArea.Count();
		if (currentPlayersCountInArea == m_iPlayersInAreaCount)
			return false;
		
		m_iPlayersInAreaCount = currentPlayersCountInArea;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//------------------------------------ SCORE PART END --------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{

		if (!m_ScoringSystem)
			return;

		m_ScoringSystem.GetAllPlayersScoreInfo(m_aAllPlayersInfo);

		if (PlayerCountChangedInArea())
		{
			GenerateRows();
			UpdateScoreboardData();
		}
		
		if (ScoreChanged() || NameChanged() || ScoreMaxChanged())
		UpdateScoreboardData();
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity ent)
	{	
		int iLocalPlayerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		RegisterPlayerInArea(iLocalPlayerID);
	
		// Local player ID on client
		IEntity playerEnt = SCR_PlayerController.GetLocalControlledEntity();
		
		if (playerEnt == ent) 
		{
			// Create widget
			m_wRoot = GetGame().GetWorkspace().CreateWidgets("{1B8C7223B314408B}UI/layouts/HUD/FiringRangeScoreboard/FireRangeTable.layout");	
			if (!m_wRoot)
				return;
			
			m_wTable = m_wRoot.FindAnyWidget("Table");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(IEntity ent)
	{		
		int iLocalPlayerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		RemovePlayerFromArea(iLocalPlayerID);

		// Make sure widget is created only on local machine
		int playerID = SCR_PlayerController.GetLocalPlayerId();

		// Local player ID on client
		IEntity playerEnt = SCR_PlayerController.GetLocalControlledEntity();
				
		if (playerEnt == ent) 
		{	
			// Find local player controller
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;
			
			// Find firing range network entity to send RPC to server and Clear player score
			SCR_FiringRangeNetworkEntity firingRangeNetworkEntity = SCR_FiringRangeNetworkEntity.GetInstance();
			if (!firingRangeNetworkEntity)
				return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return if is proxy or not
	bool IsProxy()
	{
		if (!m_RplComponent)
			m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	void GenerateRows()
	{
		if (!m_wTable)
			return;

		// Delete and generate new rows for current players
		foreach (Widget w : m_aPlayerWidgets)
		{
			w.RemoveFromHierarchy();
		}
		m_aPlayerWidgets.Clear();
	
		
		foreach (SCR_PlayerScoreInfoFiringRange info : m_aAllPlayersInfo)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets("{8643D226A0A0A299}UI/layouts/HUD/FiringRangeScoreboard/FireRangeRow.layout", m_wTable);
			m_aPlayerWidgets.Insert(w);
			
			// if the player isn't in Firing range area, hide his line
			int id = info.GetPlayerID();
			if (!IsPlayerInFiringRangeArea(id))
				w.SetVisible(false);
		}
		
		// when it's done, update also the data in the rows for the first time.
		UpdateScoreboardData();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
				
		m_ScoringSystem = SCR_FiringRangeScoringComponent.Cast(owner.FindComponent(SCR_FiringRangeScoringComponent));
				
		if (s_FiringRangeManagerMain == this)
		{			
			gameMode.GetOnPlayerConnected().Insert(SpawnCommunicationPrefab);
			
			array<int> playerIds = new array<int>();
			GetGame().GetPlayerManager().GetPlayers(playerIds);
			
			foreach (int id : playerIds)
			{
				SpawnCommunicationPrefab(id);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Register all firing range controllers into one array
	//! This is happening at the beginning of scenario. So we are able to remove assigned fireline from player in moment he disconects or is killed.
	void RegisterFiringRangeController(IEntity firingLineController)
	{
		m_aFiringLineControllers.Insert(firingLineController);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove assigned firing line and targets from player if he is killed or if he disconects
	void RemoveAssignedPlayerFromFireline(int playerID)
	{
		// check who is the owner of the target and if it's player, reset it to default
		foreach (IEntity currentElement: m_aFiringLineControllers)
		{
			SCR_FiringRangeController controller = SCR_FiringRangeController.Cast(currentElement);
			if (controller.GetFiringLineOwnerId() == playerID)
				controller.BackToDefaultTarget();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get player ID
	int GetPlayerID(IEntity ent)
	{
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Add the player into the array of players in firing range area
	void RegisterPlayerInArea(int playerID)
	{
		m_aPlayersInArea.Insert(playerID);
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove the player from the array of players in firing range area
	void RemovePlayerFromArea(int playerID)
	{
		m_aPlayersInArea.RemoveItem(playerID);		
		Replication.BumpMe();
		
		RpcAsk_ClearWidgets(playerID);
		Rpc(RpcAsk_ClearWidgets, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return true if the ID is in the list of players in firing range area. If not, returns false.
	bool IsPlayerInFiringRangeArea(int playerID)
	{
		if (m_aPlayersInArea.Find(playerID) != -1)
			return true;
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get manager instance
	SCR_FiringRangeManager GetInstance()
	{
		return m_FiringRangeManagerInstance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get MAIN manager instance
	static SCR_FiringRangeManager GetMainManagerInstance()
	{
		return s_FiringRangeManagerMain;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get m_wRoot widget
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get player name
	string GetPlayerName(IEntity ent)
	{
		auto pm = GetGame().GetPlayerManager();
		int iPlayerID = pm.GetPlayerIdFromControlledEntity(ent);
		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(iPlayerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SpawnCommunicationPrefab(int playerID)
	{
		// network entity resource
		Resource resource = Resource.Load(m_ComEnt);
		if (!resource.IsValid())
			return;
	
		IEntity ent = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld());
		if (!ent)
			return;
		
		RplComponent rpl = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rpl)
			return;
		
		SCR_FiringRangeNetworkEntity firingRangeNetworkEntity = SCR_FiringRangeNetworkEntity.Cast(ent);
		if (!firingRangeNetworkEntity)	
			return;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerID);
		if (!playerController) 
			return;
		
		RplIdentity playerRplID = playerController.GetRplIdentity();
		if (playerRplID == Replication.INVALID_IDENTITY)
			return;
		
		rpl.Give(playerRplID);
		firingRangeNetworkEntity.RegisterCommEntity(Replication.FindId(ent));
	}
	
	//------------------------------------------------------------------------------------------------
	//------------------------------------------ MP --------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	void AddIndicator(vector localCoordOfHit, vector localVectorOfHit, IEntity pOwnerEntity)
	
	{
		// find the target controller
		IEntity firingRangeController = pOwnerEntity.GetParent();
		if (!firingRangeController)
			return;
			
		// run it on all clients
		Rpc(RpcAsk_AddIndicator, Replication.FindId(firingRangeController), localCoordOfHit, localVectorOfHit);
		RpcAsk_AddIndicator(Replication.FindId(firingRangeController),localCoordOfHit,localVectorOfHit);
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveIndicators(RplId controllerReplicationId)
	{
		Rpc(RpcAsk_RemoveIndicators, controllerReplicationId);
		RpcAsk_RemoveIndicators(controllerReplicationId);
	}
	
	//------------------------------------------------------------------------------------------------
	void ControllerLight(notnull IEntity firingRangeController, ControllerLightType light, bool mode)
	{
		Rpc(RpcAsk_ControllerLight, Replication.FindId(firingRangeController), light, mode);
		RpcAsk_ControllerLight(Replication.FindId(firingRangeController), light, mode);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetControllerCounter(notnull IEntity firingRangeController, EControlerSection type, int value)
	{
		Rpc(RpcAsk_SetControllerCounter, Replication.FindId(firingRangeController), type, value);
		RpcAsk_SetControllerCounter(Replication.FindId(firingRangeController), type, value);
	}
		
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcAsk_RemoveIndicators(RplId controllerReplicationId)
	{
		SCR_FiringRangeController m_LineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (m_LineController)
			m_LineController.RemoveIndicators();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcAsk_AddIndicator(RplId controllerReplicationId, vector localCoordOfHit, vector localVectorOfHit)
	{
		SCR_FiringRangeController m_LineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (m_LineController)
			m_LineController.AddIndicator(localCoordOfHit,localVectorOfHit);	
		
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcAsk_ClearWidgets(int playerID)
	{
		// Local player ID on client
		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		if (localPlayerID == playerID)
		{
			// Remove all widgets
			m_aPlayerWidgets.Clear();
			if (!m_wRoot)
				return;
			m_wRoot.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcAsk_ControllerLight(RplId controllerReplicationId, ControllerLightType light, bool mode)
	{
		SCR_FiringRangeController m_LineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (m_LineController)
			m_LineController.SetControllerLight(light, mode);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcAsk_SetControllerCounter(RplId controllerReplicationId, EControlerSection type, int value)
	{
		SCR_FiringRangeController m_LineController = SCR_FiringRangeController.Cast(Replication.FindItem(controllerReplicationId));
		if (m_LineController)
			m_LineController.SetControllerCounter(type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CheckMasterOnlyMethod(string methodName)
	{
		if (IsProxy())
		{
			Print("Master-only method (SCR_FiringRangeController." + methodName + ") called on proxy. Some functionality might be broekn!", LogLevel.WARNING);
			return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	
	void SCR_FiringRangeManager(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		//If this instance is the 1st FiringRangeManager in the world, make it main.
		if (s_FiringRangeManagerMain == null)
				s_FiringRangeManagerMain = this;
				
		m_FiringRangeManagerInstance = this;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_FiringRangeManager()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		if (s_FiringRangeManagerMain == this)
			s_FiringRangeManagerMain == null;
	}
};