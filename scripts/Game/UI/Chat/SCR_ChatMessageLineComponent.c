class SCR_ChatMessageLineComponent : ScriptedWidgetComponent
{
	protected ref SCR_ChatMessageLineWidgets m_Widgets = new SCR_ChatMessageLineWidgets();
	
	protected ResourceName CHAT_IMAGESET = "{1872FFA1133724A2}UI/Textures/Chat/chat.imageset";
	
	protected Widget m_wRoot;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		SetEmptyMessage();
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves a copy of the color that should be used for the text of the message
	//! \param[in] style
	//! \param[in] senderFaction
	//! \param[in] senderId
	protected Color GetTextColor(notnull SCR_ChatMessageStyle style, SCR_Faction senderFaction = null, int senderId = -1)
	{
		if (senderId < 0)
			return Color.FromInt(Color.WHITE);

		if (style.m_bColoredPlayerName)
			return Color.FromInt(style.m_Color.PackToInt());

		return Color.FromInt(Color.WHITE);
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves a copy of the color that should be used for the badge of the message
	//! \param[in] style
	//! \param[in] senderFaction
	//! \param[in] localFaction
	//! \param[in] senderId
	protected Color GetBadgeColor(notnull SCR_ChatMessageStyle style, SCR_Faction senderFaction = null, SCR_Faction localFaction = null, int senderId = -1)
	{
		if (style.m_bColoredBadge)
			return style.m_Color;

		if (!style.m_bColorBadgeWithRelationColor || !senderFaction)
			return Color.FromInt(Color.WHITE);

		if (senderFaction == localFaction && senderFaction.IsPlayerCommander(senderId))
			return Color.FromInt(style.m_iBadgeColorForCommander);

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return Color.FromInt(senderFaction.GetFactionColor().PackToInt());

		SCR_AIGroup senderGroup = groupsManager.GetPlayerGroup(senderId);
		if (!senderGroup)
			return Color.FromInt(senderFaction.GetFactionColor().PackToInt());

		SCR_AIGroup localGroup = groupsManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (localGroup == senderGroup)
			return senderFaction.GetOutlineFactionColor();

		return Color.FromInt(senderFaction.GetFactionColor().PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves a copy of the color that should be used for the background of the message
	//! \param[in] style
	//! \param[in] senderFaction
	//! \param[in] localFaction
	//! \param[in] senderId
	protected Color GetBackgroundColor(notnull SCR_ChatMessageStyle style, SCR_Faction senderFaction = null, SCR_Faction localFaction = null, int senderId = -1)
	{
		if (style.m_bColoredBackground)
			return style.m_Color;

		if (style.m_bColorCommanderBackground && senderFaction && senderFaction == localFaction && senderFaction.IsPlayerCommander(senderId))
			return Color.FromInt(style.m_iCommanderBackgroundColor);

		return Color.FromInt(Color.BLACK);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] msg
	//! \param[in] style
	void SetMessage(notnull SCR_ChatMessage msg, notnull SCR_ChatMessageStyle style)
	{		
		// Enable elements which can be disabled for empty message lines
		m_Widgets.m_wBadge.SetVisible(true);
		m_Widgets.m_wTypeImage.SetVisible(true);
		
		Color channelColor = style.m_Color;
		string chatTypeImageName = style.m_sIconName;
		
		//---------------------------------------------------------------------------------
		// Common properties derived from the resolved style, regardless of message type

		SCR_Faction senderFaction;
		SCR_Faction localFaction;
		int senderId = -1;
		SCR_ChatMessageGeneral messageGeneral = SCR_ChatMessageGeneral.Cast(msg);
		if (messageGeneral && messageGeneral.m_iSenderId > 0 && !messageGeneral.m_sSenderName.IsEmpty())
		{
			senderFaction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(messageGeneral.m_iSenderId));
			localFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			senderId = messageGeneral.m_iSenderId;
		}

		Color textColor = GetTextColor(style, senderFaction, senderId);
		Color badgeColor = GetBadgeColor(style, senderFaction, localFaction, senderId);
		Color backgroundColor = GetBackgroundColor(style, senderFaction, localFaction, senderId);

		// Badge color
		m_Widgets.m_wBadge.SetColor(badgeColor);
		
		// Image color
		Color imageColor = Color.FromInt(Color.WHITE);
		if (style.m_bColoredIcon)
			imageColor = channelColor;

		m_Widgets.m_wTypeImage.SetColor(imageColor);
		
		// Background color
		float bgAlphaOld = m_Widgets.m_wBackgroundImage.GetColor().A();
		backgroundColor.SetA(bgAlphaOld); // Keep the old alpha value
		m_Widgets.m_wBackgroundImage.SetColor(backgroundColor);
		m_Widgets.m_wBackgroundImage.SetVisible(true);
		
		//---------------------------------------------------------------------------------
		// Properties specific to message type
		
		// Set properties depending on channel class
		string senderNameText;
		if (messageGeneral)
			senderNameText = messageGeneral.m_sSenderName + ": ";

		m_Widgets.m_wTypeImage.LoadImageFromSet(0, CHAT_IMAGESET, chatTypeImageName);

		string finalMessage = msg.m_sMessage;

		if(senderNameText.IsEmpty())
			m_Widgets.m_wMessageText.ClearFlags(WidgetFlags.NO_LOCALIZATION);
		else
			m_Widgets.m_wMessageText.SetFlags(WidgetFlags.NO_LOCALIZATION);
		
		m_Widgets.m_wMessageText.SetColor(textColor);		
		m_Widgets.m_wMessageText.SetText(senderNameText + finalMessage);	
		
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		playerController.SetPlatformImageTo(senderId, m_Widgets.m_wPlatformImage);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEmptyMessage()
	{
		// Hide elements, leave only text so that it constraints the height of this strip
		m_Widgets.m_wBadge.SetVisible(false);
		m_Widgets.m_wBackgroundImage.SetVisible(false);
		m_Widgets.m_wTypeImage.SetVisible(false);
		m_Widgets.m_wMessageText.SetText(" ");
		m_Widgets.m_wPlatformImage.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] visible
	void SetVisible(bool visible)
	{
		m_wRoot.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
}
