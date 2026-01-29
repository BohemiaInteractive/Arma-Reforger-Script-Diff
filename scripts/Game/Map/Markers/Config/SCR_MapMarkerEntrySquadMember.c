//! Squad leader marker entry
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntrySquadMember : SCR_MapMarkerEntryDynamic
{
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected ref map<int, SCR_MapMarkerSquadMember> m_mGroupMemberMarkers = new map<int, SCR_MapMarkerSquadMember>();// <playerID, marker>

	//------------------------------------------------------------------------------------------------
	//! Register marker
	//! \param[in] marker
	//! \param[in] playerID
	void RegisterMarker(SCR_MapMarkerSquadMember marker, int playerID)
	{
		m_mGroupMemberMarkers.Insert(playerID, marker);
	}

	//------------------------------------------------------------------------------------------------
	//! Unregister marker
	//! \param[in] playerID
	void UnregisterMarker(int playerID)
	{
		m_mGroupMemberMarkers.Remove(playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerSpawned(int playerId, IEntity player)
	{
		if (m_mGroupMemberMarkers.Contains(playerId))
			return;

		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return;

		Faction faction = SCR_FactionManager.SGetPlayerFaction(playerId);
		if (!faction)
			return;

		SCR_MapMarkerSquadMember marker = SCR_MapMarkerSquadMember.Cast(m_MarkerMgr.InsertDynamicMarker(SCR_EMapMarkerType.SQUAD_MEMBER, player));
		if (!marker)
			return;

		marker.SetFaction(faction);
		marker.SetPlayerID(playerId);
		marker.UpdatePlayerAffiliation();
		RegisterMarker(marker, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		SCR_MapMarkerSquadMember marker = m_mGroupMemberMarkers.Get(instigatorContextData.GetVictimPlayerID());
		if (marker)
			m_MarkerMgr.RemoveDynamicMarker(marker);

		UnregisterMarker(instigatorContextData.GetVictimPlayerID());
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{
		SCR_MapMarkerSquadMember marker = m_mGroupMemberMarkers.Get(playerId);
		if (marker)
			m_MarkerMgr.RemoveDynamicMarker(marker);

		UnregisterMarker(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
		return SCR_EMapMarkerType.SQUAD_MEMBER;
	}

	//------------------------------------------------------------------------------------------------
	override void InitClientSettingsDynamic(notnull SCR_MapMarkerEntity marker, notnull SCR_MapMarkerDynamicWComponent widgetComp)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void InitServerLogic()
	{
		super.InitServerLogic();

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned);
		gameMode.GetOnPlayerKilled().Insert(OnPlayerKilled);
		gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapLayerChangedDynamic(notnull SCR_MapMarkerDynamicWComponent widgetComp, int layerID)
	{
		if (layerID > 1)
			widgetComp.SetTextVisible(false);
		else
			widgetComp.SetTextVisible(true);
	}
}
