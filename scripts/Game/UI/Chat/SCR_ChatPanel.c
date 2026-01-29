/*!
Component which must be attached to chat layout.
Use public API to open the chat. It will not open itself.
However it will close itself automatically when a message is sent.
*/
class SCR_ChatPanel : ScriptedWidgetComponent
{
	// Constants

	protected const string STR_AVAILABLE_CHANNELS = "#AR-Chat_AvailableChannels";
	protected const string STR_CHANNEL_DISABLED = "#AR-Chat_ChannelDisabled";	// Message to show when channel is disabled.
	protected const string STR_CHANNEL_UNKNOWN = "#AR-Chat_ChannelUnknown";		// Message to show when entering a non valid chat command.
	protected const string STR_MESSAGE_LIMIT_REACHED = "#AR-Chat_LimitReached";	// Message to show when max characters for chat input is reached.

	//Invokers
	protected ref ScriptInvoker m_OnChatOpen;
	protected ref ScriptInvoker m_OnChatClosed;

	// Channel styles

	[Attribute()]
	ref SCR_ChatMessageStyle m_SystemChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_RadioProtocolStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_GlobalChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_GroupChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_FactionChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_VehicleChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_LocalChannelStyle;

	[Attribute()]
	ref SCR_ChatMessageStyle m_DirectChannelStyle;

	// Other attributes

	[Attribute("9", UIWidgets.EditBox, "Max amount of message line widgets")]
	protected int m_iMessageLineCount;

	[Attribute("20", UIWidgets.EditBox, "Amount of time till whole chat fades out, seconds")]
	protected float m_fFadeOutThreshold_s;

	[Attribute("false", UIWidgets.CheckBox, "When true, chat is always visible. When false, chat will fade out if msg box is hidden for more than m_fFadeOutThreshold_s time")]
	protected bool m_bAlwaysVisible;

