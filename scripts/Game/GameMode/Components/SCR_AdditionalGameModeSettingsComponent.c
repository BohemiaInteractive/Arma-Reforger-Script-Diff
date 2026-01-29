[ComponentEditorProps(category: "GameScripted/GameMode", description: "")]
class SCR_AdditionalGameModeSettingsComponentClass : SCR_BaseGameModeComponentClass
{
}

enum SCR_ENametagRelationFilter
{
	SHOW_HOSTILE = 1 << 0, 							//!< Show nametags of hostile entities
	SHOW_SAME_FACTION = 1 << 1, 					//!< Show nametags of members of own faction
	SHOW_MILITARY_ALLIES = 1 << 2, 					//!< show nametags of military allies of a different faction
	SHOW_CIVILIAN_ALLIES = 1 << 3, 					//!< show nametags of non-military allies of a different faction
	SHOW_GROUP_MEMBERS = 1 << 4, 					//!< Show nametags of members of own group
}

enum SCR_ENametagControllerFilter
{
	SHOW_PLAYERS = 1 << 0, 							//!< Show nametags of players
	SHOW_AI = 1 << 1, 								//!< Show nametags of AI
}

class SCR_AdditionalGameModeSettingsComponent : SCR_BaseGameModeComponent
{
	protected static SCR_AdditionalGameModeSettingsComponent s_Instance;
	
