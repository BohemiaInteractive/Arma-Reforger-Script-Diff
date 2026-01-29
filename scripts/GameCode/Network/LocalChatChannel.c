class LocalChatChannel : BaseChatChannel
{
	[Attribute("100","Distance to proximity comunication", UIWidgets.EditBox)]
	private float m_fMaxDistance;
	
	//------------------------------------------------------------------------------------------------
	override bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver)
	{
		if (!sender || !receiver)
			return false;
		SCR_PlayerController senderPC =SCR_PlayerController.Cast(sender.GetOwner());		
		SCR_PlayerController receiverPC =SCR_PlayerController.Cast(receiver.GetOwner());
		if (!receiverPC || !senderPC)
			return false;
		
		vector d1;
		vector d2;
		if (!(receiverPC && receiverPC.GetControlledEntity()))
			return false;
		d2 = receiverPC.GetControlledEntity().GetOrigin();
		d1 = senderPC.GetControlledEntity().GetOrigin();
		
		
		
		
		if (vector.Distance(d1, d2) < m_fMaxDistance)
			return true;
		//Faction receiver
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(BaseChatComponent sender)
	{
		SCR_PlayerController senderPC = SCR_PlayerController.Cast(sender.GetOwner());	
		
		return senderPC && senderPC.GetControlledEntity();
	}
	
};