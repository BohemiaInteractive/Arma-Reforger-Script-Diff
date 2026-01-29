class SCR_ChatComponentClass : BaseChatComponentClass
{
}

class SCR_ChatComponent : BaseChatComponent
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] channelId
	//! \return
	bool GetChannelState(BaseChatChannel channelId)
	{
		ScriptedChatEntity ent = ScriptedChatEntity.Cast(GetGame().GetChat());
		if (!ent)
			return false;
		
		return GetChannelState(ent.GetChannelId(channelId));
	}
	
	private ScriptedChatEntity m_ChatEntity;
	private int m_iRadioProtocolChannel;

	//-----------------------------------------------------------------------------
	override void OnNewMessage(string msg, int channelId, int senderId)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		if (!mgr)
			return;
		
		mgr.OnNewMessageGeneral(msg, channelId, senderId);
	}

	//-----------------------------------------------------------------------------
	override void OnNewPrivateMessage(string msg, int senderId, int receiverId)
	{		
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		if (!mgr)
			return;
		
		mgr.OnNewMessagePrivate(msg, senderId, receiverId);
	}
	
	//-----------------------------------------------------------------------------
	override void ShowMessage(string msg)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		if (!mgr)
			return;
		
		mgr.OnNewMessage(msg);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! API method for the radio protocol use.
	//! Prints the radio protocol message into the chat.
	//! \param[in] msg
	//! \param[in] freq
	static void RadioProtocolMessage(string msg, int freq = -1)
	{
		SCR_ChatPanelManager mgr = SCR_ChatPanelManager.GetInstance();
		if (!mgr)
			return;

		mgr.OnNewMessageRadioProtocol(msg, freq);
	}
}
