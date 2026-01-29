[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_NotificationSenderComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_NotificationSenderComponent : SCR_BaseGameModeComponent
{		
	[Attribute("1", desc: "Show player left notification if player left.")]
	protected bool m_bShowPlayerLeftNotification;
	
	[Attribute(EKillFeedType.UNKNOWN_KILLER.ToString(), desc: "Is killfeed enabled and what info will be displayed (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedType))]
	protected EKillFeedType m_iKillFeedType;
	
	[Attribute(EKillFeedReceiveType.GROUP_ONLY.ToString(), desc: "If killfeed is enabled, what is the relationship between the player that died and the local player. (Full killfeed is always displayed in open unlimited Editor)", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedReceiveType))]
	protected EKillFeedReceiveType m_iReceiveKillFeedType;
	
	[Attribute(SCR_EFriendlyFireKillFeedType.SHOW_FRIENDLY_KILLS_TO_TEAM.ToString(), desc: "If on Friendly fire the players in volved or the entire team will be informed who killed whom. If set to default it will use the default killfeed notification", uiwidget : UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EFriendlyFireKillFeedType))]
	protected SCR_EFriendlyFireKillFeedType m_eFriendlyFireKillFeedType;
	
	[Attribute("0", desc: "Array of Killfeed names")]
	protected ref array<ref SCR_NotificationKillfeedTypeName> m_aKillfeedTypeNames;
	
	[Attribute("0", desc: "Array of Killfeed receive names")]
	protected ref array<ref SCR_NotificationKillfeedreceiveTypeName> m_aKillfeedReceiveTypeNames;
	
	[Attribute(desc: "Array of friendly fire Killfeed names")]
	protected ref array<ref SCR_NotificationFriendlyFireKillfeedTypeName> m_aFriendlyFireKillfeedTypeNames;
	
	[Attribute("{D3BFEE28E7D5B6A1}Configs/ServerBrowser/KickDialogs.conf", desc: "If disconnect reason has a preset set to it then it will send a kick/ban notification", params: "conf")]
	protected ref SCR_ConfigurableDialogUiPresets m_PlayerKickReasonsConfig;
	
	protected Faction m_FactionOnSpawn;
	
	protected SCR_FactionManager m_FactionManager;
	
	//States
	protected bool m_bListeningToWeatherChanged;
	
	//------------------------------------------------------------------------------------------------
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		IEntity entity = instigatorContextData.GetVictimEntity();
		
		//~	No entity destroyed or entity not a character
		if (!entity || !ChimeraCharacter.Cast(entity))
			return;
		
		bool isUnlimitedEditor = false;
		
		//~ Check if player has unlimited editor and if the editor is open
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			isUnlimitedEditor = !editorManager.IsLimited();
		
		//~ Killfeed is disabled and unlimited editor is not open
		if (!isUnlimitedEditor && m_iKillFeedType == EKillFeedType.DISABLED)
			return;
		
		IEntity killerEntity = instigatorContextData.GetKillerEntity();
		
		int victimPlayerId = SCR_PossessingManagerComponent.GetPlayerIdFromControlledEntity(entity);
		
		//~ Entity is not a player so do not show notification
		if (victimPlayerId <= 0)
			return;
		
		int localPlayerID = SCR_PlayerController.GetLocalPlayerId();
		Faction localPlayerFaction = m_FactionManager.GetLocalPlayerFaction();
		
		//~ No local faction and player is not GM so don't show killfeed
		if (!localPlayerFaction && !isUnlimitedEditor)
			return;
		
		bool forceFullKillfeed;
		
		//~ Check if the player can show the notification of a player dying
		if (localPlayerFaction && !isUnlimitedEditor && m_iReceiveKillFeedType != EKillFeedReceiveType.ALL)
		{
			//~ No faction manager so don't show killfeed?
			if (!m_FactionManager)
				return;
			
			//~ Get victim faction
			Faction victimFaction;
			if (victimPlayerId > 0)
			{
				 victimFaction = m_FactionManager.GetPlayerFaction(victimPlayerId);
			}
			else if (entity)
			{
				SCR_FactionAffiliationComponent FactionAffiliation = SCR_FactionAffiliationComponent.Cast(entity.FindComponent(SCR_FactionAffiliationComponent));
				if (FactionAffiliation)
					victimFaction = FactionAffiliation.GetAffiliatedFaction();
			}
			
			//~ Is victim faction friendly to local faction
			SCR_Faction scrLocalPlayerFaction = SCR_Faction.Cast(localPlayerFaction);
			bool victimIsFriendly = (scrLocalPlayerFaction && scrLocalPlayerFaction.DoCheckIfFactionFriendly(scrLocalPlayerFaction)) || (!scrLocalPlayerFaction && localPlayerFaction.IsFactionFriendly(victimFaction));
			
			//~ Victim was killed by friendly fire is friendly to local player and not suicide and the friendly fire killfeed is not Default
			if (!isUnlimitedEditor && victimIsFriendly && m_eFriendlyFireKillFeedType != SCR_EFriendlyFireKillFeedType.DEFAULT_SETTINGS && instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
			{
				//~ Show full killfeed if friendly fire involved the a friendly faction
				if (m_eFriendlyFireKillFeedType == SCR_EFriendlyFireKillFeedType.SHOW_FRIENDLY_KILLS_TO_TEAM)
					forceFullKillfeed = true;
				//~ Show full killfeed if friendly fire involved the local player (Either as Victim or Killer)
				else if (m_eFriendlyFireKillFeedType == SCR_EFriendlyFireKillFeedType.SHOW_FRIENDLY_KILLS_TO_PLAYERS_INVOLVED)
					forceFullKillfeed = (localPlayerID == instigatorContextData.GetKillerPlayerID() || localPlayerID == victimPlayerId);
			}
			
			if (!forceFullKillfeed)
			{
				switch (m_iReceiveKillFeedType)
				{
					//~ check if in group
					case EKillFeedReceiveType.GROUP_ONLY :
					{										
						//~ Check if local player is not the same as killed otherwise they are always in the same group
						if (localPlayerID != victimPlayerId)
						{
							//~ Factions not friendly so don't show killfeed
							if (!victimIsFriendly)
								return;
							
							//~ No group manager so don't send
							SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
							if (!groupManager)
								return;
							
							SCR_AIGroup localPlayerGroup = groupManager.GetPlayerGroup(localPlayerID);
							
							//~ If not in a group or not in the same group do not send
							if (!localPlayerGroup || !localPlayerGroup.IsPlayerInGroup(victimPlayerId))
								return;
						}
						
						break;
					}
	
					//~ Check if the same faction
					case EKillFeedReceiveType.SAME_FACTION_ONLY :
					{
						//~ Check if local player is not the same as killed otherwise they are always the same faction
						if (localPlayerID != victimPlayerId)
						{
							//~ If no local faction or if not the same faction do not show killfeed
							if (!localPlayerFaction || localPlayerFaction != victimFaction)
								return;
							
							//~ If Faction is hostile towards itself still do not show killfeed (Deathmatch)
							if (!victimIsFriendly)
								return;
						}
						
						break;
					}
	
					//~ Check if allies
					case EKillFeedReceiveType.ALLIES_ONLY :
					{
						//~ Check if local player is not the same as killed otherwise they are always allied
						if (localPlayerID != victimPlayerId)
						{
							//~ Factions not friendly so don't show killfeed
							if (!victimIsFriendly)
								return;
						}
						
						break;
					}
	
					//~ Check if enemies
					case EKillFeedReceiveType.ENEMIES_ONLY :
					{
						//~ If local player killed it is never an enemy
						if (localPlayerID == victimPlayerId)
							return;
						
						//~ Factions friendly so don't show killfeed
						if (victimIsFriendly)
							return;
						
						break;
					}
				}
			}
		}
		
		SCR_ECharacterControlType victimControlType = instigatorContextData.GetVictimCharacterControlType();
		SCR_ECharacterControlType killerControlType = instigatorContextData.GetKillerCharacterControlType();
		
		int killerPlayerID =  instigatorContextData.GetKillerPlayerID();
		
		//~ Killed by GM so send GM notification
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_UNLIMITED_EDITOR))
		{
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
			{
				//~ Player killed by GM but GM not known
				if (killerPlayerID <= 0)
				{
					//SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_KILLED_BY_EDITOR, victimPlayerId);
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
				}
				//~ Player killed by GM and GM is known
				else 
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_KILLED_BY_EDITOR_PLAYER, victimPlayerId, killerPlayerID);
			}
			else 
			{
				//~ Possessed AI killed by GM but GM not known
				if (killerPlayerID <= 0)
				{
					//SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_BY_EDITOR, victimPlayerId);
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
				}
				//~ Possessed AI killed by GM and GM is known
				else 
					SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_BY_EDITOR_PLAYER, victimPlayerId, killerPlayerID);
			}
			
			return;
		}
		
		//~ Never show killer so simply show player died if limited editor unless forceFullKillFeed is true
		if (!isUnlimitedEditor && m_iKillFeedType == EKillFeedType.UNKNOWN_KILLER && !forceFullKillfeed)
		{
			//Player died
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
			//Possessed AI died
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, victimPlayerId);
			
			return;
		}
		
		//~ Death notification	
		//~ Suicide	
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
		{
			//Player Suicide
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
			//Possessed Suicide
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, victimPlayerId);
			
			return;
		}
		//If killed by other player or player that is has an unlimited editor or the full killfeed is forced
		if (killerControlType == SCR_ECharacterControlType.PLAYER || killerControlType == SCR_ECharacterControlType.UNLIMITED_EDITOR)
		{
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_KILLED_PLAYER, killerPlayerID, victimPlayerId);
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.PLAYER_KILLED_POSSESSED_AI, killerPlayerID, victimPlayerId);
			
			return;
		}
		
		//~ Get RPL component if AI or possessed AI
		RplId killerRplId;
		if ((killerControlType == SCR_ECharacterControlType.AI || killerControlType == SCR_ECharacterControlType.POSSESSED_AI) && killerEntity)
		{
			SCR_EditableCharacterComponent editableCharacterComponent = SCR_EditableCharacterComponent.Cast(killerEntity.FindComponent(SCR_EditableCharacterComponent));
			if (editableCharacterComponent)
				killerRplId = Replication.FindId(editableCharacterComponent);
			
			//~ Invalid killer so show that player simply died
			if (!killerRplId.IsValid())
			{
				//Player died
				if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
					SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
				//Possessed AI died
				else 
					SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, victimPlayerId);
				
				return;
			}
		}
		
		//~ Killed by possessed AI
		if (killerControlType == SCR_ECharacterControlType.POSSESSED_AI)
		{
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
			{
				SCR_NotificationsComponent.SendLocalLimitedEditor(ENotification.AI_KILLED_PLAYER, killerRplId, victimPlayerId);
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_PLAYER, killerPlayerID, victimPlayerId);
			}
			else 
			{
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_KILLED_POSSESSED_AI, killerPlayerID, victimPlayerId);
			}
			
			return;
		}
		//~ Killed by AI
		if (killerControlType == SCR_ECharacterControlType.AI)
		{
			if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
				SCR_NotificationsComponent.SendLocal(ENotification.AI_KILLED_PLAYER, killerRplId, victimPlayerId);
			else 
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.AI_KILLED_POSSESSED_AI, killerRplId, victimPlayerId);

			return;
		}
		
		//~ Unknown killer (Fallback)
		if (victimControlType != SCR_ECharacterControlType.POSSESSED_AI)
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_DIED, victimPlayerId);
		else 
			SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.POSSESSED_AI_DIED, victimPlayerId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorLimitedChanged(bool isLimited)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager) 
			return;
		
		int playerID = editorManager.GetPlayerID();
		
		//Became Player
		if (isLimited)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_NO_LONGER_GM, playerID);
		//Became GM
		else 
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_PLAYER_BECAME_GM, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId) 
	{
		//Join notification
		SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_JOINED, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout) 
	{
		//~ Should never be called for clients but just in case
		if (!GetGameMode().IsMaster())
			return;
		
		SCR_EditorManagerEntity editorManager;
		
		//~ Get editorManager if has any
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (core)
			editorManager = core.GetEditorManager(playerId);

		bool hasUnlimitedEditor = editorManager && !editorManager.IsLimited();
		
		//~ Is GM, Always show GM left notification even if kicked/banned to notify players that the GM has left
		if (hasUnlimitedEditor)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_GM_LEFT, playerId);		
		
		bool isKickedOrBanned = false;
		
		//~ Check if disconnect cause has message attached to it. If true: show kick/ban reason. If false: only show gm/player left
		if (m_PlayerKickReasonsConfig)
		{
			string groupId, reasonId;
			KickCauseGroup2 groupInt;
			int reasonInt;
			
			//~ Get disconnect message preset
			GetGame().GetFullKickReason(cause, groupInt, reasonInt, groupId, reasonId);
			SCR_ConfigurableDialogUiPreset preset = m_PlayerKickReasonsConfig.FindPreset(groupId + "_" + reasonId);
			
			//~ If has kick/Ban message it will send out a notification
			isKickedOrBanned = preset != null && !preset.m_sMessage.IsEmpty();
		}
		//~ No config
		else 
		{
			Print("'SCR_NotificationSenderComponent' has no 'm_PlayerKickReasonsConfig'! Make sure it is added else it will never know if a player was kicked!", LogLevel.ERROR);
		}
		
		//~ Is kicked/banned. Will also send ban notification if for some reason there is a timeout attached even if there is no specific kick message
		if (isKickedOrBanned || timeout != 0)
		{
			SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
			if (dataCollector)
			{
				SCR_PlayerData playerData = dataCollector.GetPlayerData(playerId);
				
				if (playerData)
				{
					float banTimeOut = playerData.GetTimeOut();
				
					//~ If playerData has ban timeout which is greater then timeout use that instead. This is because Heavy ban kicks the player and bans it via backend. So the timeout is set somewhere else
					if (banTimeOut > 0 && banTimeOut > timeout)
						timeout = banTimeOut;
				}
			}
			
			//~ Player kicked 
			if (timeout == 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_KICKED, playerId, KickCauseCodeAPI.GetGroup(cause), KickCauseCodeAPI.GetReason(cause), timeout);
			//~ Player perminent ban
			else if (timeout < 0)
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED_NO_DURATION, playerId, KickCauseCodeAPI.GetGroup(cause), KickCauseCodeAPI.GetReason(cause), timeout);
			//~ Player temp ban
			else
				SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_BANNED, playerId, KickCauseCodeAPI.GetGroup(cause), KickCauseCodeAPI.GetReason(cause), timeout);
		}		
		//~ Is Not kicked/banned, is player and should send on leave notification. 
		else if (m_bShowPlayerLeftNotification && !hasUnlimitedEditor)
		{
			SCR_NotificationsComponent.SendToEveryone(ENotification.PLAYER_LEFT, playerId);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		//Check if faction changed and send notification if diffrent
		if (m_FactionManager)
		{
			int playerId = requestComponent.GetPlayerId();
			//Get factions using ID
			Faction playerFaction = m_FactionManager.GetPlayerFaction(playerId);
		
			//Faction is diffrent
			if (m_FactionOnSpawn != playerFaction)
			{
				//Send player joined faction notification
				SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.PLAYER_JOINED_FACTION, playerId);
				m_FactionOnSpawn = playerFaction;
			}
		}
	}	

	//======================================== WEATHER SET TO LOOPING ========================================\\

	//------------------------------------------------------------------------------------------------
	//! Called when weather is set to looping or when weather itself is changed (which auto sets it on looping)
	//! This is to make sure the notification, that weather is set, is only called once
	//! \param[in] playerID
	void OnWeatherChangedNotification(int playerID)
	{
		if (Replication.IsClient())
			return;
		
		if (!m_bListeningToWeatherChanged)
		{
			m_bListeningToWeatherChanged = true;
			GetGame().GetCallqueue().CallLater(OnWeatherChangedNotificationDelay, 0 , false, playerID);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnWeatherChangedNotificationDelay(int playerID)
	{
		m_bListeningToWeatherChanged = false;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world) 
			return;
		
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager) 
			return;
		
		WeatherState currentState = weatherManager.GetCurrentWeatherState();
		
		if (!currentState)
			return;
		
		SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(ENotification.EDITOR_ATTRIBUTES_WEATHER_CHANGED, playerID, currentState.GetStateID());
	}
	
	//======================================== KILLFEED ========================================\\	

	//------------------------------------------------------------------------------------------------
	//! Get an list of all killfeed types and the localized name
	//! \param[in] killFeedTypeNames list of killfeed type and name
	//! \return the amount of names in the list
	int GetKillFeedTypeNames(notnull array<ref SCR_NotificationKillfeedTypeName> killFeedTypeNames)
	{
		killFeedTypeNames.Clear();
		foreach (SCR_NotificationKillfeedTypeName killfeedTypeName: m_aKillfeedTypeNames)
		{
			killFeedTypeNames.Insert(killfeedTypeName);
		}
		
		return killFeedTypeNames.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get an list of all killfeed receive types and the localized name
	//! \param[in] killFeedReceiveTypeNames list of killfeed receive type and name
	//! \return the amount of names in the list
	int GetKillFeedReceiveTypeNames(notnull array<ref SCR_NotificationKillfeedreceiveTypeName> killFeedReceiveTypeNames)
	{
		killFeedReceiveTypeNames.Clear();
		foreach (SCR_NotificationKillfeedreceiveTypeName killfeedReceiveTypeName: m_aKillfeedReceiveTypeNames)
		{
			killFeedReceiveTypeNames.Insert(killfeedReceiveTypeName);
		}
		
		return killFeedReceiveTypeNames.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the type of killfeed that is currently displayed
	EKillFeedType GetKillFeedType()
	{
		return m_iKillFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server set killfeed type
	//! \param[in] killFeedType new killfeed type to set
	//! \param[in] playerNotificationId add player ID of player that changed the type to display a notification
	void SetKillFeedType(EKillFeedType killFeedType, int playerNotificationId = -1)
	{
		if (!GetGameMode().IsMaster() || killFeedType == m_iKillFeedType)
			return;
		
		SetKillFeedTypeBroadcast(killFeedType);
		Rpc(SetKillFeedTypeBroadcast, killFeedType);
		
		if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_TYPE, playerNotificationId, killFeedType);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetKillFeedTypeBroadcast(EKillFeedType killFeedType)
	{
		m_iKillFeedType = killFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the current killfeed receive type
	EKillFeedReceiveType GetReceiveKillFeedType()
	{
		return m_iReceiveKillFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server set killfeed receive type
	//! \param[in] receiveKillFeedType new killfeed reveive type to set
	//! \param[in] playerNotificationId add player ID of player that changed the type to display a notification
	void SetReceiveKillFeedType(EKillFeedReceiveType receiveKillFeedType, int playerNotificationId = -1)
	{
		if (receiveKillFeedType == m_iReceiveKillFeedType || !GetGameMode().IsMaster())
			return;
		
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		Rpc(SetReceiveKillFeedTypeBroadcast, receiveKillFeedType);
		
		if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_KILLFEED_RECEIVE_TYPE, playerNotificationId, receiveKillFeedType);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetReceiveKillFeedTypeBroadcast(EKillFeedReceiveType receiveKillFeedType)
	{
		m_iReceiveKillFeedType = receiveKillFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the current friendly fire killfeed type
	SCR_EFriendlyFireKillFeedType GetFriendlyFireKillFeedType()
	{
		return m_eFriendlyFireKillFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Server set friendly fire killfeed type
	//! \param[in] friendlyFireKillFeedType new friendly fire killfeed type to set
	//! \param[in] playerNotificationId add player ID of player that changed the type to display a notification
	void SetFriendlyFireKillFeedType(SCR_EFriendlyFireKillFeedType friendlyFireKillFeedType, int playerNotificationId = -1)
	{
		if (friendlyFireKillFeedType == m_eFriendlyFireKillFeedType || !GetGameMode().IsMaster())
			return;
		
		SetFriendlyFireKillFeedTypeBroadcast(friendlyFireKillFeedType);
		Rpc(SetFriendlyFireKillFeedTypeBroadcast, friendlyFireKillFeedType);
		
		if (playerNotificationId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_CHANGED_FRIENDLY_FIRE_KILLFEED_TYPE, playerNotificationId, friendlyFireKillFeedType);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetFriendlyFireKillFeedTypeBroadcast(SCR_EFriendlyFireKillFeedType friendlyFireKillFeedType)
	{
		m_eFriendlyFireKillFeedType = friendlyFireKillFeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get an list of all killfeed receive types and the localized name
	//! \param[in] friendlyFireKillFeedTypeNames list of friendly fire killfeed type and name
	//! \return the amount of names in the list
	int GetFriendlyFireKillFeedTypeNames(notnull out array<ref SCR_NotificationFriendlyFireKillfeedTypeName> friendlyFireKillFeedTypeNames)
	{
		friendlyFireKillFeedTypeNames.Clear();
		foreach (SCR_NotificationFriendlyFireKillfeedTypeName friendlyFireKillfeedTypeName : m_aFriendlyFireKillfeedTypeNames)
		{
			friendlyFireKillFeedTypeNames.Insert(friendlyFireKillfeedTypeName);
		}
		
		return friendlyFireKillFeedTypeNames.Count();
	}
	
	//======================================== RPL ========================================\\

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_iKillFeedType);
		writer.WriteInt(m_iReceiveKillFeedType);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		EKillFeedType killFeedType;
		EKillFeedReceiveType receiveKillFeedType;
		
		reader.ReadInt(killFeedType);
		reader.ReadInt(receiveKillFeedType);

		SetKillFeedTypeBroadcast(killFeedType);
		SetReceiveKillFeedTypeBroadcast(receiveKillFeedType);
		
		return true;
	}
	
	//======================================== INIT ========================================\\

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Insert(OnEditorLimitedChanged);
		
		m_FactionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		super.EOnInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		super.OnPostInit(owner);
	}
	
	//======================================== DELETE ========================================\\

	//------------------------------------------------------------------------------------------------
	protected override void OnDelete(IEntity owner)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
			editorManager.GetOnLimitedChange().Remove(OnEditorLimitedChanged);
		
		super.OnDelete(owner);
	}
}

//! What kind of killfeed will be displayed in the gamemode.
//! Noted that players with unlimited Editor will always see full killfeed if Editor is open
enum EKillFeedType
{
	//~ If changes are made in this enum make sure to change m_aKillfeedTypeNames
	DISABLED = 0,			//!< Killfeed is disabled
	UNKNOWN_KILLER = 10,	//!< Will see killfeed messages but will never know who the killer is
	FULL = 20,				//!< Will see every player (except possessed pawns) killed messages. Including killer
}

//! From who will players receive kill notifications
//! Noted that players with unlimited Editor will always see full killfeed if Editor is open
enum EKillFeedReceiveType
{
	//~ If changes are made in this enum make sure to change m_aKillfeedReceiveTypeNames
	ALL = 0,				//!< Will see killfeed from allies and enemies alike
	ENEMIES_ONLY = 10,		//!< Will see killfeed from enemies only
	ALLIES_ONLY = 20,		//!< Will see killfeed from allies only
	SAME_FACTION_ONLY = 30,	//!< Will see killfeed from same faction only
	GROUP_ONLY = 40,		//!< Will see killfeed from group members only
}

enum SCR_EFriendlyFireKillFeedType
{
	DEFAULT_SETTINGS = 0,				 			//!< Kill feed will be default depending on EKillFeedType and EKillFeedFriendlyKillsType
	SHOW_FRIENDLY_KILLS_TO_PLAYERS_INVOLVED = 10, 	//!< When a player is killed by a friendly faction both they and the killer will see the full notification of the teamkill
	SHOW_FRIENDLY_KILLS_TO_TEAM = 20,				//!< When a player is killed by a friendly faction the both teams will see the full notification of the teamkill
}

//! Class to get Killfeed type name
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EKillFeedType, "m_iKillfeedType")]
class SCR_NotificationKillfeedTypeName
{
	[Attribute(desc: "Killfeed type", uiwidget : UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedType))]
	protected EKillFeedType m_iKillfeedType;
	
	[Attribute(desc: "Name displayed in Notification", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sKillfeedTypeName;
	
	//------------------------------------------------------------------------------------------------
	//! \return the killfeed type enum
	EKillFeedType GetKillfeedType()
	{
		return m_iKillfeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the killfeed type name as string
	string GetName()
	{
		return m_sKillfeedTypeName;
	}
}

//! Class to get Killfeed receive type name
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EKillFeedReceiveType, "m_iKillfeedreceiveType")]
class SCR_NotificationKillfeedreceiveTypeName
{
	[Attribute(desc: "Killfeed receive type", uiwidget : UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EKillFeedReceiveType))]
	protected EKillFeedReceiveType m_iKillfeedreceiveType;
	
	[Attribute(desc: "Name displayed in Notification", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sKillfeedReceiveTypeName;
	
	//------------------------------------------------------------------------------------------------
	//! \return the killfeed receive type enum
	EKillFeedType GetKillfeedReceiveType()
	{
		return m_iKillfeedreceiveType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the killfeed receive type name as string
	string GetName()
	{
		return m_sKillfeedReceiveTypeName;
	}
}

//! Class to get Killfeed receive type name
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EFriendlyFireKillFeedType, "m_eFriendlyFireKillfeedType")]
class SCR_NotificationFriendlyFireKillfeedTypeName
{
	[Attribute(desc: "Friendly fire Killfeed type", uiwidget : UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(SCR_EFriendlyFireKillFeedType))]
	protected SCR_EFriendlyFireKillFeedType m_eFriendlyFireKillfeedType;
	
	[Attribute(desc: "Name displayed in Notification", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sFriendlyFireKillfeedTypeName;
	
	//------------------------------------------------------------------------------------------------
	//! \return the friendly fire killfeed type enum
	SCR_EFriendlyFireKillFeedType GetFriendlyFireKillfeedType()
	{
		return m_eFriendlyFireKillfeedType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the killfeed receive type name as string
	string GetName()
	{
		return m_sFriendlyFireKillfeedTypeName;
	}
}
