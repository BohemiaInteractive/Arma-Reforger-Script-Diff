// Type definition - callbacks of chat commands
typedef func ChatCommandCallback;
void ChatCommandCallback(SCR_ChatPanel panel, string data);
typedef ScriptInvokerBase<ChatCommandCallback> ChatCommandInvoker;


/*!
Singleton object to which SCR_ChatPanels get registered/unregistered.
It stores message history and receives messages from SCR_ChatComponent.
*/
[BaseContainerProps(configRoot: true)]
class SCR_ChatPanelManager : SCR_GameCoreBase
{
	// Constants
	
	// Size of chat history. When more messages than this are added, old messages are deleted.
	protected const int CHAT_HISTORY_SIZE = 256;

	// Start character of each command
	// note: admin commands are different and are not handled by this chat UI system. They start with a different character.
	const string CHAT_COMMAND_CHARACTER = "/";
	
	
	protected bool m_bInitDone = false;
	
	protected ref array<ref SCR_ChatMessage> m_aMessages = {};
	protected ref array<SCR_ChatPanel> m_aChatPanels = {};
	protected ref array<ref SCR_ProfanityFilterRequestCallback> m_aProfanityCallbacks = {};
	
	// Registered chat commands
	protected ref map<string, ref ChatCommandInvoker> m_mCommands = new map<string, ref ScriptInvokerBase<ChatCommandCallback>>;
	
	ScriptedChatEntity m_ChatEntity;	
	
