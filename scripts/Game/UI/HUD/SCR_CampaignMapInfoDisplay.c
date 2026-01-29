class SCR_CampaignMapInfoDisplay : SCR_CampaignInfoDisplay
{
	protected bool m_bMapOpen;
	protected bool m_bCommanderRoleEnabled;
	protected bool m_bIsPlayerCommander;

	protected Widget m_wCommanderNameWrapper;
	protected ImageWidget m_wPlatformIcon;
	protected RichTextWidget m_wCommanderName;
	protected Widget m_wPlatformScale;

	protected MapConfiguration m_MapConfig;

	protected BaseContainer m_GameplaySettings;

	protected const string CURRENT_COMMANDER_AI_TEXT = "#AR-FactionCommander_CurrentCommander_AI";
	protected const string CURRENT_COMMANDER_PLAYER_TEXT = "#AR-FactionCommander_CurrentCommander";

	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		super.DisplayStartDrawInit(owner);

		m_GameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");

		SCR_FactionCommanderHandlerComponent comp = SCR_FactionCommanderHandlerComponent.GetInstance();

		if (!comp)
			return true;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (campaign)
			m_bCommanderRoleEnabled = campaign.GetCommanderRoleEnabled();

		comp.GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		super.DisplayStartDraw(owner);

		m_wCommanderNameWrapper = m_wRoot.FindAnyWidget("CommanderNameWrapper");
		m_wCommanderName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CurrentCommanderPlayers"));
		m_wPlatformScale = m_wRoot.FindAnyWidget("ScalePlatform");
		m_wPlatformIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget("PlatformIcon"));

		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		super.DisplayStopDraw(owner);

		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		SCR_FactionCommanderHandlerComponent commanderHandler = SCR_FactionCommanderHandlerComponent.GetInstance();
		if (!commanderHandler)
			return;

		commanderHandler.GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateHUD()
	{
		m_bPeriodicRefresh = false;

		if (!m_wRoot || !m_bInitDone)
			return;

		if (m_Campaign.IsTutorial() || m_Campaign.GetIsMatchOver())
		{
			Show(false);
			return;
		}

		if (m_bMapOpen)
			Show(true);

		UpdateHUDValues();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateHUDValues()
	{
		super.UpdateHUDValues();
		string name;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!faction)
			return;

		int commanderId = faction.GetCommanderId();

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		if (!m_wCommanderNameWrapper || !m_wCommanderName)
			return;

		m_bIsPlayerCommander = playerController.GetLocalPlayerId() == commanderId;

		bool isSupportStationMapMode;

		if (m_MapConfig)
		{
			EMapEntityMode currentMapEntityMode = m_MapConfig.MapEntityMode;
			isSupportStationMapMode = currentMapEntityMode == EMapEntityMode.LOGISTICS_STATION || 
			currentMapEntityMode == EMapEntityMode.OPERATIONS_STATION || 
			currentMapEntityMode == EMapEntityMode.COMBAT_SUPPORT_STATION;
		}

		if (!m_bCommanderRoleEnabled || isSupportStationMapMode)
		{
			m_wCommanderNameWrapper.SetVisible(false);
			return;
		}
		
		m_wCommanderNameWrapper.SetVisible(true);

		if (!faction.IsAICommander())
			name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(commanderId);
		else
			name = CURRENT_COMMANDER_AI_TEXT;

		m_wCommanderName.SetTextFormat(CURRENT_COMMANDER_PLAYER_TEXT, name);

		SetPlatformImage(!faction.IsAICommander(), commanderId, playerController);
	}

	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_MapConfig = config;

		if (m_Campaign.IsTutorial())
			return;

		m_bMapOpen = true;

		if (SCR_DeployMenuMain.GetDeployMenu() == null)
		{
			Show(true);
			UpdateHUD();
		}
		else
		{
			Show(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_MapConfig = null;

		if (m_Campaign.IsTutorial())
			return;

		m_bMapOpen = false;
		Show(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPlatformImage(bool setVisible, int commanderId, notnull SCR_PlayerController playerController)
	{
		if (!m_wPlatformIcon || !m_wPlatformScale)
			return;

		if (!setVisible)
		{
			m_wPlatformScale.SetVisible(false);
			return;
		}

		bool showOnPC;
		if (m_GameplaySettings)
			m_GameplaySettings.Get("m_bPlatformIconNametag", showOnPC);

		bool res = playerController.SetPlatformImageTo(commanderId, m_wPlatformIcon, showOnPC: showOnPC);
		m_wPlatformScale.SetVisible(res);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		if (faction && faction == SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction()))
			UpdateHUDValues();
	}
}
