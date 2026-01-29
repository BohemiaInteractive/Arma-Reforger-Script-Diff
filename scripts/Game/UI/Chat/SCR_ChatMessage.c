/*!
Classes for chat messages.
*/

//! Base class for all chat messages
//! Has message text, creation time
class SCR_ChatMessage
{
	string 				m_sMessage;

	void SCR_ChatMessage(string message)
	{
		m_sMessage = message;
	}
};

//! Generic messages, used by players: they originate from someone, sometimes directed to someone, and use some game channel
//! Adds a channel, sender id, sender name
class SCR_ChatMessageGeneral : SCR_ChatMessage
{
	BaseChatChannel m_Channel;
	int				m_iSenderId;
	string			m_sSenderName;

	void SCR_ChatMessageGeneral(string message, BaseChatChannel channel = null, int senderId = -1, string senderName = "")
	{
		m_iSenderId = senderId;
		m_sSenderName = senderName;
		m_Channel = channel;
	}
};

//! Private message
//! Adds a specific receiver ID
class SCR_ChatMessagePrivate : SCR_ChatMessageGeneral
{
	int				m_iReceiverId;
	
	void SCR_ChatMessagePrivate(string message, BaseChatChannel channel = null, int senderId = -1, string senderName = "", int receiverId = -1)
	{
		m_iReceiverId = receiverId;
	}
};

// Class for radio protocol messages
class SCR_ChatMessageRadioProtocol : SCR_ChatMessage
{
	// todo figure out what we need for the radio protocol
	
	int m_iFrequency;
	
	void SCR_ChatMessageRadioProtocol(string message, int frequency)
	{
		m_iFrequency = frequency;
	}
};