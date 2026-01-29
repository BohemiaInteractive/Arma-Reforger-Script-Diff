/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Chat
\{
*/

class BaseChatChannel: ScriptAndConfig
{
	//!String identifier of channel
	proto external string GetName();
	/*!
	Returns channel style object.
	*/
	proto external BaseChatChannelStyle GetStyle();

	// callbacks

	event protected void OnInit (IEntity owner);
	//! Determine, if the message will be delivered to the receiver
	event bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver) { return true; };
	//! Returns true if the message can be sent to this channel
	event bool IsAvailable(BaseChatComponent sender) { return true; };
	/*!
	Method processes chat message before its send to the server and after receiving it by the server.
	Will be revisited and made osbolete soon
	\param message Chat message
	\param senderChatComp Senders ChatComponent
	\param isAuthority true when executed on the server
	\return Return if the message should be send further through the chat pipeline (send to the server or to the other clients)
	*/
	event bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority) { return true; };
}

/*!
\}
*/