	[Attribute("{973C90F6B6135A50}UI/layouts/HUD/Chat/ChatMessage.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sChatMessageLineLayout;

	// State of UI
	protected int m_iHistoryId = 0;			// Id of message for history scrolling
	protected int m_iHistoryIdLowestMessage = 0;			// Id of message for history scrolling
	protected bool m_bHistoryMode = false;	// When disabled, we are always looking at new messages.
	protected ref array<SCR_ChatMessageLineComponent> m_aMessageLines = {};	// Message line components. We reuse same lines for messages.
	protected bool m_bOpen = false;

	// Cached components
	protected ScriptedChatEntity m_ChatEntity;
	protected BaseChatChannel m_ActiveChannel;

	// Animations
	ref SCR_FadeInOutAnimator m_MessageHistoryAnimator;

	// Widgets and components
	protected Widget m_wRoot;
	protected ref SCR_ChatPanelWidgets m_Widgets = new SCR_ChatPanelWidgets();
	EditBoxFilterComponent m_MessageEditBoxComponent;

	//------------------------------------------------------------------------------------------------
	// PUBLIC

	//------------------------------------------------------------------------------------------------
	//! This must be called for chat to work. Typically it should be called from HUD Manager or from Menu.
	void OnUpdateChat(float timeSlice)
	{
		m_MessageHistoryAnimator.ForceVisible(m_bOpen || m_bAlwaysVisible);

		if (m_bOpen)
		{

			GetGame().GetInputManager().ActivateContext("ChatContext", 1);

			if (m_Widgets.m_MessageEditBox.GetText().Length() >= m_MessageEditBoxComponent.GetNumLimitOfCharacters())
			{
				ShowChannelWarning(null, STR_MESSAGE_LIMIT_REACHED);
				m_Widgets.m_WarningChannelMessage.SetVisible(true);
			}
			else
			{
				m_Widgets.m_WarningChannelMessage.SetVisible(false);
			}
		}

		m_MessageHistoryAnimator.Update(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Adjust the amount of messages shown when the size of the HUD changes
	void OnHUDResize(int newMessageLineCount)
	{
		m_iMessageLineCount = newMessageLineCount;
		int count = m_aMessageLines.Count();

		if (count == m_iMessageLineCount)
			return;

		if (count > m_iMessageLineCount)
		{
			for (int i = count - 1; i >= m_iMessageLineCount; i--)
			{
				SCR_ChatMessageLineComponent w = m_aMessageLines[i];

				if (w)
				{
					w.GetRootWidget().RemoveFromHierarchy();
					m_aMessageLines.RemoveItemOrdered(w);
				}

			}
		}
		else
		{
			for (int i = count; i < m_iMessageLineCount; i++)
			{
				Widget lineWidget = GetGame().GetWorkspace().CreateWidgets(m_sChatMessageLineLayout, m_Widgets.m_MessageHistory);
				SCR_ChatMessageLineComponent comp = SCR_ChatMessageLineComponent.Cast(lineWidget.FindHandler(SCR_ChatMessageLineComponent));
				comp.SetEmptyMessage();
				m_aMessageLines.Insert(comp);
			}
		}

		UpdateChatMessages();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpen()
	{
		return m_bOpen;
	}

	//------------------------------------------------------------------------------------------------
	//! True when fading in or when has faded in already.
	bool GetFadeOut()
	{
		int state = m_MessageHistoryAnimator.GetState();
		return state == SCR_FadeInOutAnimator.STATE_FADE_OUT || state == SCR_FadeInOutAnimator.STATE_FADING_DONE;
	}

	//------------------------------------------------------------------------------------------------
	//! True when fading out or has faded out already.
	bool GetFadeIn()
	{
		int state = m_MessageHistoryAnimator.GetState();
		return state == SCR_FadeInOutAnimator.STATE_FADE_IN || state == SCR_FadeInOutAnimator.STATE_FADE_OUT_WAIT;
	}


	//------------------------------------------------------------------------------------------------
	// INTERNAL / PROTECTED


	//------------------------------------------------------------------------------------------------
	//! Don't call it yourself, use SCR_ChatPanelManager instead.
	void Internal_Open()
	{
		// If m_OnChatOpen is not null we know it's used by the HudManager so we check if the parent (HudManager slot) is enabled (enabled = visible)
		if (m_bOpen || m_OnChatOpen && !m_wRoot.GetParent().IsEnabled())
			return;

		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();

		// When we open the chat, history mode is disabled unless we start scrolling
		m_bHistoryMode = false;
		m_iHistoryId = mgr.GetMessages().Count();
		m_iHistoryIdLowestMessage = m_iHistoryId;

		m_bOpen = true;
		if (m_OnChatOpen)
			m_OnChatOpen.Invoke();

		//m_Widgets.m_EditOverlay.SetOpacity(1);
		AnimateWidget.Opacity(m_Widgets.m_EditOverlay, 1, UIConstants.FADE_RATE_DEFAULT);

		// Animate the edit background color
		Color colorStart = Color.FromSRGBA(226, 167, 79, 255); // Orange
		Color colorEnd = Color.FromSRGBA(0, 0, 0, 140); // Black
		m_Widgets.m_EditBackgroundImage.SetColor(colorStart);
		AnimateWidget.Color(m_Widgets.m_EditBackgroundImage, colorEnd, UIConstants.FADE_RATE_SLOW);

		m_Widgets.m_MessageEditBox.SetEnabled(true);

		bool bFocus = false;
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			bFocus = true;

		if (bFocus)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_MessageEditBox);
			m_Widgets.m_MessageEditBox.ActivateWriteMode();
		}

		if (m_ActiveChannel)
		{
			if (!m_ActiveChannel.IsAvailable(GetChatComponent()))
				CycleChannels(true);
		}


		// Instantly show the channel name overlay when we open the chat
		if (m_Widgets.m_ChannelTagOverlay)
		{
			m_Widgets.m_ChannelTagOverlay.SetVisible(true);
		}


		m_Widgets.m_WarningChannelMessage.SetVisible(false);
		UpdateChatMessages();
	}


	//------------------------------------------------------------------------------------------------
	//! Don't call it yourself, use SCR_ChatPanelManager instead.
	void Internal_Close()
	{
		if (!m_bOpen)
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace.GetFocusedWidget() == m_Widgets.m_MessageEditBox)
		{
			workspace.SetFocusedWidget(null);
		}

		m_bOpen = false;
		if (m_OnChatClosed)
			m_OnChatClosed.Invoke();

		if (m_Widgets.m_MessageEditBox)
		{
			m_Widgets.m_MessageEditBox.SetText(string.Empty);
			m_Widgets.m_MessageEditBox.SetEnabled(false);
		}

		AnimateWidget.StopAnimation(m_Widgets.m_EditOverlay, WidgetAnimationOpacity); // Stop the fade in animation in case it's not completed to prevent chat sticking to screen
		m_Widgets.m_EditOverlay.SetOpacity(0); // Hide it instantly, otherwise it looks weird

		m_Widgets.m_WarningChannelMessage.SetVisible(false);

		m_iHistoryId = SCR_ChatPanelManager.GetInstance().GetMessages().Count() - 1; // Force scroll to the end
		m_iHistoryIdLowestMessage = m_iHistoryId;

		UpdateChatMessages();
	}


	//------------------------------------------------------------------------------------------------
	void Internal_OnNewMessage(SCR_ChatMessage msg)
	{
		// Update history index
		if (!m_bHistoryMode)
		{
			m_iHistoryId = SCR_ChatPanelManager.GetInstance().GetMessages().Count() - 1; // Scroll to the end if we are not scrolling the chat now
			m_iHistoryIdLowestMessage = m_iHistoryId;
		}

		m_MessageHistoryAnimator.FadeIn();
		this.UpdateChatMessages();
	}

	//------------------------------------------------------------------------------------------------
	notnull Widget GetWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		// Bail if we are in workbench editor
		if (SCR_Global.IsEditMode())
			return;

		m_wRoot = w;

		SCR_ChatPanelManager chatPanelMgr = SCR_ChatPanelManager.GetInstance();
		m_ChatEntity = ScriptedChatEntity.Cast(GetGame().GetChat());

		if (!chatPanelMgr)
		{
			Print("[Chat] SCR_ChatPanelManager is not found. Chat will not work.", LogLevel.ERROR);
			return;
		}

		m_Widgets.Init(w);
		m_MessageEditBoxComponent = m_Widgets.m_MessageEditBoxComponent0;

		VerifyChannelStyles();


		m_Widgets.m_MessageEditBox.SetEnabled(false);

		// This callback will hide/show the overlay depending on typed text
		m_Widgets.m_MessageEditBoxComponent1.GetOnChange().Insert(Callback_OnEditBoxChange);

		// Init the widgets in the history layout
		m_MessageHistoryAnimator = new SCR_FadeInOutAnimator(m_Widgets.m_MessageHistory, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, m_fFadeOutThreshold_s);

		for (int i = 0; i < m_iMessageLineCount; i++)
		{
			Widget lineWidget = GetGame().GetWorkspace().CreateWidgets(m_sChatMessageLineLayout, m_Widgets.m_MessageHistory);
			SCR_ChatMessageLineComponent comp =
				SCR_ChatMessageLineComponent.Cast(lineWidget.FindHandler(SCR_ChatMessageLineComponent));
			comp.SetEmptyMessage();
			m_aMessageLines.Insert(comp);
		}

		// Init inputs
		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.AddActionListener("ChatSendMessage", EActionTrigger.DOWN, Callback_OnSendMessageAction);
		inputMgr.AddActionListener("ChatEscape", EActionTrigger.DOWN, Callback_OnCloseAction);
		inputMgr.AddActionListener("ChatConfirm", EActionTrigger.DOWN, Callback_OnGamepadConfirmAction);
		inputMgr.AddActionListener("ChatHistoryBrowse", EActionTrigger.VALUE, Callback_OnHistoryAction);
		inputMgr.AddActionListener("ChatSwitchChannel", EActionTrigger.VALUE, Callback_OnSwitchChannelAction);
		inputMgr.AddActionListener("ChatSwitchChannelSpace", EActionTrigger.DOWN, Callback_OnPressSpaceAfterCommand);

		//m_Widgets.m_EditOverlay.SetVisible(false);
		m_Widgets.m_EditOverlay.SetOpacity(0);

		//Initialization will depend on avaliable channels for the player
		if (m_ChatEntity && chatPanelMgr)
		{
			chatPanelMgr.Internal_EnableAllChannels();
			SetActiveChannel(m_ChatEntity.GetDefaultChannel());
		}

		// Instantly show previous messages when this UI panel is created.
		// But only show it if any chat panel is faded in.
		// We don't want to fade in the panel if previous one was inactive for long time.
		if (chatPanelMgr.GetAnyPanelFadedIn())
		{
			m_iHistoryId = chatPanelMgr.GetMessages().Count() - 1;
			m_iHistoryIdLowestMessage = m_iHistoryId;
			m_MessageHistoryAnimator.FadeIn();
			UpdateChatMessages();
		}

		// Register at chat panel mgr
		if (chatPanelMgr)
			chatPanelMgr.Register(this);
	}



	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// Bail if we are in workbench editor
		if (SCR_Global.IsEditMode())
			return;

		InputManager inputMgr = GetGame().GetInputManager();
		inputMgr.RemoveActionListener("ChatSendMessage", EActionTrigger.DOWN, Callback_OnSendMessageAction);
		inputMgr.RemoveActionListener("ChatEscape", EActionTrigger.DOWN, Callback_OnCloseAction);
		inputMgr.RemoveActionListener("ChatConfirm", EActionTrigger.DOWN, Callback_OnGamepadConfirmAction);
		inputMgr.RemoveActionListener("ChatHistoryBrowse", EActionTrigger.VALUE, Callback_OnHistoryAction);
		inputMgr.RemoveActionListener("ChatSwitchChannel", EActionTrigger.VALUE, Callback_OnSwitchChannelAction);
		inputMgr.RemoveActionListener("ChatSwitchChannelSpace", EActionTrigger.DOWN, Callback_OnPressSpaceAfterCommand);

		// Unregister at chat panel mgr
		SCR_ChatPanelManager chatPanelMgr = SCR_ChatPanelManager.GetInstance();
		if (chatPanelMgr)
			chatPanelMgr.Unregister(this);
	}



