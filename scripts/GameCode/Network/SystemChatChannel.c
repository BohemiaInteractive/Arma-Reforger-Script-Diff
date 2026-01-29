class SystemChatChannel : BaseChatChannel
{	
	//------------------------------------------------------------------------------------------------
	override bool ProcessMessage(BaseChatComponent sender, string message, bool isAuthority)
	{
		if (isAuthority && message.StartsWith("!shutdown"))
		{
			Print("Commencing server shutdown via chat command.");
			GetGame().RequestClose();
		}

		return true;
	}
	
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