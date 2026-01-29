/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class BaseChatEntityClass: GenericEntityClass
{
}

class BaseChatEntity: GenericEntity
{
	/*!
	\return Total number of channels
	*/
	proto external int GetChannelsCount();
	/*!
	\param[channelId] Defined by BaseChatEntity. Indexed from 0.
	\return Name of channel as defined in BaseChatEntity
	*/
	proto external string GetChannelName(int channelId);
	/*!
	\param[channelId] Channel ID indexed from 0.
	\return Channel or null if ID is invalid
	*/
	proto external BaseChatChannel GetChannel(int channelId);
	/*!
	\return Channel or null if no whisper channel is configured
	*/
	proto external PrivateMessageChannel GetWhisperChannel();
	/*!
	\return Returns the channel which was setup as default
	*/
	proto external BaseChatChannel GetDefaultChannel();
	/*!
	\return Base channel style object.
	*/
	proto external BaseChatChannelStyle GetStyle();
}

/*!
\}
*/