	//------------------------------------------------------------------------------------------------
	// Updates the display with the messages
	// lastMessageId - id of the last message to be shown in chat
	protected void UpdateChatMessages()
	{
		array<ref SCR_ChatMessage> messages = SCR_ChatPanelManager().GetInstance().GetMessages();

		int messageCount = messages.Count();

		if (messageCount > 0)
		{
			int lastMessageId = m_iHistoryId;
			if (!m_bHistoryMode)
				lastMessageId = m_iHistoryIdLowestMessage;

			lastMessageId = Math.ClampInt(lastMessageId, 0, messageCount - 1);
			int currentMessageCount = m_iMessageLineCount;

			for (int i = 0; i < currentMessageCount; i++)
			{
				int messageId = lastMessageId - i;	// Message 0 is oldest message
				int widgetId = i;					// Widget 0 is at the bottom

				if (i > m_aMessageLines.Count() - 1 || i < 0)
					continue;

				SCR_ChatMessageLineComponent lineComp = m_aMessageLines[widgetId];

				if (messageId >= 0 && messageId <= lastMessageId)
				{
					SCR_ChatMessage msg = messages[messageId];
					SCR_ChatMessageStyle style = GetMessageStyle(msg);
					lineComp.SetVisible(true);
					lineComp.SetMessage(msg, style);

				}
				else
				{
					// This line is blank
					//this.DecorateBlankMessageWidget(messageWidget);
					lineComp.SetVisible(false);
				}
			}
		}
		else
		{
			for (int i = 0; i < m_iMessageLineCount; i++)
			{
				if (i > m_aMessageLines.Count() - 1 || i < 0)
					continue;

				SCR_ChatMessageLineComponent lineComp = m_aMessageLines[i];

				// This line is blank
				//this.DecorateBlankMessageWidget(messageWidget);
				lineComp.SetVisible(false);
			}
		}
	}


