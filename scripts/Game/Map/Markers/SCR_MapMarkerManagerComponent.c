[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_MapMarkerManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Map marker manager, keeps array of markers and takes care of synchronization
//! Attached to GameMode entity
class SCR_MapMarkerManagerComponent : SCR_BaseGameModeComponent
{	
	[Attribute("{720E8E61D7692172}Configs/Map/MapMarkerConfig.conf", UIWidgets.ResourceNamePicker )]
	protected ResourceName m_sMarkerCfgPath;
	
	protected static SCR_MapMarkerManagerComponent s_Instance;
	
	protected ref array<ref SCR_MapMarkerBase> m_aStaticMarkers = {};		// local or replicated static markers, one time RPC call for sync
	protected ref array<ref SCR_MapMarkerBase> m_aDisabledMarkers = {};		// disabled static markers
	protected ref array<SCR_MapMarkerEntity> m_aDynamicMarkers = {};		// dynamically replicated markers
	
	protected ref array<ref SCR_ScriptProfanityFilterRequestCallback> m_aProfanityCallbacks = {};		// profanity callbacks of markers
	
	protected int m_iID;					// server side only, unique id created for markers
	protected vector m_vVisibleFrameMin;	// screen coords of the visible frame in map - min point
	protected vector m_vVisibleFrameMax;	// screen coords of the visible frame in map - max point
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapMarkerSyncComponent m_MarkerSyncComp;
	protected ref SCR_MapMarkerConfig m_MarkerCfg;
		
	//------------------------------------------------------------------------------------------------
	//! GETTERS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_MapMarkerManagerComponent GetInstance() 
	{ 
		return s_Instance; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_MapMarkerConfig GetMarkerConfig()
	{
		return m_MarkerCfg;
	}
		
	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_MapMarkerBase> GetStaticMarkers()
	{
		array<SCR_MapMarkerBase> output = {};
		foreach (SCR_MapMarkerBase marker: m_aStaticMarkers)
		{
			output.Insert(marker);
		}

		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_MapMarkerBase> GetDisabledMarkers()
	{
		array<SCR_MapMarkerBase> output = {};
		foreach (SCR_MapMarkerBase marker: m_aDisabledMarkers)
		{
			output.Insert(marker);
		}

		return output;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_MapMarkerEntity> GetDynamicMarkers()
	{
		return m_aDynamicMarkers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] w
	//! \return
	SCR_MapMarkerBase GetMarkerByWidget(Widget w)
	{		
		foreach ( SCR_MapMarkerBase marker : m_aStaticMarkers )
		{
			if (w == marker.GetRootWidget())
				return marker;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] markerID
	//! \return
	SCR_MapMarkerBase GetStaticMarkerByID(int markerID)
	{
		foreach ( SCR_MapMarkerBase marker : m_aStaticMarkers )
		{
			if (markerID == marker.GetMarkerID())
				return marker;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] markerID
	//! \return
	SCR_MapMarkerBase GetDisabledMarkerByID(int markerID)
	{
		foreach (SCR_MapMarkerBase marker : m_aDisabledMarkers)
		{
			if (markerID == marker.GetMarkerID())
				return marker;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] type
	//! \param[in] target
	//! \return
	SCR_MapMarkerEntity GetDynamicMarkerByTarget(SCR_EMapMarkerType type, IEntity target)
	{
		foreach (SCR_MapMarkerEntity marker : m_aDynamicMarkers)
		{
			if (type == marker.GetType() && target == marker.GetTarget())
				return marker;
		}
		
		return null;
	}
		
	//------------------------------------------------------------------------------------------------
	//! SPAWN/REMOVE API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Insert predefined type of static marker
	//! \param[in] type is primary marker type
	//! \param[in] worldX is x world poosition
	//! \param[in] worldY is y world poosition
	//! \param[in] isLocal determiens whether the marker will be created client side only or synchronized to other eligible players
	//! \param[in] configId is secondary marker ID used to select a predefined subtype
	//! \param[in] factionFLags determine which factions are able to see the marker, 0 for everyone
	void InsertStaticMarkerByType(SCR_EMapMarkerType type, int worldX, int worldY, bool isLocal, int configId = -1, int factionFlags = 0, bool isServerMarker = false)
	{
		if (!m_MarkerCfg)
			return;
				
		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(type);
		marker.SetWorldPos(worldX, worldY);
		marker.SetMarkerConfigID(configId);
		
		if (factionFlags != 0)
			marker.SetMarkerFactionFlags(factionFlags);
		
		InsertStaticMarker(marker, isLocal, isServerMarker);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Insert customized static marker
	//! \param[in] marker is the subject
	//! \param[in] isLocal determines whether the marker is synchronised to other players or local
	//! \param[in] isServerMarker determines whether the marker is spawned by server and should not count towards marker limits (different from server-client spawning his own markers)
	void InsertStaticMarker(SCR_MapMarkerBase marker, bool isLocal, bool isServerMarker = false)
	{
		if (isLocal)	// local
		{
			marker.SetMarkerOwnerID(GetGame().GetPlayerController().GetPlayerId());
			m_aStaticMarkers.Insert(marker);
			marker.OnCreateMarker();
		}
		else 
		{
			if (isServerMarker)
			{
				AssignMarkerUID(marker);
				marker.SetMarkerOwnerID(-1);
				
				OnAddSynchedMarker(marker);	// add server side
				OnAskAddStaticMarker(marker);
			}
			else 
			{
				if (!m_MarkerSyncComp)
				{
					if (!FindMarkerSyncComponent())
						return;
				}
				
				m_MarkerSyncComp.AskAddStaticMarker(marker);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepare military marker based on existing configure attributes within marker config
	//! \param[in] faction
	//! \param[in] dimension
	//! \param[in] typeFlags
	//! \return military marker with set symbol parameters for network synchronisation
	SCR_MapMarkerBase PrepareMilitaryMarker(EMilitarySymbolIdentity faction, EMilitarySymbolDimension dimension, EMilitarySymbolIcon typeFlags)
	{
		if (!m_MarkerCfg)
			return null;
		
		SCR_MapMarkerEntryMilitary militaryConfig = SCR_MapMarkerEntryMilitary.Cast(m_MarkerCfg.GetMarkerEntryConfigByType(SCR_EMapMarkerType.PLACED_MILITARY));
		if (!militaryConfig)
			return null;
		
		// check whether the provided parameters are defined within marker config
		int factionID = militaryConfig.GetFactionEntryID(faction);
		int dimensionID = militaryConfig.GetDimensionEntryID(dimension);
		if (factionID == -1 || dimensionID == -1)
			return null;
		
		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_MILITARY);
		marker.SetMarkerConfigID(dimensionID * 100 + factionID); // combination of faction and dimension id
		marker.SetFlags(typeFlags);
		
		return marker;
	}
				
	//------------------------------------------------------------------------------------------------
	//! Authority only
	//! \param[in] type
	//! \param[in] entity
	//! \param[in] configId
	//! \return
	SCR_MapMarkerEntity InsertDynamicMarker(SCR_EMapMarkerType type, IEntity entity, int configId = -1)
	{
		if (!m_pGameMode.IsMaster())
			return null;
				
		SCR_MapMarkerEntryDynamic cfgEntry = SCR_MapMarkerEntryDynamic.Cast(m_MarkerCfg.GetMarkerEntryConfigByType(type));
		if (!cfgEntry)
			return null;
		
		Resource prefab = Resource.Load(cfgEntry.GetMarkerPrefab());
		
		SCR_MapMarkerEntity markerEnt = SCR_MapMarkerEntity.Cast(GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld()));
		if (!markerEnt)
			return null;
		
		markerEnt.SetType(type, configId);
		markerEnt.SetTarget(entity);
		
		return markerEnt;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove a static marker
	//! \param[in] marker
	void RemoveStaticMarker(SCR_MapMarkerBase marker)
	{
		if (marker.GetMarkerID() == -1)	// local
		{
			marker.OnDelete();
			m_aStaticMarkers.RemoveItem(marker);
		}
		else 							// synched
		{
			if (marker.GetMarkerOwnerID() == -1)	// is server marker
			{
				if (!Replication.IsServer())		// cannot delete server makers from client
					return;
				
				OnRemoveSynchedMarker(marker.GetMarkerID());
				OnAskRemoveStaticMarker(marker.GetMarkerID());
			}
			else 
			{
				if (!m_MarkerSyncComp)
				{
					if (!FindMarkerSyncComponent())
						return;
				}
				
				m_MarkerSyncComp.AskRemoveStaticMarker(marker.GetMarkerID());
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Authority only
	//! \param[in] ent
	void RemoveDynamicMarker(notnull SCR_MapMarkerEntity ent)
	{
		if (!m_pGameMode.IsMaster())
			return;
		
		RplComponent.DeleteRplEntity(ent, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void AssignMarkerUID(SCR_MapMarkerBase marker)
	{
		if (!Replication.IsServer())
			return;
		
		if (m_iID == int.MAX)
			m_iID == 0;
		else
			m_iID++;
		
		marker.SetMarkerID(m_iID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable stream out of specific marker for target identity
	//! \param[in] marker is the subject marker
	//! \param[in] pController PlayerController is the subject player's controller
	//! \param[in] state sets target state of stream out -> enabled if true, meaning that replication will stream out the subject
	protected void HandleStreamOut(SCR_MapMarkerEntity marker, PlayerController pController, bool state)
	{
		RplComponent rplComponent = RplComponent.Cast(marker.FindComponent(RplComponent));
		if (!rplComponent || !pController)
			return;
				
		RplIdentity identity = pController.GetRplIdentity();
		if (!identity.IsValid())
			return;
		
		rplComponent.EnableStreamingConNode(identity, state);
		
		if (identity == RplIdentity.Local())	// if this is a hosted server, we cannot control visibilty through streaming, and so it has to be set manually
			marker.SetLocalVisible(!state);
	} 
	
	//------------------------------------------------------------------------------------------------
	//! Network streaming rules for the marker
	//! Authority only -> Called when marker faction is assigned
	//! \param[in] marker
	void SetMarkerStreamRules(notnull SCR_MapMarkerEntity marker)
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		
		foreach (int player : players)
		{
			Faction markerFaction = marker.GetFaction();
			
			if (!markerFaction || markerFaction == SCR_FactionManager.SGetPlayerFaction(player))
				HandleStreamOut(marker, GetGame().GetPlayerManager().GetPlayerController(player), false);
			else
				HandleStreamOut(marker, GetGame().GetPlayerManager().GetPlayerController(player), true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Network streaming rules of all markers of player
	//! Authority only ->  Set when player is spawned
	//! \param[in] playerID
	void SetStreamRulesForPlayer(int playerID)
	{			
		foreach (SCR_MapMarkerEntity dynamicMarker : m_aDynamicMarkers)
		{
			Faction markerFaction = dynamicMarker.GetFaction();
			
			if (!markerFaction || markerFaction == SCR_FactionManager.SGetPlayerFaction(playerID))
				HandleStreamOut(dynamicMarker, GetGame().GetPlayerManager().GetPlayerController(playerID), false);
			else
				HandleStreamOut(dynamicMarker, GetGame().GetPlayerManager().GetPlayerController(playerID), true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set manager to run UI updates for markers
	//! \param[in] state
	void EnableUpdate(bool state)
	{
		if (state)
			ConnectToMarkerManagerSystem();
		else 
			DisconnectFromMarkerManagerSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FindMarkerSyncComponent()
	{
		PlayerController playerContr =  GetGame().GetPlayerController();
		if (!playerContr)
			return false;
		
		m_MarkerSyncComp = SCR_MapMarkerSyncComponent.Cast(playerContr.FindComponent(SCR_MapMarkerSyncComponent));
		if (m_MarkerSyncComp)
			return true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable/disable static marker
	//! \param[in] marker
	//! \param[in] state
	void SetStaticMarkerDisabled(notnull SCR_MapMarkerBase marker, bool state)
	{
		if (state || marker.GetBlocked())
		{
			if (!m_aDisabledMarkers.Contains(marker))
			{
				m_aDisabledMarkers.Insert(marker);
				m_aStaticMarkers.RemoveItem(marker);
			}
			
			marker.SetUpdateDisabled(true);
		}
		else
		{
			if (!m_aStaticMarkers.Contains(marker))
			{
				m_aStaticMarkers.Insert(marker);
				m_aDisabledMarkers.RemoveItem(marker);
			}
			
			marker.SetUpdateDisabled(false);
			
			if (!marker.GetRootWidget())	// happens when map is closed and widget ref is lost
				marker.OnCreateMarker();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers the marker within manager, called by the marker entity
	//! \param[in] markerEnt
	void RegisterDynamicMarker(SCR_MapMarkerEntity markerEnt)
	{
		if (!m_aDynamicMarkers.Contains(markerEnt))
			m_aDynamicMarkers.Insert(markerEnt);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Unregisters the marker within manager, called by the marker entity
	//! \param[in] markerEnt
	void UnregisterDynamicMarker(SCR_MapMarkerEntity markerEnt)
	{
		if (m_aDynamicMarkers.Contains(markerEnt))
			m_aDynamicMarkers.RemoveItem(markerEnt);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToMarkerManagerSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_MapMarkerManagerSystem markerSystem = SCR_MapMarkerManagerSystem.Cast(world.FindSystem(SCR_MapMarkerManagerSystem));
		if (!markerSystem)
			return;
		
		markerSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromMarkerManagerSystem()
	{
		World world = GetOwner().GetWorld();
		SCR_MapMarkerManagerSystem markerSystem = SCR_MapMarkerManagerSystem.Cast(world.FindSystem(SCR_MapMarkerManagerSystem));
		if (!markerSystem)
			return;
		
		markerSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		m_MapEntity.GetMapVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax);
		
		for (int i = m_aStaticMarkers.Count() - 1; i >= 0; i--)
		{
			if (!m_aStaticMarkers[i].OnUpdate(m_vVisibleFrameMin, m_vVisibleFrameMax))
				SetStaticMarkerDisabled(m_aStaticMarkers[i], true);
			else
				SetStaticMarkerDisabled(m_aStaticMarkers[i], false);
		}
				
		foreach (SCR_MapMarkerEntity markerEnt : m_aDynamicMarkers)
		{
			if (markerEnt)
				markerEnt.OnUpdate();
		}
		
		#ifdef MARKERS_DEBUG
			UpdateDebug(timeSlice);
		#endif 
	}
	
	#ifdef MARKERS_DEBUG
	//------------------------------------------------------------------------------------------------
	void UpdateDebug(float timeSlice)
	{
		DbgUI.Begin("Markers debug");
		string dbg = "disabled: %1 | static: %2 | dynamic: %3 ";
		DbgUI.Text( string.Format( dbg, m_aDisabledMarkers.Count(), m_aStaticMarkers.Count(), m_aDynamicMarkers.Count() ) );
		
		/*string line =  "name: %1 | distance: %2 | opacity base: %3 | opacity fade: %4 | %5";
		foreach ( SCR_NameTagData tag : m_aNameTags )
		{
			DbgUI.Text( string.Format( line, tag.m_sName, (int)tag.m_fDistance, tag.m_fVisibleOpacity, tag.m_fOpacityFade, tag.m_NameTagWidget.GetOpacity() ));
		}*/
		
		DbgUI.End();
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	// EVENTS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! RPC result of marker add broadcast
	//! Is an event and should NOT be called directly, it is only public since server needs to call it from SCR_MapMarkerSyncComponent
	//! \param[in] marker
	void OnAddSynchedMarker(SCR_MapMarkerBase marker)
	{									
		m_aStaticMarkers.Insert(marker);
		FactionManager factionManager = GetGame().GetFactionManager();
		
		if (System.IsConsoleApp())
		{
			marker.SetServerDisabled(true);
		}
		else
		{
			if (marker.GetMarkerOwnerID() > -1)//player made marker
				marker.RequestProfanityFilter();

			if (factionManager && marker.GetMarkerFactionFlags() != 0)
			{
				Faction localFaction = SCR_FactionManager.SGetLocalPlayerFaction();
				bool isMyFaction = marker.IsFaction(factionManager.GetFactionIndex(localFaction));
				
				if (!localFaction || !isMyFaction)
				{
					if (Replication.IsServer())				// hosted server 
						marker.SetServerDisabled(true);
					else 
						m_aStaticMarkers.RemoveItem(marker);
	
					return;
				}
			}
		}

		SCR_MapEntity mapEnt = SCR_MapEntity.GetMapInstance();
		if (mapEnt.IsOpen() && mapEnt.GetMapUIComponent(SCR_MapMarkersUI))		
			marker.OnCreateMarker(true);
		
		CheckMarkersUserRestrictions();
	}
	
	//------------------------------------------------------------------------------------------------
	//! RPC result of marker remove broadcast
	//! Is an event and should NOT be called directly, it is only public since server needs to call it from SCR_MapMaprkerSyncComponent
	//! \param[in] markerID
	void OnRemoveSynchedMarker(int markerID)
	{			
		SCR_MapMarkerBase marker = GetStaticMarkerByID(markerID);
		if (marker)
		{
			m_aStaticMarkers.RemoveItem(marker);
		}
		else
		{
			marker = GetDisabledMarkerByID(markerID);
			if (!marker)
				return;

			m_aDisabledMarkers.RemoveItem(marker);
		}
		
		marker.OnDelete();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side call from sync component for RPC marker add broadcast
	//! \param[in] markerData
	void OnAskAddStaticMarker(SCR_MapMarkerBase markerData)
	{
		Rpc(RPC_DoAddStaticMarker, markerData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server side call from sync component for RPC marker remove broadcast
	//! \param[in] markerID
	void OnAskRemoveStaticMarker(int markerID)
	{
		Rpc(RPC_DoRemoveStaticMarker, markerID);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    protected void RPC_DoAddStaticMarker(SCR_MapMarkerBase markerData)
    {			
		OnAddSynchedMarker(markerData);
    }
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
    protected void RPC_DoRemoveStaticMarker(int markerID)
    {		
		OnRemoveSynchedMarker(markerID);
    }
	
	//------------------------------------------------------------------------------------------------
	//! Faction manager event -> server only
	//! \param[in] playerID
	//! \param[in] playerComponent
	//! \param[in] faction
	void OnPlayerFactionChanged_S(int playerID, SCR_PlayerFactionAffiliationComponent playerComponent, Faction faction)
	{
		SetStreamRulesForPlayer(playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapPanEnd(float x, float y)
	{
		m_MapEntity.GetMapVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax);
		
		for (int i = m_aDisabledMarkers.Count() - 1; i >= 0; i--)
		{
			if (m_aDisabledMarkers[i].TestVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax))
				SetStaticMarkerDisabled(m_aDisabledMarkers[i], false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckMarkersUserRestrictions()
	{
		SocialComponent sc = GetSocialComponent();
		if (!sc || System.IsConsoleApp())
			return;
		
		for (int i = m_aStaticMarkers.Count() - 1; i >= 0; i--)
		{
			m_aStaticMarkers[i].SetBlocked(!sc.IsPrivilegedTo(EUserInteraction.UserGeneratedContent) || sc.IsRestricted(m_aStaticMarkers[i].GetMarkerOwnerID(), EUserInteraction.UserGeneratedContent));
			SetStaticMarkerDisabled(m_aStaticMarkers[i], !m_aStaticMarkers[i].TestVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax));
		}
		
		for (int i = m_aDisabledMarkers.Count() - 1; i >= 0; i--)
		{
			m_aDisabledMarkers[i].SetBlocked(!sc.IsPrivilegedTo(EUserInteraction.UserGeneratedContent) || sc.IsRestricted(m_aDisabledMarkers[i].GetMarkerOwnerID(), EUserInteraction.UserGeneratedContent));
			SetStaticMarkerDisabled(m_aDisabledMarkers[i], !m_aDisabledMarkers[i].TestVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration mapConfig)
	{
		for (int i = m_aDisabledMarkers.Count() - 1; i >= 0; i--)
		{
			SetStaticMarkerDisabled(m_aDisabledMarkers[i], !m_aDisabledMarkers[i].TestVisibleFrame(m_vVisibleFrameMin, m_vVisibleFrameMax));
		}
		
		CheckMarkersUserRestrictions();
	}
	
	//------------------------------------------------------------------------------------------------
	protected SocialComponent GetSocialComponent()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;
		
		return SocialComponent.Cast(pc.FindComponent(SocialComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	//	UTILITIES
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	SCR_ScriptProfanityFilterRequestCallback RequestProfanityFilter(string text)
	{
		if (text.IsEmpty())
			return null;
		
		return RequestProfanityFilter({text});
	}

	//------------------------------------------------------------------------------------------------
	//! Profanity filter processing that is intended to do multiple strings at once
	//! \param[in] textsToFilter
	SCR_ScriptProfanityFilterRequestCallback RequestProfanityFilter(notnull array<string> textsToFilter)
	{
		if (textsToFilter.IsEmpty())
			return null;

		SCR_ScriptProfanityFilterRequestCallback profanityFilter = new SCR_ScriptProfanityFilterRequestCallback();

		if (!GetGame().GetPlatformService().FilterProfanityAsync(textsToFilter, profanityFilter))
			return null;

		m_aProfanityCallbacks.Insert(profanityFilter);

		return profanityFilter;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilteredCallback(array<string> texts)
	{
		bool isPlatformXbox = GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.XBOX;
		
		int i;
		foreach (SCR_MapMarkerBase marker: m_aStaticMarkers)
		{
			if (marker.GetMarkerOwnerID() == -1)
				continue;

			if (!texts.IsIndexValid(i))
				break;
			

			if (isPlatformXbox)
			{
				string resultText;
				SCR_ProfaneFilter.ReplaceProfanities(texts[i], resultText);
				marker.SetCustomText(resultText);
			}
			else
			{
				marker.SetCustomText(texts[i]);
			}
			i++;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		if (!m_pGameMode.IsMaster())	// server only
			return;
		
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);			
		SCR_MapMarkerSyncComponent markerSyncComp = SCR_MapMarkerSyncComponent.Cast(playerController.FindComponent(SCR_MapMarkerSyncComponent));
		if (markerSyncComp)
			markerSyncComp.ClearOwnedMarkers();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected bool RplSave(ScriptBitWriter writer)
	{
		int count = 0;
		
		// We want to replicate also the disabled markers to clients in the case of Listen server in coop missions
		array<SCR_MapMarkerBase> markersSimple = GetStaticMarkers();
		foreach(SCR_MapMarkerBase markerDis : GetDisabledMarkers())
		{
			markersSimple.Insert(markerDis);
		}
		
		if (markersSimple.IsEmpty())
		{
			writer.WriteInt(count);
			return true;
		}
		
		foreach (SCR_MapMarkerBase marker : markersSimple)
		{
			if (marker.GetMarkerID() == -1)
				continue;
			else
				count++;
		}
		
		writer.WriteInt(count);
		
		WorldTimestamp timestamp;

		foreach (SCR_MapMarkerBase marker : markersSimple)
		{
			if (marker.GetMarkerID() == -1)
				continue;
			
			int pos[2];
			marker.GetWorldPos(pos);
			
			writer.WriteInt(pos[0]);
			writer.WriteInt(pos[1]);
			writer.WriteInt(marker.GetMarkerID());
			writer.WriteInt(marker.GetMarkerOwnerID());
			writer.WriteInt(marker.GetFlags());
			writer.WriteInt(marker.GetMarkerConfigID());
			writer.WriteInt(marker.GetMarkerFactionFlags());
			writer.Write(marker.GetRotation(), 16);		// 2 bytes are enough
			writer.Write(marker.GetType(), 8);			// 1 byte is enough, not expected to go over 256 types
			writer.Write(marker.GetColorEntry(), 8);	// 1 byte is enough, no expected over 256 colors
			writer.Write(marker.GetIconEntry(), 16);	// 1 byte should be enough, leaving the extra byte to not underestimate modders
			writer.WriteString(marker.GetCustomText());
			writer.WriteBool(marker.IsTimestampVisible());
			if (marker.IsTimestampVisible())
			{
				timestamp = marker.GetTimestamp();
				writer.Write(timestamp, 64); // added timestamp
			}
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected bool RplLoad(ScriptBitReader reader)
	{
		int count;
		reader.ReadInt(count);
		if (count == 0)
			return true;
		
		int posX, posY, markerID, markerOwnerID, flags, markerConfigID, factionFlags, markerType, colorID, iconID, rotation;
		string customText;
		bool isTimestampVisible;
		WorldTimestamp timestamp;
		SCR_MapMarkerBase marker;
		array<string> textsToFilter = {};
		
		for (int i; i < count; i++)
		{	
				
			reader.ReadInt(posX);
			reader.ReadInt(posY);
			reader.ReadInt(markerID);
			reader.ReadInt(markerOwnerID);
			reader.ReadInt(flags);
			reader.ReadInt(markerConfigID);
			reader.ReadInt(factionFlags);
			reader.Read(rotation, 16);
			reader.Read(markerType, 8);
			reader.Read(colorID, 8);
			reader.Read(iconID, 16);
			reader.ReadString(customText);
			reader.ReadBool(isTimestampVisible);
			if (isTimestampVisible)
				reader.Read(timestamp, 64);
			
			marker = new SCR_MapMarkerBase();
			marker.SetType(markerType);
			marker.SetWorldPos(posX, posY);
			marker.SetMarkerID(markerID);
			marker.SetMarkerOwnerID(markerOwnerID);
			marker.SetFlags(flags);
			marker.SetMarkerConfigID(markerConfigID);
			marker.SetMarkerFactionFlags(factionFlags);
			marker.SetRotation(rotation);
			marker.SetColorEntry(colorID);
			marker.SetIconEntry(iconID);
			marker.SetCustomText(customText);

			marker.SetTimestampVisibility(isTimestampVisible);
			if (isTimestampVisible)
				marker.SetTimestamp(timestamp);
						
			m_aStaticMarkers.Insert(marker);
			if (marker.GetMarkerOwnerID() > -1)
				textsToFilter.Insert(marker.GetCustomText());
		}
		
		if (!textsToFilter.IsEmpty())
		{
			SCR_ScriptProfanityFilterRequestCallback cb = RequestProfanityFilter(textsToFilter);
			if (cb)
				cb.m_OnResult.Insert(OnFilteredCallback);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{	
		bool isMaster = m_pGameMode.IsMaster();
		
		array<ref SCR_MapMarkerEntryConfig> entryCfgs = m_MarkerCfg.GetMarkerEntryConfigs();
		foreach ( SCR_MapMarkerEntryConfig cfg : entryCfgs )
		{
			SCR_MapMarkerEntryDynamic entryDynamic = SCR_MapMarkerEntryDynamic.Cast(cfg);
			if (entryDynamic)
			{
				if (isMaster)
					entryDynamic.InitServerLogic();
				
				entryDynamic.InitClientLogic();
			}
		}
		
		if (!isMaster)	// init server logic below
			return;
		
		SCR_FactionManager mgr = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (mgr)
			mgr.GetOnPlayerFactionChanged_S().Insert(OnPlayerFactionChanged_S);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_MapEntity.GetOnMapPanEnd().Insert(OnMapPanEnd);
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		
		Resource container = BaseContainerTools.LoadContainer(m_sMarkerCfgPath);
		if (container)
			m_MarkerCfg = SCR_MapMarkerConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));
		
		SetEventMask(owner, EntityEvent.INIT);
	} 
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_MapMarkerManagerComponent()
	{
		s_Instance = null;
	}
}
