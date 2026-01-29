//------------------------------------------------------------------------------------------------
class ServerChatChannel : BaseChatChannel
{

	//------------------------------------------------------------------------------------------------
	override bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver)
	{
		return true;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(BaseChatComponent sender)
	{
		
		SCR_PlayerController senderPC =SCR_PlayerController.Cast(sender.GetOwner());
		if (!senderPC)
			return false;
		
		if (RplSession.Mode() == RplMode.Client)
		{
			return false;
		}
			
		else 
		{
			return true;
		}
			
	}

};