	//------------------------------------------------------------------------------------------------
	// PUBLIC
	
	
	//------------------------------------------------------------------------------------------------
	//! Opens the chat panel and ensures that all other panels are closed.
	void OpenChatPanel(notnull SCR_ChatPanel panel)
	{
		// Ensure all other panels are closed
		foreach (SCR_ChatPanel p : m_aChatPanels)
		{
			if (p != panel)
				p.Internal_Close();
		}
		
		panel.Internal_Open();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Closes the chat panel.
	void CloseChatPanel(notnull SCR_ChatPanel panel)
	{
		// To be safe, just close all of them
		CloseAllChatPanels();
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Toggles the chat panel: opens if it's closed, closes if it's open.
	void ToggleChatPanel(notnull SCR_ChatPanel panel)
	{
		// Ensure all other panels are closed
		foreach (SCR_ChatPanel p : m_aChatPanels)
		{
			if (p != panel)
				p.Internal_Close();
		}
		
		if (panel.IsOpen())
			panel.Internal_Close();
		else
			panel.Internal_Open();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void OnMenuClosed()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		
		if (!menuManager || !menuManager.IsAnyMenuOpen())
			ShowAllChatPanels();
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowAllChatPanels()
	{
		foreach (SCR_ChatPanel panel : m_aChatPanels)
		{
			panel.GetWidget().SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowChatPanel(notnull SCR_ChatPanel panel)
	{
		panel.GetWidget().SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideAllChatPanels()
	{
		foreach (SCR_ChatPanel panel : m_aChatPanels)
		{
			panel.GetWidget().SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseAllChatPanels()
	{
		foreach (SCR_ChatPanel p : m_aChatPanels)
		{
			if (p.IsOpen())
				p.Internal_Close();
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns SCR_ChatPanelManager instance.
	static SCR_ChatPanelManager GetInstance()
	{	
		return SCR_ChatPanelManager.Cast(SCR_GameCoresManager.GetCore(SCR_ChatPanelManager));
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns the array of messages. Array is not a copy, so be careful.
	array<ref SCR_ChatMessage> GetMessages()
	{
		return m_aMessages;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns an array of all commands registered with GetCommandInvoker
	array<string> GetAllRegisteredCommands()
	{
		array<string> a = {};
		foreach (string comm, auto invoker : m_mCommands)
			a.Insert(comm);
		return a;
	}
	
	
	//------------------------------------------------------------------------------------------------
	void ShowHelpMessage(string msg)
	{
		ref SCR_ChatMessage m = new SCR_ChatMessage(msg);
		
		this.OnNewMessage(m);
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns a script invoker for provided chat command, or creates a new one.
	//! This way other systems can create chat commands and subscribe to them.
	ChatCommandInvoker GetCommandInvoker(string name)
	{
		// Bail if name is incorrect
		if (name.IsEmpty())
			return null;
		
		ChatCommandInvoker invoker = m_mCommands.Get(name);
		
		if (!invoker)
		{
			invoker = new ChatCommandInvoker;
			m_mCommands.Insert(name, invoker);
		}
		
		// Verify
		if (!m_mCommands.Get(name))
			Print(string.Format("Error while registering chat command: %1", name), LogLevel.WARNING);
		
		return invoker;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Returns true when there is any chat panel faded in.
	bool GetAnyPanelFadedIn()
	{
		foreach (SCR_ChatPanel panel : m_aChatPanels)
		{
			if (panel.GetFadeIn())
				return true;
		}
		
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Registration of chat panels. These are called when a chat panel is created and destroyed.
	
	//------------------------------------------------------------------------------------------------
	void Register(SCR_ChatPanel panel)
	{
		if (!m_aChatPanels.Contains(panel))
			m_aChatPanels.Insert(panel);
		
		CloseAllChatPanels();
	}
	
	//------------------------------------------------------------------------------------------------
	void Unregister(SCR_ChatPanel panel)
	{		
		m_aChatPanels.RemoveItem(panel);
	}
	
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Handling of new messages. Called by SCR_ChatComponent.
	
	//------------------------------------------------------------------------------------------------
	void OnNewMessagePrivate(string msg, int senderId, int receiverId)
	{
		if (!m_ChatEntity)
			return;
		
		int playerId = GetGame().GetPlayerController().GetPlayerId();
		
		if (! (senderId == playerId || receiverId == playerId))
			return;
		
		SCR_ChatMessagePrivate m = new SCR_ChatMessagePrivate(
			msg,
			m_ChatEntity.GetWhisperChannel(),
			senderId,
			//GetGame().GetPlayerManager().GetPlayerName(senderId),
			SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(senderId),
			receiverId);
		
		OnNewMessage(m);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNewMessageRadioProtocol(string msg, int frequency)
	{		
		SCR_ChatMessageRadioProtocol m = new SCR_ChatMessageRadioProtocol(
			msg,
			frequency);
		
		this.OnNewMessage(m);
	}

	//------------------------------------------------------------------------------------------------
	void OnNewMessageGeneral(string msg, int channelId, int senderId)
	{		
		SCR_ChatMessageGeneral m = new SCR_ChatMessageGeneral(
			msg,
			m_ChatEntity.GetChannel(channelId),
			senderId,
			//GetGame().GetPlayerManager().GetPlayerName(senderId));
			SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(senderId));
		
		this.OnNewMessage(m);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNewMessage(string msg)
	{
		SCR_ChatMessage m = new SCR_ChatMessage(msg);
		
		this.OnNewMessage(m);
	}
	
	
	
	
	//------------------------------------------------------------------------------------------------
	//! Initializaiton
	
	
	//------------------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		// Clear message array so we don't see messages from previous session
		m_aMessages.Clear();
		
		m_ChatEntity = ScriptedChatEntity.Cast(GetGame().GetChat());

		// OnGameStart runs several time for same object,
		// but this code must be run only once
		if (!m_bInitDone)
		{
			InitDefaultChatCommands();
			m_bInitDone = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		m_aMessages.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitDefaultChatCommands()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables all channels
	void Internal_EnableAllChannels()
	{	
		PlayerController pc = GetGame().GetPlayerController();
		
		if (!pc || !m_ChatEntity)
			return;
		
		BaseChatComponent chatComp = BaseChatComponent.Cast(pc.FindComponent(BaseChatComponent));
		
		if (!chatComp)
			return;
		
		for (int i = 0; i< m_ChatEntity.GetChannelsCount() ; i++)
		{
			BaseChatChannel chatChannel = m_ChatEntity.GetChannel(i);
			chatComp.SetChannel(i, true);
		}
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! OTHER
	
	//------------------------------------------------------------------------------------------------
	//! Called from a chat pannel when it detects that user sends a chat command.
	void Internal_OnChatCommand(SCR_ChatPanel panel, string command, string otherData)
	{
		auto invoker = m_mCommands.Get(command);
		
		if (!invoker)
			return;
		
		invoker.Invoke(panel, otherData);
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	// PROTECTED
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnNewMessage(SCR_ChatMessage msg)
	{
		SCR_ProfanityFilterRequestCallback filterCallback = new SCR_ProfanityFilterRequestCallback(msg);
		array<string> textToFilter = {};
		textToFilter.Insert(msg.m_sMessage);
		filterCallback.m_OnResultInstance.Insert(OnMessageFiltered);
		m_aProfanityCallbacks.Insert(filterCallback);
		
		//if we return fail, we call the method manually to still show the unchanged text and delete callback
		if (!GetGame().GetPlatformService().FilterProfanityAsync(textToFilter, filterCallback))
			OnMessageFiltered(filterCallback, textToFilter, msg);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMessageFiltered(SCR_ProfanityFilterRequestCallback callback, array<string> filteredTexts, SCR_ChatMessage originalMessage)
	{		
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.XBOX)
		{
			SCR_ProfaneFilter.ReplaceProfanities(filteredTexts.Get(0), originalMessage.m_sMessage);
		}
		else
		{
			originalMessage.m_sMessage = filteredTexts.Get(0);
		}
		
		m_aMessages.Insert(originalMessage);
		
		if (m_aMessages.Count() > CHAT_HISTORY_SIZE)
			m_aMessages.RemoveOrdered(0);
		
		//! Notify all registered chat panels
		foreach (SCR_ChatPanel panel : m_aChatPanels)
			panel.Internal_OnNewMessage(originalMessage);
		
		m_aProfanityCallbacks.RemoveItem(callback);
	}
};

class SCR_ProfanityFilterRequestCallback : SCR_ScriptProfanityFilterRequestCallback
{
	ref ScriptInvoker m_OnResultInstance = new ScriptInvoker; //(SCR_ProfanityFilterRequestCallback callback, array<string> filteredTexts, SCR_ChatMessage originalMessage)
	ref SCR_ChatMessage m_OriginalMessage;
	
	void SCR_ProfanityFilterRequestCallback(SCR_ChatMessage originalMgs)
	{
		m_OriginalMessage = originalMgs;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFilteredResult()
	{		
		GetTexts(m_FilteredTexts);
		m_OnResultInstance.Invoke(this, m_FilteredTexts, m_OriginalMessage);
	}	
};