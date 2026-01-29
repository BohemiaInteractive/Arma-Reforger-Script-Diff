//------------------------------------------------------------------------------------------------
//! Squad leader marker entry 
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntrySquadLeader: SCR_MapMarkerEntryDynamic
{
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected SCR_AIGroup m_bCurrentSquad;					// saved active squad of the current player 
	
	protected ref map<SCR_AIGroup, SCR_MapMarkerSquadLeader> m_mGroupMarkers = new map<SCR_AIGroup, SCR_MapMarkerSquadLeader>();
	
	//------------------------------------------------------------------------------------------------
	//! Register marker here so it can be fetched from the map
	void RegisterMarker(SCR_MapMarkerSquadLeader marker, SCR_AIGroup group)
	{
		m_mGroupMarkers.Insert(group, marker);
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterMarker(SCR_AIGroup group)
	{
		m_mGroupMarkers.Remove(group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Pans the map to your own squad
	protected void FocusCurrentSquad()
	{
		if (!m_bCurrentSquad || !m_ToolMenuEntry.IsEntryEnabled())
			return;
		
		SCR_MapMarkerSquadLeader leaderEnt = m_mGroupMarkers.Get(m_bCurrentSquad);
		if (leaderEnt)
		{
			vector wPos = leaderEnt.GetWorldPos();
			int screenX, screenY;
			
			SCR_MapEntity.GetMapInstance().WorldToScreen(wPos[0], wPos[2], screenX, screenY);
			SCR_MapEntity.GetMapInstance().PanSmooth(screenX, screenY);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables/disables the map tool button for panning to your squad 
	protected void UpdateToolEntryState()
	{
		if (m_ToolMenuEntry)
		{
			if (m_bCurrentSquad != null && GetGame().GetPlayerController().GetPlayerId() != m_bCurrentSquad.GetLeaderID())	// currently squad in squad && we are not the leader
				m_ToolMenuEntry.SetEnabled(true);
			else 
				m_ToolMenuEntry.SetEnabled(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_GroupsManagerComponent event
	protected void OnPlayableGroupCreated(SCR_AIGroup group)
	{		
		SCR_MapMarkerSquadLeader marker = SCR_MapMarkerSquadLeader.Cast(m_MarkerMgr.InsertDynamicMarker(SCR_EMapMarkerType.SQUAD_LEADER, group));
		if (!marker)
			return;
		
		marker.SetFaction(group.GetFaction());
		RegisterMarker(marker, group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_GroupsManagerComponent event
	protected void OnPlayableGroupRemoved(SCR_AIGroup group)
	{
		SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
		if (marker)
			m_MarkerMgr.RemoveDynamicMarker(marker);
		
		UnregisterMarker(group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerLeaderChanged(int groupID, int playerId)
	{			
		SCR_AIGroup group = m_GroupsManager.FindGroup(groupID);
		SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
		if (marker)
			marker.SetPlayerID(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerAdded(SCR_AIGroup group, int playerId)
	{			
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (group.IsPlayerInGroup(playerController.GetPlayerId() == playerId))
		{
			m_bCurrentSquad = group;
			UpdateToolEntryState();
			
			SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
			if (marker)
				marker.UpdatePlayerAffiliation();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerRemoved(SCR_AIGroup group, int playerId)
	{			
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;
		
		if (m_bCurrentSquad == group && group.IsPlayerInGroup(playerController.GetPlayerId() == playerId))
		{
			m_bCurrentSquad = null;
			UpdateToolEntryState();
			
			SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
			if (marker)
				marker.UpdatePlayerAffiliation();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerSpawned(int playerId, IEntity player)
	{
		UpdateMarkerTarget(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerKilled( notnull SCR_InstigatorContextData instigatorContextData)
	{
		UpdateMarkerTarget(instigatorContextData.GetVictimPlayerID());
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{		
		UpdateMarkerTarget(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnGroupCustomNameChanged(SCR_AIGroup group)
	{
		SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
		if (marker)
			marker.SetTextUpdate();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update marker target, will trigger creation of a marker if within map
	protected void UpdateMarkerTarget(int playerID)
	{
		SCR_AIGroup group = m_GroupsManager.GetPlayerGroup(playerID);
		if (!group || !group.IsPlayerLeader(playerID))					// ignore if not leader
			return;
		
		SCR_MapMarkerSquadLeader marker = m_mGroupMarkers.Get(group);
		if (marker)
			marker.SetPlayerID(playerID);
	} 
	
	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
	 	return SCR_EMapMarkerType.SQUAD_LEADER;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitServerLogic()
	{	
		super.InitServerLogic();
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_GroupsManager.GetOnPlayableGroupCreated().Insert(OnPlayableGroupCreated);
		m_GroupsManager.GetOnPlayableGroupRemoved().Insert(OnPlayableGroupRemoved);
		
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnPlayerLeaderChanged);
		
		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitClientLogic()
	{
		SCR_AIGroup.GetOnCustomNameChanged().Insert(OnGroupCustomNameChanged);
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapLayerChangedDynamic(notnull SCR_MapMarkerDynamicWComponent widgetComp, int layerID)
	{
		if (layerID > 1) 
			widgetComp.SetTextVisible(false);
		else
			widgetComp.SetTextVisible(true);	
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(notnull SCR_MapEntity mapEnt, notnull SCR_MapMarkersUI markerUIComp)
	{
		super.OnMapOpen(mapEnt, markerUIComp);
		
		UpdateToolEntryState();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapInit(notnull SCR_MapEntity mapEnt, notnull SCR_MapMarkersUI markerUIComp)
	{
		super.OnMapInit(mapEnt, markerUIComp);
		
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(mapEnt.GetMapUIComponent(SCR_MapToolMenuUI));
		if (!toolMenu)
			return;
		
		m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", "squad", 20); // add to menu
		m_ToolMenuEntry.m_OnClick.Insert(FocusCurrentSquad);
		m_ToolMenuEntry.SetEnabled(true);
	}
}