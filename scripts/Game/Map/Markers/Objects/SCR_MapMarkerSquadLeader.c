//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Markers")]
class SCR_MapMarkerSquadLeaderClass : SCR_MapMarkerEntityClass
{
	[Attribute(defvalue: "1", desc:"Can show other squad leaders to this squad leader")]
	protected bool m_bCanLeaderSeeOtherLeaders;

	[Attribute(defvalue: "1", desc:"Can show other squad leaders to this squad member, member will see only own leader")]
	protected bool m_bCanMemberSeeOtherLeaders;

	//------------------------------------------------------------------------------------------------
	bool CanLeaderSeeOtherLeaders()
	{
		return m_bCanLeaderSeeOtherLeaders;
	}

	//------------------------------------------------------------------------------------------------
	bool CanMemberSeeOtherLeaders()
	{
		return m_bCanMemberSeeOtherLeaders;
	}
}

//------------------------------------------------------------------------------------------------
//! Dynamic map marker -> squad leader
class SCR_MapMarkerSquadLeader : SCR_MapMarkerEntity
{
	[RplProp(onRplName: "OnPlayerIdUpdate")]
	protected int m_PlayerID;							// target ID, needed for visibility rules and fetching group

	bool m_bDoGroupTextUpdate;							// group text update flag
	protected bool m_bDoGroupSymbolUpdate;				// group symbol update flag
	protected SCR_AIGroup m_Group;
	protected SCR_MapMarkerSquadLeaderComponent m_SquadLeaderWidgetComp;
	protected bool m_bDoLocalVisibilityUpdate;
		
	//------------------------------------------------------------------------------------------------
	// RPL EVENTS
	//------------------------------------------------------------------------------------------------
	void OnPlayerIdUpdate()
	{
		m_bDoLocalVisibilityUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	// API SERVER
	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int id)
	{
		m_PlayerID = id;
		m_MarkerFaction = SCR_FactionManager.SGetPlayerFaction(id);
		
		Replication.BumpMe();
		
		if (!System.IsConsoleApp())
			OnPlayerIdUpdate();
		
		UpdateTarget();
	}
	