	[Attribute("1", desc: "If true team kill punishment is enabled, otherwise it is disabled", category: "Teamkilling"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bEnableTeamKillPunishment;
	
	[Attribute("1", desc: "If true allows for vehicles (and potential other entities) to be refunded at depods.", category: "Entity Refund"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bAllowEntityRefundingAction;
	
	[Attribute("#AR-Editor_Action_Disabled_By_GM", desc: "Shown on the entity action why refunding is disabled. The action is simply hidden if this is empty.", category: "Entity Refund")]
	protected LocalizedString m_sEntityRefundingDisabledReason;

	[Attribute("1", desc: "If true it will show additional information in form of ui element that shows ballistic data (f.e. in mortars).", category: "Entity Refund"), RplProp(onRplName: "OnAdditionalSettingsChanged")]
	protected bool m_bProjectileBallisticInfoVisibility;
	
	[Attribute("1", desc: "If true, vehicle (wreck) salvage is enabled, otherwise it is disabled", category: "Vehicle Salvage"), RplProp()]
	protected bool m_bEnableVehicleSalvage;
	
	[Attribute((SCR_ENametagRelationFilter.SHOW_SAME_FACTION + SCR_ENametagRelationFilter.SHOW_GROUP_MEMBERS + SCR_ENametagRelationFilter.SHOW_MILITARY_ALLIES + SCR_ENametagRelationFilter.SHOW_CIVILIAN_ALLIES).ToString(), desc: "", category: "Nametag Filters", uiwidget: UIWidgets.Flags, enumType: SCR_ENametagRelationFilter), RplProp(onRplName: "OnNametagRelationFilterUpdated")]
	protected SCR_ENametagRelationFilter m_eNametagRelationFilters;
	
	[Attribute((SCR_ENametagControllerFilter.SHOW_AI + SCR_ENametagControllerFilter.SHOW_PLAYERS).ToString(), desc: "", category: "Nametag Filters", uiwidget: UIWidgets.Flags, enumType: SCR_ENametagControllerFilter), RplProp(onRplName: "OnNametagControllerFilterUpdated")]
	protected SCR_ENametagControllerFilter m_eNametagControllerFilters;

	[Attribute(desc: "If true artillery ai command will get position offset based on the distance from the player to the position at which player ordered the artillery")]
	protected bool m_bAdditionalArtilleryOrderDistancePenalty;

	[RplProp(onRplName: "OnNightNoiseSettingChanged"), Attribute(desc: "Disable noise effect which will be visible in dark places")]
	protected bool m_bDisableNightNoiseEffect = true;
	
	protected ref ScriptInvokerInt m_OnNametagRelationFilterUpdated;
	protected ref ScriptInvokerInt m_OnNametagControllerFilterUpdated;
	

	protected ref ScriptInvokerVoid m_OnChangeAdditionalSettingsInvoker;

	//------------------------------------------------------------------------------------------------
	//! Gets own instance
	//! \return Additional Game Mode Settings Component Instance
	static SCR_AdditionalGameModeSettingsComponent GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetProjectileBallisticInfoVisibility()
	{
		return m_bProjectileBallisticInfoVisibility;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnChangeAdditionalSettingsInvoker()
	{
		if (!m_OnChangeAdditionalSettingsInvoker)
			m_OnChangeAdditionalSettingsInvoker = new ScriptInvokerVoid();

		return m_OnChangeAdditionalSettingsInvoker;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAdditionalSettingsChanged()
	{
		if (!m_OnChangeAdditionalSettingsInvoker)
			return;

		m_OnChangeAdditionalSettingsInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	//! On Updated function for relation filters
	void OnNametagRelationFilterUpdated()
	{
		if (m_OnNametagRelationFilterUpdated)
			m_OnNametagRelationFilterUpdated.Invoke(m_eNametagRelationFilters);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets nametag relation filter invoker
	//! \return Invoker for when nametag relation filter is updated
	ScriptInvokerInt GetOnNametagRelationFilterUpdated()
	{
		if (!m_OnNametagRelationFilterUpdated)
			m_OnNametagRelationFilterUpdated = new ScriptInvokerInt();
		
		return m_OnNametagRelationFilterUpdated;
	}
		
	//------------------------------------------------------------------------------------------------
	//! On Updated function for controller filters
	void OnNametagControllerFilterUpdated()
	{
		if (m_OnNametagControllerFilterUpdated)
			m_OnNametagControllerFilterUpdated.Invoke(m_eNametagControllerFilters);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets nametag controller filter invoker
	//! \return Invoker for when nametag controller filter is updated
	ScriptInvokerInt GetOnNametagControllerFilterUpdated()
	{
		if (!m_OnNametagControllerFilterUpdated)
			m_OnNametagControllerFilterUpdated = new ScriptInvokerInt();
		
		return m_OnNametagControllerFilterUpdated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks for given relation filter
	//! \param[in] filter The filter that it checks for
	bool HasNametagRelationFilters(SCR_ENametagRelationFilter filter)
	{
		return SCR_Enum.HasFlag(m_eNametagRelationFilters, filter);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Current nametag relation filters
	SCR_ENametagRelationFilter GetNametagRelationFilters()
	{
		return m_eNametagRelationFilters;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets relation filter & handles replication
	//! \param[in] filters Relation filters to set
	//! \param[in] playerId The ID of the player
	void SetNametagRelationFilters(SCR_ENametagRelationFilter filters, int playerId = -1)
	{
		if (!GetGameMode().IsMaster())
			return;
		
		if (m_eNametagRelationFilters == filters)
			return;
		
		m_eNametagRelationFilters = filters;
				
		OnNametagRelationFilterUpdated();
		Replication.BumpMe();
		
		if (playerId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_NAMETAG_RELATION_FILTER_CHANGED, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks for given controller filter
	//! \param[in] filter The filter that it checks for
	//! \return true or false based on if it has the filter
	bool HasNametagControllerFilter(SCR_ENametagControllerFilter filter)
	{
		return SCR_Enum.HasFlag(m_eNametagControllerFilters, filter);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Current nametag controller filters
	SCR_ENametagControllerFilter GetNametagControllerFilters()
	{
		return m_eNametagControllerFilters;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets controller filter & handles replication
	//! \param[in] filters Controller filters to set 
	//! \param[in] playerId The ID of the player
	void SetNametagControllerFilters(SCR_ENametagControllerFilter filters, int playerId = -1)
	{
		if (!GetGameMode().IsMaster())
			return;
		
		if (m_eNametagControllerFilters == filters)
			return;
		
		m_eNametagControllerFilters = filters;
		
		OnNametagControllerFilterUpdated();
		Replication.BumpMe();
		
		if (playerId > 0)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_NAMETAG_CONTROLLER_FILTER_CHANGED, playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! State of the scenario setting that dictates if game should add an additional offset to the position of the artillery command issued by the player
	//! \return true if position of the artillery command issued by the player should be spoofed with random offset
	bool IsAdditionalArtileryOrderDistancePenaltyEnabled()
	{
		return m_bAdditionalArtilleryOrderDistancePenalty;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets team kill punishment is enabled or disabled and notifies players if playerID is provided. (Server only)
	//! \param[in] enablePunishment Enable punishment setting for team kills, toggles team kill punishment on or off for all players
	//! \param[in] playerID Player ID is an optional parameter representing the ID of the GM changing the setting. Will send notification if changed
	void SetEnableTeamKillPunishment_S(bool enablePunishment, int playerID = -1)
	{
		if (m_bEnableTeamKillPunishment == enablePunishment || !GetGameMode().IsMaster())
			return;
		
		m_bEnableTeamKillPunishment = enablePunishment;
		
		if (playerID > 0)
		{
			if (m_bEnableTeamKillPunishment)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_TEAMKILL_PUNISHMENT_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_TEAMKILL_PUNISHMENT_DISABLED, playerID);
		}
			
		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return whether team killing punishment is enabled.
	bool IsTeamKillingPunished()
	{
		return m_bEnableTeamKillPunishment;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set if players are allowed to refund entities or not. Note currently only vehicles can be refunded. (Server only)
	//! \param[in] allowRefunding Set if vehicles are allowed to be refunded
	//! \param[in] playerID Optional, will send a notification when a player (GM) changes this setting. No notifications are send if playerID is -1
	void SetAllowEntityRefundingAction_S(bool allowRefunding, int playerID = -1)
	{
		if (m_bAllowEntityRefundingAction == allowRefunding || !GetGameMode().IsMaster())
			return;
		
		m_bAllowEntityRefundingAction = allowRefunding;
		
		if (playerID > 0)
		{
			if (m_bAllowEntityRefundingAction)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_REFUND_ENTITY_AT_DEPOTS_ENABLED, playerID);
			else 
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_REFUND_ENTITY_AT_DEPOTS_DISABLED, playerID);
		}
			
		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return If players are allowed to refund entities
	bool IsEntityRefundingActionAllowed()
	{
		return m_bAllowEntityRefundingAction;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Disable reason for action when the player cannot refund. If empty will simply not show the action
	string GetEntityRefundingDisabledReason()
	{
		return m_sEntityRefundingDisabledReason;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if game should show ui elements that provide projectile ballistic data.
	//! \param[in] shouldShow Set if such ui elements should be visible
	//! \param[in] playerID Optional, will send a notification when a player (GM) changes this setting. No notifications are send if playerID is -1
	void SetProjectileBallisticInfoVisibility_S(bool shouldShow, int playerID = -1)
	{
		if (m_bProjectileBallisticInfoVisibility == shouldShow || !GetGameMode().IsMaster())
			return;
		
		m_bProjectileBallisticInfoVisibility = shouldShow;

		OnAdditionalSettingsChanged();
		Replication.BumpMe();

		if (playerID <= 0)
			return;

		if (m_bProjectileBallisticInfoVisibility)
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_MORTAR_BALLISTIC_DATA_VISIBILITY_ENABLED, playerID);
		else
			SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_MORTAR_BALLISTIC_DATA_VISIBILITY_DISABLED, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if vehicle salvage is enabled
	//! \return whether vehicle salvage is enabled
	bool IsVehicleSalvageEnabled()
	{
		return m_bEnableVehicleSalvage;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables or disables vehicle salvage
	//! \param enableSetting Saves the setting's state
	//! \param playerID The ID of the player
	void SetEnableVehicleSalvage_S(bool enableSetting, int playerID = -1)
	{
		if (m_bEnableVehicleSalvage == enableSetting || !GetGameMode().IsMaster())
			return;
		
		m_bEnableVehicleSalvage = enableSetting;
		
		if (playerID > 0)
		{
			if (m_bEnableVehicleSalvage)
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_VEHICLE_SALVAGE_ENABLED, playerID);
			else
				SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_VEHICLE_SALVAGE_DISABLED, playerID);
		}
		
		OnAdditionalSettingsChanged();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] disabled set to true will disable night grain effect for the host and all clients
	void SetNightNoiseEffectState_S(bool disabled)
	{
		m_bDisableNightNoiseEffect = disabled;
		OnNightNoiseSettingChanged();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNightNoiseSettingChanged()
	{
		SCR_NoiseFilterEffect.SetNightNoiseEffectState(m_bDisableNightNoiseEffect);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_AdditionalGameModeSettingsComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (s_Instance)
		{
			Print("'SCR_AdditionalGameModeSettingsComponent' exists twice in the world!", LogLevel.WARNING);
			return;
		}
			
		s_Instance = this;
	}
	
}
