
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

		const int authorId = entity.GetAuthorPlayerID();
		if (authorId <= 0) //SCR_EditableEntityComponent.GetAuthorPlayerID can return 0 when there is no author
			return false;

		const string name = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(authorId);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(name))
			return false;

		text.SetText(name);

		const PlatformKind authorPlatform = entity.GetAuthorPlatform();
		ImageWidget platformIcon = ImageWidget.Cast(widget.FindAnyWidget(PLATFORM_ICON_WIDGET_NAME));
		if (platformIcon && authorPlatform != PlatformKind.NONE)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(authorPlatform, platformIcon, showOnPC: true, showOnXbox: true);
		}

		return true;
	}

	//---- REFACTOR NOTE END ----
}
