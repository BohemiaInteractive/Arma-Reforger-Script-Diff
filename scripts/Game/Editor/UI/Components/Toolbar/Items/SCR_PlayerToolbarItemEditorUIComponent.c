class SCR_PlayerToolbarItemEditorUIComponent : SCR_EntityToolbarItemEditorUIComponent
{
	[Attribute()]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute()]
	protected string m_sPlayerPlatformIconName;

	protected TextWidget m_wPlayerName;
	protected ImageWidget m_wPlatformIconWidget;
	
	//------------------------------------------------------------------------------------------------
	override void SetEntity(SCR_EditableEntityComponent entity, Widget widget, SCR_EditableEntitySlotManagerUIComponent slotManager)
	{
		super.SetEntity(entity, widget, slotManager);
		
		SCR_EditablePlayerDelegateComponent delegate = SCR_EditablePlayerDelegateComponent.Cast(entity);
		if (!delegate)
			return;
				
		int playerID = delegate.GetPlayerID();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		m_wPlayerName = TextWidget.Cast(widget.FindAnyWidget(m_sPlayerNameWidgetName));
		if (m_wPlayerName)
			m_wPlayerName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));

		m_wPlatformIconWidget = ImageWidget.Cast(widget.FindAnyWidget(m_sPlayerPlatformIconName));
		if (m_wPlatformIconWidget)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(playerManager.GetPlatformKind(playerID), m_wPlatformIconWidget, showOnPC: true, showOnXbox: true);
	}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		if (m_wPlayerName)
			m_wPlayerName.SetColor(Color.FromInt(UIColors.HIGHLIGHTED.PackToInt()));
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		if (m_wPlayerName)
			m_wPlayerName.SetColor(Color.FromInt(Color.WHITE));
		
		return false;
	}
}