	//------------------------------------------------------------------------------------------------
	protected void SendMessage()
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();

		SCR_ChatComponent chatComponent = GetChatComponent();

		if (!chatComponent || !m_ActiveChannel)
			return;

		string message;
		if (m_Widgets.m_MessageEditBox)
			message = m_Widgets.m_MessageEditBox.GetText();
		else
			return;

		// Check if we want to send some command
		string cmd = this.GetCommand(message);

		// If there's a command, we don't want to really send the message
		// Note: by now the channel tag or player name have been already removed from the message,
		// so there is no command here any more
		if (!cmd.IsEmpty())
		{
			// Notify the chat panel mgr, pass the message with removed command
			message = message.Substring(cmd.Length() + 1, message.Length() - cmd.Length() - 1);
			message.TrimInPlace();

			mgr.Internal_OnChatCommand(this, cmd, message);

			return;
		}

		if (!m_ActiveChannel.IsAvailable(chatComponent))
		{
			SCR_ChatMessageStyle style = this.GetChannelStyle(m_ActiveChannel);
			SCR_ChatPanelManager.GetInstance().ShowHelpMessage(STR_CHANNEL_DISABLED);
		}
		else
		{
			if (PrivateMessageChannel.Cast(m_ActiveChannel))
			{
				// Get whisper receiver ID
				int playerId = this.GetPlayerIdByName(cmd);

				if (playerId != -1)
				{
					// Remove player name from the message
					message = message.Substring(1, message.Length() - 1);

					chatComponent.SendPrivateMessage(message, playerId);
				}
			}
			else
			{
				int channelId = GetChannelId(m_ActiveChannel);
				chatComponent.SendMessage(message, channelId);
			}
		}
	}



	//------------------------------------------------------------------------------------------------
	//! next: true - cycle up, false - cycle down
	protected void CycleChannels(bool next)
	{
		if (!m_ChatEntity)
			return;

		SCR_ChatComponent chatComponent = GetChatComponent();

		if (!chatComponent)
			return;

		int id = 0;

		if (m_ActiveChannel)
			id = GetChannelId(m_ActiveChannel);

		int nTries = 0;
		int channelCount = m_ChatEntity.GetChannelsCount();

		while (nTries < channelCount)
		{
			if (next)
			{
				id++;
				if (id == channelCount)
					id = 0;
			}
			else
			{
				id--;
				if (id == -1)
					id = channelCount - 1;
			}

			BaseChatChannel channel = m_ChatEntity.GetChannel(id);

			if (channel.IsAvailable(chatComponent))
			{
				this.SetActiveChannel(channel);
				return;
			}

			nTries++;
		}
	}



	//------------------------------------------------------------------------------------------------
	protected void ShowChannelWarning(BaseChatChannel Chatchannel = null, string message = "")
	{
		if (Chatchannel && message == string.Empty)
			m_Widgets.m_WarningText.SetText(STR_CHANNEL_DISABLED);
		else
			m_Widgets.m_WarningText.SetText(message);

		m_Widgets.m_WarningChannelMessage.SetVisible(true);
	}



	//------------------------------------------------------------------------------------------------
	//! Returns command from this message string, if it has a command. Otherwise returns empty string.
	//! Returned string is always lowercase
	protected string GetCommand(string s)
	{
		if (!s.StartsWith(SCR_ChatPanelManager.CHAT_COMMAND_CHARACTER) || s.Length() < 2)
			return string.Empty;

		int cmdEnd = s.IndexOf(" ");

		// No space, everything ot the right of / is command
		if (cmdEnd == -1)
			cmdEnd = s.Length();

		string ret = s.Substring(1, cmdEnd - 1);
		ret.ToLower();

		return ret;
	}



	//------------------------------------------------------------------------------------------------
	//! Returns string with help message about available channels.
	string Internal_GetChannelListHelpMessage()
	{
		SCR_ChatComponent chatComponent = GetChatComponent();

		if (!m_ChatEntity || !chatComponent)
			return string.Empty;

		BaseChatChannel chatChannel;
		string help_message = STR_AVAILABLE_CHANNELS +": ";
		for (int j = 0; j < m_ChatEntity.GetChannelsCount(); j++)
		{
			if (chatComponent.GetChannelState(j))
			{
				chatChannel = m_ChatEntity.GetChannel(j);
				if (chatChannel.IsAvailable(chatComponent))
				{
					SCR_ChatMessageStyle style = GetChannelStyle(chatChannel);

					// Resolve color of the channel icon
					Color imageColor = Color.FromInt(Color.WHITE);
					if (style.m_bColoredIcon)
						imageColor = style.m_Color;

					string tagImage = string.Format("<image set=\"%1\" name=\"%2\" scale=\"1\"/>",
						style.m_IconImageset,
						style.m_sIconName);

					string tagImageColored = SCR_RichTextTags.TagColor(tagImage, imageColor);

					help_message += string.Format("\n /%1  %2 %3",
						style.m_sPrefix,
						tagImageColored,
						style.m_sName);
				}
			}

		}

		return help_message;
	}



	//------------------------------------------------------------------------------------------------
	protected BaseChatChannel FindChannelByPrefix(string prefix)
	{
		if (!m_ChatEntity)
			return null;

		bool found = false;
		int i = 0;
		int channelCount = m_ChatEntity.GetChannelsCount();
		while (!found && i < channelCount)
		{
			BaseChatChannel channel = m_ChatEntity.GetChannel(i);
			SCR_ChatMessageStyle style = GetChannelStyle(channel);

			if (style)
			{
				if (style.m_sPrefix == prefix)
					return channel;
			}

			i++;
		}
		return null;
	}



	//------------------------------------------------------------------------------------------------
	protected int GetPlayerIdByName(string name)
	{
		if (name.IsEmpty())
			return -1;

		PlayerManager pm = GetGame().GetPlayerManager();
		// All players because the message could be from someone who already disconnected
		array<int> players;
		pm.GetAllPlayers(players);

		for (int i = pm.GetAllPlayerCount() - 1; i >= 0; --i)
		{
			if (pm.GetPlayerName(i) == name)
				return i;
		}

		return -1;
	}



	//------------------------------------------------------------------------------------------------
	//! Switches channels and replaces the channel tag
	protected void SetActiveChannel(notnull BaseChatChannel channel, string ReceiverName = string.Empty)
	{
		SCR_ChatComponent chatComponent = GetChatComponent();

		if (!chatComponent || !m_ChatEntity)
			return;

		int channelId = m_ChatEntity.GetChannelId(channel);
		m_Widgets.m_WarningChannelMessage.SetVisible(false);
		m_ActiveChannel = channel;

		SCR_ChatMessageStyle style = m_SystemChannelStyle;

		// Set color and image of the channel tag

		// Resolve style
		style = GetChannelStyle(m_ActiveChannel);

		Color color = Color.FromInt(Color.WHITE);
		if (style.m_bColoredIcon)
		{
			color = style.m_Color;
		}
		m_Widgets.m_ChannelTypeImage.SetColor(color);
		m_Widgets.m_ChannelTagEditor.SetColor(color);
		string chatTypeImageName = style.m_sIconName;
		if (!chatTypeImageName.IsEmpty())
			m_Widgets.m_ChannelTypeImage.LoadImageFromSet(0, style.m_IconImageset, chatTypeImageName);

		string channelName = style.m_sName;

		if (PrivateMessageChannel.Cast(channel))
		{
			if (ReceiverName != string.Empty)
			{
				m_Widgets.m_ChannelTagEditor.SetText(channelName + " " + ReceiverName);
				m_Widgets.m_ChannelTagOverlay.SetText(channelName + " " + ReceiverName);
			}
		}
		else if (chatComponent.GetChannelState(channel))
		{
			if (m_ActiveChannel)
			{
				if (m_Widgets.m_ChannelTagOverlay)
				{
					m_Widgets.m_ChannelTagEditor.SetText(channelName);
					//string tagOverlayText = string.Format(WidgetManager.Translate("#AR-Chat_SendToChannel", WidgetManager.Translate(style.m_sNameLower)));
					m_Widgets.m_ChannelTagOverlay.SetTextFormat("#AR-Chat_SendToChannel", style.m_sNameLower);
				}
			}
		}
	}



	//------------------------------------------------------------------------------------------------
	protected void VerifyChannelStyles()
	{
		int n = 0;

		// Ensure at least system style is available
		if (!m_SystemChannelStyle)
		{
			m_SystemChannelStyle = new SCR_ChatMessageStyle();
			n++;
		}


		array<Managed> styles = {
			m_SystemChannelStyle,
			m_RadioProtocolStyle,
			m_GlobalChannelStyle,
			m_FactionChannelStyle,
			m_VehicleChannelStyle,
			m_LocalChannelStyle,
			m_DirectChannelStyle
		};

		foreach (Managed style : styles)
		{
			if (!style)
				n++;
		}

		if (n > 0)
		{
			string s = string.Format("[Chat] %1 chat channel styles are not configured.", n);
			Print(s, LogLevel.ERROR);
		}
	}



	//------------------------------------------------------------------------------------------------
	//! Returns style of this channel. If not resolved, returns system style.
	protected SCR_ChatMessageStyle GetChannelStyle(BaseChatChannel channel)
	{
		if (channel == null)
			return m_SystemChannelStyle;

		SCR_ChatMessageStyle style;
		switch (channel.Type())
		{
			case BaseChatChannel:
			{
				if (channel.GetName() == "Global")
					style = m_GlobalChannelStyle;
				break;
			}

			case GroupChatChannel:
				style = m_GroupChannelStyle;
				break;
			case FactionChatChannel:
				style = m_FactionChannelStyle;
				break;
			case SCR_VehicleChatChannel:
				style = m_VehicleChannelStyle;
				break;
			case LocalChatChannel:
				style = m_LocalChannelStyle;
				break;
			case PrivateMessageChannel:
				style = m_DirectChannelStyle;
				break;
		}

		if (!style)
			return m_SystemChannelStyle;

		return style;
	}



	//------------------------------------------------------------------------------------------------
	protected SCR_ChatMessageStyle GetMessageStyle(notnull SCR_ChatMessage msg)
	{
		// Is it a radio protocol msg? Radio protocol is msg is special.
		if (SCR_ChatMessageRadioProtocol.Cast(msg))
			return m_RadioProtocolStyle;

		// Is it a general msg? General msgs have a channel.
		SCR_ChatMessageGeneral generalMsg = SCR_ChatMessageGeneral.Cast(msg);
		if (generalMsg)
		{
			if (generalMsg.m_Channel)
			{
				return GetChannelStyle(generalMsg.m_Channel);
			}
		}

		// Perhaps a system message then or smth similar
		return m_SystemChannelStyle;
	}



	//------------------------------------------------------------------------------------------------
	protected SCR_ChatComponent GetChatComponent()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return null;

		return SCR_ChatComponent.Cast(pc.FindComponent(SCR_ChatComponent));
	}


	//------------------------------------------------------------------------------------------------
	protected int GetChannelId(notnull BaseChatChannel channel)
	{
		if (!m_ChatEntity)
			return -1;

		return m_ChatEntity.GetChannelId(channel);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChatOpen()
	{
		if (!m_OnChatOpen)
			m_OnChatOpen = new ScriptInvoker();
		return m_OnChatOpen;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChatClosed()
	{
		if (!m_OnChatClosed)
			m_OnChatClosed = new ScriptInvoker();
		return m_OnChatClosed;
	}

	//------------------------------------------------------------------------------------------------
	SCR_FadeInOutAnimator GetFadeInOutAnimator()
	{
		return m_MessageHistoryAnimator;
	}





	//------------------------------------------------------------------------------------------------
	// CALLBACKS AND ACTIONS


	//------------------------------------------------------------------------------------------------
	//! Called by space bar when we change channel by tag
	protected void Callback_OnPressSpaceAfterCommand()
	{
		if (!m_bOpen)
			return;

		SCR_ChatComponent chatComponent = GetChatComponent();

		if (!chatComponent)
			return;

		string message;

		message = m_Widgets.m_MessageEditBox.GetText();
		string cmd = this.GetCommand(message);

		// Do nothing if there is no command
		// Or the command doesn't appear to be a channel tag
		if (cmd.IsEmpty() || cmd.Length() != 1 || !("0123456789".Contains(cmd)))
			return;

		BaseChatChannel channel = FindChannelByPrefix(cmd);

		if (!channel)
		{
			ShowChannelWarning(null, STR_CHANNEL_UNKNOWN);
			return;
		}

		// Remove channel prefix
		message = message.Substring(cmd.Length() + 1, message.Length() - cmd.Length() - 1);
		message.TrimInPlace();

		if (channel.IsAvailable(chatComponent))
		{
			this.SetActiveChannel(channel);
			m_Widgets.m_MessageEditBox.SetText(message);

			// SetText doesn't cause a call to OnChange event handler of the message box
			// so we call it ourselves
			Callback_OnEditBoxChange();
		}
		else
		{
			ShowChannelWarning(channel, STR_CHANNEL_DISABLED);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Esc is pressed
	protected void Callback_OnCloseAction()
	{
		Internal_Close();
	}


	//------------------------------------------------------------------------------------------------
	//! Called when chat toggle key is pressed.
	protected void Callback_OnSendMessageAction()
	{
		if (!m_bOpen)
			return;

		if (!m_Widgets.m_MessageEditBox.GetText().IsEmpty())
			SendMessage();

		Internal_Close();

		// Do nothing if it's closed. It's opened externally.
	}


	//------------------------------------------------------------------------------------------------
	//! When chat is opened while using gamepad, we don't set focus on edit box, so that player can change channel.
	//! Instead focus is set when player uses the ChatConfirm action. Should be mapped only to gamepad.
	protected void Callback_OnGamepadConfirmAction()
	{
		if (!m_bOpen)
			return;

		Widget wFocused = GetGame().GetWorkspace().GetFocusedWidget();
		if (wFocused != m_Widgets.m_MessageEditBox)
			GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_MessageEditBox);

		if (!m_Widgets.m_MessageEditBox.IsInWriteMode())
			m_Widgets.m_MessageEditBox.ActivateWriteMode();
	}


	//------------------------------------------------------------------------------------------------
	//! History scrolling
	protected void Callback_OnHistoryAction(float value)
	{
		if (!m_bOpen)
			return;

		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();

		// Minimum how many messages we want to show while scrolling
		// When too few messages, chat looks weird
		// todo we shouldn't use message count but instead real positioning of msg widget
		// because messages might be multiline
		int minHistoryOffset = m_iMessageLineCount;

		array<ref SCR_ChatMessage> messages = mgr.GetMessages();
		int messagesCount = messages.Count();

		if (value != 0)
		{
			if (messagesCount < minHistoryOffset)
			{
				// Too few messages to scroll
				m_iHistoryId = messagesCount - 1;
				m_iHistoryIdLowestMessage = m_iHistoryId;
				m_bHistoryMode = false;
			}
			else
			{
				m_iHistoryId -= value;
				int lastMessageId = messagesCount - 1;
				m_iHistoryId = Math.ClampInt(m_iHistoryId, minHistoryOffset - 1, lastMessageId);

				int linesCount = 0;
				int offset;
				bool increasing = true;
				int step = 1;
				bool hadToIncrease;

				while (linesCount < m_iMessageLineCount && messagesCount > m_iHistoryId - offset && m_iHistoryId - offset >= -1)
				{
					linesCount += 1;
					if (m_iHistoryId - offset > m_iHistoryId)
						m_iHistoryIdLowestMessage = m_iHistoryId - offset;
					else
						m_iHistoryIdLowestMessage = m_iHistoryId;

					offset += step;

					if (step < 0)
						hadToIncrease = true; // This ensures we don't scroll history id unnecesarily too deep (deeper than what's visible)

					if (m_iHistoryId - offset < 0) // No more messges above this one, we have to try the other way
					{
						step = -1;
						offset = -1;

						if (linesCount >= m_iMessageLineCount) // Reached max at the first message -> make sure to show it
							m_iHistoryIdLowestMessage = m_iHistoryId;
					}
					else if (m_iHistoryId - offset >= messagesCount) // Ran out of messages, abort
						break;
				}

				if (hadToIncrease)
					m_iHistoryId += value;

				m_bHistoryMode = m_iHistoryId != lastMessageId;
				UpdateChatMessages();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Channel switching
	protected void Callback_OnSwitchChannelAction(float value, EActionTrigger reason)
	{
		if (!m_bOpen)
			return;

		if (value != 0)
		{
			bool direction = value > 0;
			CycleChannels(direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called when text in the edit box changes.
	protected void Callback_OnEditBoxChange()
	{
		if (!m_bOpen)
			return;

		int textLen = m_Widgets.m_MessageEditBox.GetText().Length();
		m_Widgets.m_ChannelTagOverlay.SetVisible(textLen == 0); // Visible when there is no text
	}
}
