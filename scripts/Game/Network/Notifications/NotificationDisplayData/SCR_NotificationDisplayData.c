/*!
Used to get the display information for each notification.
This is the most basic of the SCR_NotificationDisplayData and can only display a simple string without references.
SCR_NotificationData should not have any parameters set.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ENotification, "m_NotificationKey")]
class SCR_NotificationDisplayData
{
	[Attribute("0", UIWidgets.SearchComboBox, "Notification", "", ParamEnumArray.FromEnum(ENotification) )]
	ENotification m_NotificationKey;
	
	[Attribute()]
	bool m_bPriorityNotification;
	
	[Attribute("0", desc: "If true will add the (Friendly) indicator to any entities named in the notification that are friendly towards the local player that recieved the notification")]
	protected bool m_bAddFriendlyIndicator;
	
	[Attribute(desc: "Holds the display information of the notification, Fill in Name and Color. Optional: Icon")]
	ref SCR_UINotificationInfo m_info;	
	
	//Max amount of Characters a username can be. (Xbox = 12, Playstation = 16, Steam = 32)
	const int MAX_USERNAME_CHARACTERS = 32;
	
	//! String used to indicate friendly entities together with the name eg: (Friendly) [PlayerName]. Used only if m_bAddFriendlyIndicator is true
	protected const LocalizedString FRIENDLY_INDICATOR = "#AR-Notification_FriendlyIndicator";
	
	protected const LocalizedString LOCAL_PLAYER_INDICATOR = "#AR-Notification_LocalPlayerIndicator";
	
	protected static const float PLATFORM_ICON_SIZE = 2;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets the initial display data such as initial notification position and faction related color of notification
	\param data SCR_NotificationData to get the parameters
	*/
	void SetInitialDisplayData(SCR_NotificationData data)
	{
		//Save Initial positions
		if (m_info.GetEditorSetPositionData() != ENotificationSetPositionData.NEVER_AUTO_SET_POSITION)
			SetPosition(data);
			
		//Save faction related color. Is ignored if no faction related color type is assigned
		SetFactionRelatedColor(data);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the display data
	\param data SCR_NotificationData to get the parameters
	\param[out] icon as ResourceName
	\param[out] colorEnum as ENotificationColor to be used by the SCR_NotificationMessageUIComponent to get the color from SCR_NotificationsLogDisplay
	*/
	void GetDisplayVisualizationData(SCR_NotificationData data, out SCR_UINotificationInfo info = null, out ENotificationColor colorEnum = ENotificationColor.NEUTRAL)
	{
		info = m_info;
		
		colorEnum = m_info.GetNotificationColor();

		if (colorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			colorEnum = data.GetFactionRelatedColor();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the display data. Only valid if UI info is SCR_ColoredTextNotificationUIInfo
	\param data SCR_NotificationData to get the parameters
	\return colorEnum as ENotificationColor to be used by the SCR_NotificationMessageUIComponent to get the color from SCR_NotificationsLogDisplay
	*/
	ENotificationColor GetTextColor(SCR_NotificationData data)
	{
		SCR_ColoredTextNotificationUIInfo coloredTextInfo = SCR_ColoredTextNotificationUIInfo.Cast(m_info);
		if (!coloredTextInfo)
			return ENotificationColor.NEUTRAL;
		
		ENotificationColor colorEnum = coloredTextInfo.GetNotificationTextColor();

		if (colorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			colorEnum = data.GetFactionRelatedTextColor();
		
		return colorEnum;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the display data
	\param data SCR_NotificationData to get the parameters
	\param[out] leftTextColorEnum as ENotificationColor to be used by the SCR_SplitNotificationMessageUIComponent to get the color from SCR_NotificationsLogDisplay
	\param[out] rightTextColorEnum as ENotificationColor to be used by the SCR_SplitNotificationMessageUIComponent to get the color from SCR_NotificationsLogDisplay
	*/
	void GetSplitNotificationTextColors(SCR_NotificationData data, out ENotificationColor leftTextColorEnum = ENotificationColor.NEUTRAL, out ENotificationColor rightTextColorEnum = ENotificationColor.NEUTRAL)
	{
		SCR_SplitNotificationUIInfo splitNotificationUIInfo = SCR_SplitNotificationUIInfo.Cast(m_info);
		
		if (!splitNotificationUIInfo)
			return;
		
		ENotificationColor leftFactionColor, rightFactionColor;
		data.GetSplitFactionRelatedColor(leftFactionColor, rightFactionColor);
		
		leftTextColorEnum = splitNotificationUIInfo.GetLeftTextColor();
		
		if (leftTextColorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE || splitNotificationUIInfo.ShouldReplaceLeftColorWithRightColorIfAlly())
			leftTextColorEnum = leftFactionColor;
		
		rightTextColorEnum = splitNotificationUIInfo.GetRightTextColor();
		
		if (rightTextColorEnum >= ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			rightTextColorEnum = rightFactionColor;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Returns the display text using the parameters set in SCR_NotificationData
	\param data SCR_NotificationData to get the parameters
	\return string display text as shown in the notification
	*/
	string GetText(SCR_NotificationData data)
	{
		if (!m_info)
		{
			Print("(" + typename.EnumToString(ENotification, data.GetID()) + ") SCR_NotificationDisplayData has no UIInfo assigned!", LogLevel.WARNING); 
			return string.Empty;
		} 
		
		return m_info.GetName();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	If true it will merge param1 with param2 using param2 as format (eg: Translate(param2, param1)) before constructing the full notification formatted string. Param2 will need %1 to in the localization. 
	Mainly used by TaskStateChanged as the objective name (param2) includes the location name (param1) which need to be formatted before formatting the entire string.
	\return bool True if must merge
	*/
	bool MergeParam1With2()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Priority
	\return bool
	*/
	bool GetPriority()
	{
		return m_bPriorityNotification;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get UI info
	\return string Ui info holding various of display information for the notification
	*/
	SCR_UINotificationInfo GetNotificationUIInfo()
	{
		return m_info;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetPlayerName(int playerID, out string playerName)
	{
		//~ Name already assigned
		if (!playerName.IsEmpty())
			return true;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return false;
		
		playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
				
		if (playerName.Length() > MAX_USERNAME_CHARACTERS)
		{
			string trimedName = playerName.Substring(0, MAX_USERNAME_CHARACTERS);
			playerName = trimedName + "...";
		}
		
		PlatformKind playerPlatform = playerManager.GetPlatformKind(SCR_PlayerController.GetLocalPlayerId());
		
		//! If the local player is on PSN, show a platform Icon in front of the name.
		if (!playerName.IsEmpty() && playerPlatform == PlatformKind.PSN)
		{
			if (playerManager.GetPlatformKind(playerID) == PlatformKind.PSN)
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
			else
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
		}
		
		if (SCR_PlayerController.GetLocalPlayerId() == playerID)
			playerName = WidgetManager.Translate(LOCAL_PLAYER_INDICATOR, playerName);
		else if (!playerName.IsEmpty() && m_bAddFriendlyIndicator && IsPlayerFriendlyToLocalPlayer(playerID))
			playerName = WidgetManager.Translate(FRIENDLY_INDICATOR, playerName);
			
		return !playerName.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetEditableEntityName(int entityRplID, out string entityName, bool useCharacterName = false)
	{
		//~ Name already assigned
		if (!entityName.IsEmpty())
			return true;
		
		//Get target Entity. Also works with non-editable entity iD
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityRplID));
		if (!editableEntity)
		{
			IEntity entity = IEntity.Cast(Replication.FindItem(entityRplID));
			if (!entity)
			{
				RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(entityRplID));
				if (rplComponent)
					entity = rplComponent.GetEntity();
			}
			if (entity)
				editableEntity = SCR_EditableEntityComponent.Cast(entity.FindComponent(SCR_EditableEntityComponent));
		}
		
		//~ Editable entity component
		if (editableEntity)
		{
			if (editableEntity.GetEntityType() != EEditableEntityType.TASK)
			{
				//~ Entity is a player so use that name instead
				if (editableEntity.GetPlayerID() > 0)
					return GetPlayerName(editableEntity.GetPlayerID(), entityName);
				
				if (useCharacterName)
				{
					if (SCR_EditableCharacterComponent.Cast(editableEntity))
					{
						string format, firstname, alias, surname;
						if (GetCharacterName(entityRplID, format, firstname, alias, surname))
						{
							entityName = WidgetManager.Translate(format, firstname, alias, surname);
							if (!entityName.IsEmpty())
							{
								if (m_bAddFriendlyIndicator && IsEntityFriendlyToLocalPlayer(editableEntity.GetOwner()))
									entityName = WidgetManager.Translate(FRIENDLY_INDICATOR, entityName);
									
								return true;
							}
						}
					}
				}
				
				//~ Use entity name
				entityName = editableEntity.GetDisplayName();
			}
			//~ Get objective type name
			else 
			{
				SCR_EditableTaskComponentClass taskPrefabData = SCR_EditableTaskComponentClass.Cast(editableEntity.GetComponentData(editableEntity.GetOwner()));
				if (taskPrefabData)
					entityName = taskPrefabData.GetObjectiveTypeName();
				else 
					entityName = editableEntity.GetDisplayName();
			}
		}
		
		if (!entityName.IsEmpty() && m_bAddFriendlyIndicator && IsEntityFriendlyToLocalPlayer(editableEntity.GetOwner()))
			entityName = WidgetManager.Translate(FRIENDLY_INDICATOR, entityName);

		return !entityName.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerFriendlyToLocalPlayer(int notificationPlayerID, bool friendlyIfNoFaction = false)
	{
		//~ Is player self no need to show friendly
		if (SCR_PlayerController.GetLocalPlayerId() == notificationPlayerID)
			return false;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager) 
			return false;
		
		Faction notificationPlayerFaction = factionManager.GetPlayerFaction(notificationPlayerID);
		if (!notificationPlayerFaction)
			return friendlyIfNoFaction;
		
		//Get local player faction
		Faction playerSelfFaction = factionManager.GetLocalPlayerFaction();
		if (!playerSelfFaction) 
			return friendlyIfNoFaction;
		
		return playerSelfFaction.IsFactionFriendly(notificationPlayerFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsEntityFriendlyToLocalPlayer(IEntity notificationEntity, bool friendlyIfNoFaction = false)
	{		
		//If no entity given or the entity is the same as local controlled entity return false
		if (!notificationEntity || SCR_PlayerController.GetLocalControlledEntity() == notificationEntity) 
			return false;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager) 
			return false;
		
		Faction notificationEntityFaction;
		
		//Get notification entity ChimeraCharacter
		SCR_ChimeraCharacter notificationEntityChimera = SCR_ChimeraCharacter.Cast(notificationEntity);
		if (notificationEntityChimera)
		{
			notificationEntityFaction = notificationEntityChimera.GetFaction();
		}
		else 
		{
			FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(notificationEntity.FindComponent(FactionAffiliationComponent));
			if (factionAffiliationComponent)
				notificationEntityFaction = factionAffiliationComponent.GetAffiliatedFaction();
		}
			
		if (!notificationEntityFaction)
			return friendlyIfNoFaction;
		
		//Get local player faction
		Faction playerSelfFaction = factionManager.GetLocalPlayerFaction();
		if (!playerSelfFaction) 
			return friendlyIfNoFaction;

		return playerSelfFaction.IsFactionFriendly(notificationEntityFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Returns faction name by index
	protected bool GetFactionName(int factionIndex, out string factionName)
	{
		//~ Name already assigned
		if (!factionName.IsEmpty())
			return true;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return false;
		
		Faction faction = factionManager.GetFactionByIndex(factionIndex);
		if (!faction)
			return false;
		
		factionName = faction.GetFactionName();
		return !factionName.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetCharacterName(int entityRplID, out string format, out string firstname, out string alias, out string surname)
	{
		//Get target Entity. Also works with non-editable entity iD
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entityRplID));
		if (!editableEntity)
		{
			IEntity entity = IEntity.Cast(Replication.FindItem(entityRplID));
			if (!entity)
			{
				RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(entityRplID));
				if (rplComponent)
					entity = rplComponent.GetEntity();
			}
			if (entity)
				editableEntity = SCR_EditableEntityComponent.Cast(entity.FindComponent(SCR_EditableEntityComponent));
			
			if (!editableEntity)
				return false;
		}

		SCR_CharacterIdentityComponent charIdentity = SCR_CharacterIdentityComponent.Cast(editableEntity.GetOwner().FindComponent(SCR_CharacterIdentityComponent));
		if (!charIdentity)
    		return false;
		
		charIdentity.GetFormattedFullName(format, firstname, alias, surname);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Returns translated callsign in correct formatting
	protected bool GetEntityCallsign(int rplID, out string callsign)
	{
		//~ Name already assigned
		if (!callsign.IsEmpty())
			return true;
		
		Managed managed = Replication.FindItem(rplID);
		if (!managed)
    		return false;
		
		SCR_CallsignBaseComponent callsignComponent;
		ScriptComponent scriptComp = ScriptComponent.Cast(managed);
		
		//~ Get managed script
		if (scriptComp)
		{
			callsignComponent = SCR_CallsignBaseComponent.Cast(scriptComp.GetOwner().FindComponent(SCR_CallsignBaseComponent));
		}
		//~ Get managed entity
		else 
		{
			IEntity entity = IEntity.Cast(managed);
			if (!entity)
				return false;
			
			callsignComponent =  SCR_CallsignBaseComponent.Cast(entity.FindComponent(SCR_CallsignBaseComponent));
		}
		
		if (!callsignComponent)
			return false;
		
		string company, platoon, squad, character, format;
		if (!callsignComponent.GetCallsignNames(company, platoon, squad, character, format))
			return false;
		
		//~ Translate the callsign strings so it can be send as 1 string
		callsign = WidgetManager.Translate(format, company, platoon, squad, character);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Returns group name, or callsign if no name assigned
	protected bool GetGroupNameFromGroupID(int playerGroupId, out string groupName)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;
		
		SCR_AIGroup aiGroup;
		aiGroup = groupsManager.FindGroup(playerGroupId);
		
		if (!aiGroup)
			return false;

		groupName = aiGroup.GetCustomName();
		
		//~ No custom name set so get Callsign instead
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(groupName))
			return GetEntityCallsign(Replication.FindId(aiGroup), groupName);
			
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get inventory item name
	protected bool GetInventoryItemName(RplId rplId, out string itemName)
	{
		if (!itemName.IsEmpty())
			return true;
		
		RplComponent itemRplcComp = RplComponent.Cast(Replication.FindItem(rplId));
		if (!itemRplcComp)
			return false;
		
		IEntity itemEntity = itemRplcComp.GetEntity();
		if (!itemEntity)
			return false;
		
		InventoryItemComponent item = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
		if (!item)
			return false;
		
		ItemAttributeCollection attributeCollection = item.GetAttributes();
		if (!attributeCollection)
			return false;
		
		UIInfo uiInfo = attributeCollection.GetUIInfo();
		if (!uiInfo)
			return false;
		
		itemName = uiInfo.GetName();
		
		return !itemName.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Gives a position vector that is linked to the notification. 
	This is for editor functionality so the GM can teleport to the notification positions
	Will return false if specific SCR_NotificationDisplayData does not have a linked position
	\param data SCR_NotificationData to get the parameters
	\param[out] position vector the position linked to the notification
	\return bool returns false if has no position saved
	*/
	bool GetPosition(SCR_NotificationData data, out vector position)
	{
		if (m_info.GetEditorSetPositionData() == ENotificationSetPositionData.AUTO_SET_AND_UPDATE_POSITION)
			SetPosition(data);
		
		data.GetPosition(position);
		
		if (position == vector.Zero)
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set the position of the target entity the SCR_NotificationData
	\param data SCR_NotificationData to get the parameters
	*/
	void SetPosition(SCR_NotificationData data)
	{
	
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool CanSetPosition(SCR_NotificationData data)
	{
		vector position;
		data.GetPosition(position);
		
		return !((m_info.GetEditorSetPositionData() == ENotificationSetPositionData.AUTO_SET_POSITION_ONCE && position != vector.Zero) || m_info.GetEditorSetPositionData() == ENotificationSetPositionData.NEVER_AUTO_SET_POSITION);
	}
	
	//------------------------------------------------------------------------------------------------
	//Set position data using Player ID
	protected void SetPositionDataEditablePlayer(int playerID, SCR_NotificationData data)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		SCR_EditableEntityComponent playerEntity = SCR_EditableEntityComponent.GetEditableEntity(playerManager.GetPlayerControlledEntity(playerID));
		
		if (!playerEntity) 
			return;
		
		vector position;
		
		if (!playerEntity.GetPos(position)) 
			return;
		
		data.SetPosition(position);
	}
	
	//------------------------------------------------------------------------------------------------
	//Set position data using Editable Entity ID
	protected void SetPositionDataEditableEntity(int enditableEntityID, SCR_NotificationData data)
	{
		SCR_EditableEntityComponent targetEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(enditableEntityID));

		if (!targetEntity) return;
		
		vector position;
		if (!targetEntity.GetPos(position)) return;
		
		data.SetPosition(position);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Sets the color enum related to faction if SCR_NotificationDisplayData Color is set to: 'FACTION_RELATED_FRIENDLY_IS_NEGATIVE', 'FACTION_RELATED_FRIENDLY_IS_POSITIVE', 'FACTION_RELATED_ENEMY_IS_NEGATIVE_ONLY' or 'FACTION_RELATED_FRIENDLY_IS_POSITIVE_ONLY' and there is a target to check faction of.
	\param data
	*/
	void SetFactionRelatedColor(SCR_NotificationData data)
	{

	}
	
	//------------------------------------------------------------------------------------------------
	//Get faction related color of player target
	protected ENotificationColor GetFactionRelatedColorPlayer(int notificationPlayerID, ENotificationColor colorType)
	{
		//If not a faction set color keep current color
		if (colorType < ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			return colorType;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager) 
			return ENotificationColor.NEUTRAL;
		
		Faction notificationPlayerFaction = factionManager.GetPlayerFaction(notificationPlayerID);
		
		//~ Notification color is equal to faction color
		if (colorType == ENotificationColor.FACTION_COLOR)
		{
			if (!notificationPlayerFaction)
				return ENotificationColor.NEUTRAL;
			
			int factionIndex = factionManager.GetFactionIndex(notificationPlayerFaction);
			
			//~ Return minus number to let system know to use it to get faction color
			return (factionIndex +1) * -1;
		}
		
		if (!GetGame().GetPlayerController()) 
			return ENotificationColor.NEUTRAL;
		
		//Get factions using ID
		Faction playerSelfFaction = factionManager.GetLocalPlayerFaction();
		
		if (!playerSelfFaction || !notificationPlayerFaction) 
			return ENotificationColor.NEUTRAL;

		//Check if friendly
		if (playerSelfFaction.IsFactionFriendly(notificationPlayerFaction))
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEUTRAL;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.POSITIVE;
		}
		else
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.NEUTRAL;
		}
		
		//No color set
		return ENotificationColor.NEUTRAL;
	}
	
	//------------------------------------------------------------------------------------------------
	//Get faction related color of entity target
	protected ENotificationColor GetFactionRelatedColorEntity(int notificationEntityID, ENotificationColor colorType)
	{	
		//If not a faction set color keep current color
		if (colorType < ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
			return colorType;
		
		if (!GetGame().GetPlayerController()) return ENotificationColor.NEUTRAL;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager) 
			return ENotificationColor.NEUTRAL;
		
		//Get notification entity
		SCR_EditableEntityComponent notificationEntity = SCR_EditableEntityComponent.Cast(Replication.FindItem(notificationEntityID));
		if (!notificationEntity) 
			return ENotificationColor.NEUTRAL;
		
		//Get notification entity ChimeraCharacter
		SCR_ChimeraCharacter notificationEntityChimera = SCR_ChimeraCharacter.Cast(notificationEntity.GetOwner());
		if (!notificationEntityChimera) 
			return ENotificationColor.NEUTRAL;
		
		Faction notificationEntityFaction = notificationEntityChimera.GetFaction();
		
		//~ Notification color is equal to faction color
		if (colorType == ENotificationColor.FACTION_COLOR)
		{
			if (!notificationEntityFaction)
				return ENotificationColor.NEUTRAL;
			
			int factionIndex = factionManager.GetFactionIndex(notificationEntityFaction);
			
			//~ Return minus number to let system know to use it to get faction color
			return (factionIndex +1) * -1;
		}
		
		//Get factions
		Faction playerSelfFaction = factionManager.GetLocalPlayerFaction();
		
		if (!playerSelfFaction || !notificationEntityFaction) 
			return ENotificationColor.NEUTRAL;
		
		//Check if friendly
		if (playerSelfFaction.IsFactionFriendly(notificationEntityFaction))
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEUTRAL;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.POSITIVE;
		}
		else
		{
			if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_NEGATIVE)
				return ENotificationColor.POSITIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_ENEMY_IS_NEGATIVE_ONLY)
				return ENotificationColor.NEGATIVE;
			else if (colorType == ENotificationColor.FACTION_FRIENDLY_IS_POSITIVE_ONLY)
				return ENotificationColor.NEUTRAL;
		}
		
		//No color set
		return ENotificationColor.NEUTRAL;
	}	
	
	//------------------------------------------------------------------------------------------------
	//Check if entities are friendly
	protected bool AreEntitiesFriendly(int entity1ID, bool entity1IsPlayer, int entity2ID, bool entity2IsPlayer)
	{
		Faction faction1, faction2;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (entity1IsPlayer || entity2IsPlayer)
		{
			if (!factionManager)
				return false;
		}
		
		SCR_EditableEntityComponent entity;
		SCR_ChimeraCharacter entityChimera;
		
		//Get notification entity
		
		//Get entity 1 faction
		if (entity1IsPlayer)
		{
			faction1 = factionManager.GetPlayerFaction(entity1ID);
		}
		else 
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entity1ID));
			if (!entity) 
				return false;
		
			//Get notification entity SCR_ChimeraCharacter
			entityChimera = SCR_ChimeraCharacter.Cast(entity.GetOwner());
			if (!entityChimera) 
				return false;
			
			faction1 = entityChimera.GetFaction();
		}
		
		//Get entity 2 faction
		if (entity2IsPlayer)
		{
			faction2 = factionManager.GetPlayerFaction(entity2ID);
		}
		else 
		{
			entity = SCR_EditableEntityComponent.Cast(Replication.FindItem(entity2ID));
			if (!entity) 
				return false;
		
			//Get notification entity SCR_ChimeraCharacter
			entityChimera = SCR_ChimeraCharacter.Cast(entity.GetOwner());
			if (!entityChimera) 
				return false;
			
			faction2 = entityChimera.GetFaction();
		}
		
		if (!faction1 || !faction2)
			return false;
		
		return faction1.IsFactionFriendly(faction2);
	}
};