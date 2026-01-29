class SCR_FactionCommanderOpenMapUserAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "Open with specific map config", "", ParamEnumArray.FromEnum(EMapEntityMode))]
	protected EMapEntityMode m_eOpenWithMapEntityMode;

	[Attribute("0", uiwidget: UIWidgets.CheckBox, "Always show action", "")]
	protected bool m_bAlwaysShow;

	protected const string CANNOT_OPEN_MAP_TEXT = "#AR-FactionCommander_UserAction_CannotPerform_OpenMap";

	protected SCR_FactionCommanderBaseLogicComponent m_FactionCommanderBaseLogic;
	protected SCR_CommandPostMapMenuUI m_CommandPostMapMenu;
	protected SCR_CampaignMobileAssemblyComponent m_MobileAssemblyComponent;

	protected bool m_bIsCommanderRoleEnabled;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_GameModeCampaign campaignGameMode = SCR_GameModeCampaign.Cast(gameMode);
		if (campaignGameMode)
			m_bIsCommanderRoleEnabled = campaignGameMode.GetCommanderRoleEnabled();
		else
			m_bIsCommanderRoleEnabled = true;

		if (!SCR_FactionCommanderHandlerComponent.GetInstance())
			return;

		m_FactionCommanderBaseLogic = SCR_FactionCommanderBaseLogicComponent.Cast(pOwnerEntity.FindComponent(SCR_FactionCommanderBaseLogicComponent));
		m_MobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(pOwnerEntity.FindComponent(SCR_CampaignMobileAssemblyComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (!m_FactionCommanderBaseLogic && !m_MobileAssemblyComponent || !m_bIsCommanderRoleEnabled)
			return false;

		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!playerFaction)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);

		if (m_FactionCommanderBaseLogic && !m_FactionCommanderBaseLogic.IsControlledByFaction(playerFaction))
			return false;

		if (m_MobileAssemblyComponent && m_MobileAssemblyComponent.GetParentFaction() != playerFaction)
			return false;

		if (m_bAlwaysShow)
		{
			SetCannotPerformReason(CANNOT_OPEN_MAP_TEXT);
			return true;
		}
		else if (!playerFaction.IsPlayerCommander(playerId))
		{
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_FactionCommanderBaseLogic && !m_MobileAssemblyComponent)
			return false;

		SCR_CampaignFaction playerFaction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!playerFaction)
			return false;

		if (m_FactionCommanderBaseLogic && !m_FactionCommanderBaseLogic.IsControlledByFaction(playerFaction))
			return false;

		if (m_MobileAssemblyComponent && m_MobileAssemblyComponent.GetParentFaction() != playerFaction)
			return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);

		if (!playerFaction.IsPlayerCommander(playerId))
		{
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
		if (!configComp)
			return;

		configComp.SetCommandPostMapEntityMode(m_eOpenWithMapEntityMode);
		OpenCommandPostMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenCommandPostMenu()
	{
		MenuManager menuManager = g_Game.GetMenuManager();
		m_CommandPostMapMenu = SCR_CommandPostMapMenuUI.Cast(menuManager.OpenMenu(ChimeraMenuPreset.CommandPostMapMenu));
		m_CommandPostMapMenu.GetOnClosedByEditor().Insert(OnClosedByEditor);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClosedByEditor()
	{
		// the map was closed by the editor, it is going to be opened again
		GetGame().GetCallqueue().Call(OpenCommandPostMenu);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