	//------------------------------------------------------------------------------------------------
	// EVENTS & OTHERS
	//------------------------------------------------------------------------------------------------
	void SetTextUpdate()
	{
		m_bDoGroupTextUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetGroup()
	{
		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AssignGroup()
	{
		SCR_GroupsManagerComponent comp = SCR_GroupsManagerComponent.GetInstance();
		if (!comp)
			return;
		
		m_Group = comp.GetPlayerGroup(m_PlayerID);
		
		if (m_Group)
			SCR_MapMarkerEntrySquadLeader.Cast(m_ConfigEntry).RegisterMarker(this, m_Group);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Target tracking based on playerID 
	//! Authority only
	protected void UpdateTarget()
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_PlayerID);
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
	//! Set military symbol image, can change during lifetime
	protected void UpdateGroupMilitarySymbol()
	{
		if (!m_Group)
		{
			AssignGroup();
			return;
		}
		
		string company, platoon, squad, character, format;
		m_Group.GetCallsigns(company, platoon, squad, character, format);

	 	SCR_Faction faction = SCR_Faction.Cast(m_Group.GetFaction());
		if (faction)
		{
			string flag = m_Group.GetGroupFlag();
			if (flag == string.Empty)
				flag = faction.GetFlagName(0);
			
			SetImage(faction.GetGroupFlagImageSet(), flag);
		}
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetImage(m_sImageset, m_sIconName);
		
		UpdatePlayerAffiliation();
		m_bDoGroupSymbolUpdate = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set group text, can change during lifetime
	protected void UpdateGroupText()
	{
		if (!m_Group)
		{
			AssignGroup();
			return;
		}
		
		// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
		SetText(SCR_GroupHelperUI.GetTranslatedGroupNameAndRoleName(m_Group));
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetText(m_sText);
		
		m_bDoGroupTextUpdate = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether we are in a squad and if it should be visible on map
	void UpdatePlayerAffiliation()
	{	
		if (!m_wRoot)
			return;
		
		if (m_Group.IsPlayerInGroup(GetGame().GetPlayerController().GetPlayerId()))
			m_SquadLeaderWidgetComp.SetGroupActive(true, m_Group.GetFactionName());
		else
			m_SquadLeaderWidgetComp.SetGroupActive(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether we are in a squad and if it should be visible on map
	void UpdateLocalVisibility()
	{
		m_bDoLocalVisibilityUpdate = false;

		PlayerController pController = GetGame().GetPlayerController();
		if (!pController)
			return;

		if (m_PlayerID == pController.GetPlayerId())
		{
			// if this is us, dont display
			SetLocalVisible(false);
			return;
		}

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup localPlayerGroup = groupManager.GetPlayerGroup(pController.GetPlayerId());
		if (!localPlayerGroup)
		{
			SetLocalVisible(false);
			return;
		}

		bool isLocalPlayerLeader = localPlayerGroup.IsPlayerLeader(pController.GetPlayerId());

		if (isLocalPlayerLeader && CanLeaderSeeOtherLeaders())
		{
			SetLocalVisible(true);
			return;
		}

		if (!isLocalPlayerLeader && (CanMemberSeeOtherLeaders() || localPlayerGroup.IsPlayerInGroup(m_PlayerID)))
		{
			SetLocalVisible(true);
			return;
		}

		SetLocalVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanLeaderSeeOtherLeaders()
	{
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		SCR_MapMarkerSquadLeaderClass prefabData = SCR_MapMarkerSquadLeaderClass.Cast(GetPrefabData());
		if (!prefabData)
			return true;

		return prefabData.CanLeaderSeeOtherLeaders();
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanMemberSeeOtherLeaders()
	{
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return true;

		SCR_MapMarkerSquadLeaderClass prefabData = SCR_MapMarkerSquadLeaderClass.Cast(GetPrefabData());
		if (!prefabData)
			return true;

		return prefabData.CanMemberSeeOtherLeaders();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerLeaderChanged(int groupID, int playerId)
	{
		if (m_PlayerID <= 0)
			return;

		m_bDoLocalVisibilityUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerAdded(SCR_AIGroup group, int playerId)
	{
		if (m_PlayerID <= 0)
			return;

		m_bDoLocalVisibilityUpdate = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		if (!faction || faction != SCR_FactionManager.SGetLocalPlayerFaction())
			return;

		m_bDoLocalVisibilityUpdate = true
	}

	//------------------------------------------------------------------------------------------------
	//! Update names when user settings are changed (f.e. xbox UGC)
	protected void OnUserSettingsChanged()
	{
		m_bDoGroupTextUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_AIGroup event
	//! Triggers when squad flag is changed
	protected void OnFlagSelected()
	{
		m_bDoGroupSymbolUpdate = true;
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override protected void OnMapLayerChanged(int layerID)
	{
		super.OnMapLayerChanged(layerID);
		
		if (m_SquadLeaderWidgetComp)
			m_SquadLeaderWidgetComp.SetLayerID(layerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCreateMarker()
	{
		RplComponent rplComp = RplComponent.Cast(FindComponent(RplComponent));
		if (rplComp.IsOwner())	// authority only
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_PlayerID);
			if (ent)
				SetTarget(ent);
		}
		
		Faction markerFaction = SCR_FactionManager.SGetPlayerFaction(m_PlayerID);	
		Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();	
		if (!localFaction || localFaction.IsFactionEnemy(markerFaction))	// markers could still get streamed in rare cases due to distance based streaming, in which case we check for faction and dont display
			return;	
			
		super.OnCreateMarker();
		
		m_bDoGroupSymbolUpdate = true;
		m_bDoGroupTextUpdate = true;
		
		m_SquadLeaderWidgetComp = SCR_MapMarkerSquadLeaderComponent.Cast(m_MarkerWidgetComp);
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		SCR_AIGroup.GetOnFlagSelected().Insert(OnFlagSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete()
	{
		super.OnDelete();
		
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		SCR_AIGroup.GetOnFlagSelected().Remove(OnFlagSelected);
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
		
		if (m_bDoGroupSymbolUpdate)
			UpdateGroupMilitarySymbol();
		
		if (m_bDoGroupTextUpdate)
			UpdateGroupText();
		
		if (m_SquadLeaderWidgetComp.m_bIsHovered)
			m_SquadLeaderWidgetComp.UpdateGroupInfoPosition(m_iScreenX, m_iScreenY);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnPlayerLeaderChanged);

		SCR_FactionCommanderHandlerComponent factionCommanderHandler = SCR_FactionCommanderHandlerComponent.GetInstance();
		if (factionCommanderHandler)
			factionCommanderHandler.GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MapMarkerSquadLeader()
	{
		if (m_ConfigEntry)
			SCR_MapMarkerEntrySquadLeader.Cast(m_ConfigEntry).UnregisterMarker(m_Group);

		SCR_AIGroup.GetOnPlayerAdded().Remove(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(OnPlayerLeaderChanged);

		SCR_FactionCommanderHandlerComponent factionCommanderHandler = SCR_FactionCommanderHandlerComponent.GetInstance();
		if (factionCommanderHandler)
			factionCommanderHandler.GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);
	}
}
