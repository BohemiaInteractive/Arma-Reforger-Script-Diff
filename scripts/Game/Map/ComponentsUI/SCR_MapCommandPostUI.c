class SCR_MapCommandPostUI : ScriptedWidgetComponent
{
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourceNamePicker, "Imageset Wrapper UI with mode icons", "imageset", category: "Frontend")]
	ResourceName m_sWrapperImageset;

	protected ButtonWidget m_wCloseButtonWidget;
	protected SCR_InputButtonComponent m_InputButton;

	protected bool m_bCommanderRoleEnabled;

	protected string m_sModeName;
	protected string m_sIconName;
	protected Color m_ModeColor;

	protected RichTextWidget m_wCommanderName;
	protected Widget m_wPlatformLayout;
	protected ImageWidget m_wPlatformIcon;
	protected TextWidget m_wModeText;
	protected ImageWidget m_wModeTopBar;
	protected ImageWidget m_wModeIcon;

	protected BaseContainer m_GameplaySettings;

	protected const string CURRENT_COMMANDER_AI_TEXT = "#AR-FactionCommander_CurrentCommander_AI";
	protected const string CURRENT_COMMANDER_PLAYER_TEXT = "#AR-FactionCommander_CurrentCommander";
	protected const string LOGISTICS_LABEL_TEXT = "#AR-FactionCommander_UserAction_LogisticsStation";
	protected const string COMBAT_SUPPORT_LABEL_TEXT = "#AR-FactionCommander_UserAction_RadioStation";
	protected const string OPERATIONS_LABEL_TEXT = "#AR-FactionCommander_UserAction_EngineeringStation";
	protected const string LOGISTICS_ICON = "veh_trunk";
	protected const string COMBAT_SUPPORT_ICON = "characters";
	protected const string OPERATIONS_ICON = "scenarios";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_GameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");

		m_wCloseButtonWidget = ButtonWidget.Cast(w.FindAnyWidget("Toolbar_PlayButton_CloseMap"));
		if (!m_wCloseButtonWidget)
			return;

		m_InputButton = SCR_InputButtonComponent.Cast(SCR_WidgetTools.FindHandlerInChildren(m_wCloseButtonWidget, SCR_InputButtonComponent));
		if (!m_InputButton)
			return;

		m_InputButton.m_OnClicked.Insert(OnCloseButtonClicked);

		SCR_CampaignLogisticMapUIBase.GetOnBaseSelected().Insert(OnBaseClicked);

		if (SCR_FactionCommanderHandlerComponent.GetInstance())
			SCR_FactionCommanderHandlerComponent.GetInstance().GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);

		SetCommanderNameWidget(w);
		SetModeType(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		SCR_CampaignLogisticMapUIBase.GetOnBaseSelected().Remove(OnBaseClicked);

		if (m_InputButton)
			m_InputButton.m_OnClicked.Remove(OnCloseButtonClicked);

		if (SCR_FactionCommanderHandlerComponent.GetInstance())
			SCR_FactionCommanderHandlerComponent.GetInstance().GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCloseButtonClicked()
	{
		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseClicked(bool selected)
	{
		if (!m_wCloseButtonWidget || !m_InputButton)
			return;

		bool isEnabled = !selected;
		m_wCloseButtonWidget.SetEnabled(isEnabled);
		m_InputButton.SetEnabled(isEnabled);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes the Command Post Menu when local player loses commander role
	protected void OnFactionCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		int playerId = SCR_PlayerController.GetLocalPlayerId();
		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_PlayerController.GetLocalMainEntityFaction());

		// Skip closing if another faction commander changed or local player is a new commander
		if (!playerFaction || playerFaction != faction || commanderPlayerId == playerId)
			return;

		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		menuManager.CloseMenuByPreset(ChimeraMenuPreset.CommandPostMapMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCommanderNameWidget(Widget w)
	{
		m_wCommanderName = RichTextWidget.Cast(w.FindAnyWidget("CurrentCommander"));
		if (!m_wCommanderName)
			return;

		m_wPlatformLayout = w.FindAnyWidget("PlatformLayout");
		if (!m_wPlatformLayout)
			return;

		m_wPlatformIcon = ImageWidget.Cast(w.FindAnyWidget("PlatformIcon"));
		if (!m_wPlatformIcon)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!faction)
			return;

		int commanderId = faction.GetCommanderId();
		if (!commanderId)
			return;

		string name = CURRENT_COMMANDER_AI_TEXT;

		if (!faction.IsAICommander())
			name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(commanderId);

		m_wCommanderName.SetTextFormat(CURRENT_COMMANDER_PLAYER_TEXT, name);

		bool showOnPC;
		if (m_GameplaySettings)
			m_GameplaySettings.Get("m_bPlatformIconNametag", showOnPC);

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
		{
			bool res = playerController.SetPlatformImageTo(commanderId, m_wPlatformIcon, showOnPC: showOnPC);
			m_wPlatformLayout.SetVisible(res);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetModeType(Widget w)
	{
		m_wModeText = TextWidget.Cast(w.FindAnyWidget("ModeText"));
		m_wModeTopBar = ImageWidget.Cast(w.FindAnyWidget("TopBadgeLine"));
		m_wModeIcon = ImageWidget.Cast(w.FindAnyWidget("ModeIcon"));

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;
		
		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;

		EMapEntityMode currentMapEntityMode = configComp.GetCommandPostMapEntityMode();

		switch (currentMapEntityMode)
		{
			case EMapEntityMode.LOGISTICS_STATION:
			{
				m_sModeName = LOGISTICS_LABEL_TEXT;
				m_sIconName = LOGISTICS_ICON;
				m_ModeColor = UIColors.EDITOR_MODE_GM_MODE;
				break;
			}
			case  EMapEntityMode.COMBAT_SUPPORT_STATION:
			{
				m_sModeName = COMBAT_SUPPORT_LABEL_TEXT;
				m_sIconName = COMBAT_SUPPORT_ICON;
				m_ModeColor = UIColors.EDITOR_MODE_ADMIN_MODE;
				break;
			}
			case  EMapEntityMode.OPERATIONS_STATION:
			{
				m_sModeName = OPERATIONS_LABEL_TEXT;
				m_sIconName = OPERATIONS_ICON;
				m_ModeColor = UIColors.EDITOR_MODE_ARMAVISION;
				break;
			}
		}

		m_wModeText.SetText(m_sModeName);
		m_wModeTopBar.SetColor(m_ModeColor);
		m_wModeIcon.SetColor(m_ModeColor);
		m_wModeIcon.LoadImageFromSet(0, m_sWrapperImageset, m_sIconName);
	}
}
