// Current state of the chat:
// - There is no reliable way how to know that messagebox is visible
//   This is a problem with browsing history

class ScriptedChatEntityClass: BaseChatEntityClass
{
};

class ScriptedChatEntity : BaseChatEntity
{	
	//------------------------------------------------------------------------------------------------
	void ScriptedChatEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
	}	

	
	//------------------------------------------------------------------------------------------------
	int GetChannelId(BaseChatChannel channel)
	{
		int count = GetChannelsCount();
		
		for (int i = 0; i < count; i++)
		{
			if (GetChannel(i) == channel)
				return i;
		}
		
		return -1;
	}
	
};