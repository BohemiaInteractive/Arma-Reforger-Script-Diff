[EntityEditorProps(category: "GameScripted/Markers")]
class SCR_MapMarkerSquadMemberClass : SCR_MapMarkerEntityClass
{
}

//! Dynamic map marker -> squad member
class SCR_MapMarkerSquadMember : SCR_MapMarkerEntity
{
	[RplProp(onRplName: "OnPlayerIdUpdate")]
	protected int m_iPlayerID;							// target ID, needed for visibility rules and fetching group

	protected SCR_MapMarkerSquadMemberComponent m_SquadMemberWidgetComp;
	protected bool m_bDoLocalVisibilityUpdate;

	//------------------------------------------------------------------------------------------------
	//! Check whether we are in a squad and if it should be visible on map
	void UpdateLocalVisibility()
	{
		m_bDoLocalVisibilityUpdate = false;

		PlayerController pController = GetGame().GetPlayerController();
		if (!pController)
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		if (m_iPlayerID == pController.GetPlayerId())	// if this is us, dont display
		{
			SetLocalVisible(false);
			return;
		}

		SCR_AIGroup localPlayerGroup = groupManager.GetPlayerGroup(pController.GetPlayerId());

		if (!localPlayerGroup)
		{
			SetLocalVisible(false);
			return;
		}

		if (localPlayerGroup.IsPlayerLeader(pController.GetPlayerId()) && localPlayerGroup.IsPlayerInGroup(m_iPlayerID))
		{
			// player to whom the mark belongs is in the group
			SetLocalVisible(true);
			UpdatePlayerAffiliation();
			return;
		}

		SetLocalVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Rpl event when m_iPlayerID is updated
	void OnPlayerIdUpdate()
	{
		UpdateLocalVisibility();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] id
	void SetPlayerID(int id)
	{
		m_iPlayerID = id;
		m_MarkerFaction = SCR_FactionManager.SGetPlayerFaction(id);

		Replication.BumpMe();

		if (!System.IsConsoleApp())
			OnPlayerIdUpdate();

		UpdateTarget();
	}

	//------------------------------------------------------------------------------------------------
	//! Target tracking based on playerID
	//! Authority only
	protected void UpdateTarget()
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPlayerID);
		if (ent)
		{
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
			if (!charController.IsDead())
			{
				SetTarget(ent);
				SetGlobalVisible(true);

				return;
			}
		}

		SetTarget(null);
		SetGlobalVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Check whether we are in a squad and if it should be visible on map
	void UpdatePlayerAffiliation()
	{
		if (!m_wRoot)
			return;

		if (!m_MarkerFaction)
			m_MarkerFaction = SCR_FactionManager.SGetPlayerFaction(m_iPlayerID);

		if (!m_MarkerFaction)
			return;

		m_SquadMemberWidgetComp.SetFaction(m_MarkerFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerLeaderChanged(int groupID, int playerId)
	{
		if (m_iPlayerID != playerId)
			return;

		m_bDoLocalVisibilityUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerAdded(SCR_AIGroup group, int playerId)
	{
		if (m_iPlayerID != playerId)
			return;

		m_bDoLocalVisibilityUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	protected void OnPlayerRemoved(SCR_AIGroup group, int playerId)
	{
		if (m_iPlayerID != playerId)
			return;

		m_bDoLocalVisibilityUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMapLayerChanged(int layerID)
	{
		super.OnMapLayerChanged(layerID);

		if (m_SquadMemberWidgetComp)
			m_SquadMemberWidgetComp.SetLayerID(layerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCreateMarker()
	{
		RplComponent rplComp = RplComponent.Cast(FindComponent(RplComponent));
		if (rplComp.IsOwner())
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iPlayerID);
			if (ent)
				SetTarget(ent);
		}

		Faction markerFaction = SCR_FactionManager.SGetPlayerFaction(m_iPlayerID);
		Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!localFaction || localFaction.IsFactionEnemy(markerFaction))	// markers could still get streamed in rare cases due to distance based streaming, in which case we check for faction and dont display
			return;

		super.OnCreateMarker();

		m_bDoLocalVisibilityUpdate = true;

		m_SquadMemberWidgetComp = SCR_MapMarkerSquadMemberComponent.Cast(m_MarkerWidgetComp);
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate()
	{
		// updates only if the marker is visible and when the map is open
		if (m_bDoLocalVisibilityUpdate)
			UpdateLocalVisibility();

		if (!m_wRoot)
			return;

		super.OnUpdate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnPlayerLeaderChanged);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ~SCR_MapMarkerSquadMember()
	{
		SCR_AIGroup.GetOnPlayerAdded().Remove(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(OnPlayerLeaderChanged);
	}
}
