/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseChatComponentClass: GameComponentClass
{
}

//! This component takes care of sending chat messages
class BaseChatComponent: GameComponent
{
	//! Returns the parent entity of this component.
	proto external IEntity GetOwner();
	/*!
	Enables/disables chat-channel. System channel (channelId = 0) is enabled by default
	\param channelId Defined by BaseChatEntity. Indexed from 0.
	\param enabled Enable/disable receiving of messages in channel
	*/
	proto external void SetChannel(int channelId, bool enabled);
	/*!
	\param channelId Defined by BaseChatEntity. Indexed from 0.
	\return true for enabled channel, false otherwise
	*/
	proto external bool GetChannelState(int channelId);
	/*!
	Send message to specified channel
	\param msgStr Message to be sent
	\param channelId Defined by BaseChatEntity. Indexed from 0.
	*/
	proto external void SendMessage(string msgStr, int channelId);
	/*!
	Send message to specific player
	\param msgStr Message to be sent
	\param receiverId Receiver PlayerID
	*/
	proto external void SendPrivateMessage(string msgStr, int receiverId);

	// callbacks

	/*!
	Event triggered on every message delivered to BaseChatComponent
	\param msg Payload
	\param channelId Defined by BaseChatEntity. Indexed from 0.
	\param senderId Sender PlayerID compatible with lobby
	*/
	event protected void OnNewMessage(string msg, int channelId, int senderId);
	/*!
	Event triggered on every private message delivered to BaseChatComponent
	\param msg Payload
	\param senderId Sender PlayerID
	\param receiverId Receiver PlayerID
	*/
	event protected void OnNewPrivateMessage(string msg, int senderId, int receiverId);
	event protected void ShowMessage(string msg);
}

/*!
\}
*/
