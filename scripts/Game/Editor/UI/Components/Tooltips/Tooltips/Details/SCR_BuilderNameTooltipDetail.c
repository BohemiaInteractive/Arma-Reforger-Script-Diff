
[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_BuilderNameTooltipDetail : SCR_EntityTooltipDetail
{
	protected const string AUTHOR_TEXT_WIDGET_NAME = "Text";
	protected const string PLATFORM_ICON_WIDGET_NAME = "PlatformIcon";

	//---- REFACTOR NOTE START: Searching for "Text" dirrectly 
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		TextWidget text = TextWidget.Cast(widget.FindAnyWidget(AUTHOR_TEXT_WIDGET_NAME));
		if (!text)
			return false;
		
		BaseGameMode gameMode = GetGame().GetGameMode();	
		if (!gameMode)
			return false;

		int playerId;
		SCR_CampaignBuildingCompositionComponent compositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(entity.GetOwner().FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (compositionComponent)
			playerId = compositionComponent.GetBuilderId();

		if (playerId == 0)//SCR_CampaignBuildingCompositionComponent.INVALID_PLAYER_ID = 0
			playerId = entity.GetAuthorPlayerID();

		if (playerId <= 0)//SCR_EditableEntityComponent.GetAuthorPlayerID can return 0 when there is no author
			return false;

		PlatformKind playerPlatformKind;
		string name = GetPlayerName(playerId, playerPlatformKind);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(name))
			return false;
		
		text.SetText(name);

		ImageWidget platformIcon = ImageWidget.Cast(widget.FindAnyWidget(PLATFORM_ICON_WIDGET_NAME));
		if (platformIcon && playerPlatformKind != PlatformKind.NONE)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(playerPlatformKind, platformIcon, showOnPC: true, showOnXbox: true);
		}

		return true;
	}
	
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected string GetPlayerName(int playerID, out PlatformKind playerPlatformKind = PlatformKind.NONE)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager)
			playerPlatformKind = playerManager.GetPlatformKind(playerID);

		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
	}
}
