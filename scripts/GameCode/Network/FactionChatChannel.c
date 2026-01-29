class FactionChatChannel : BaseChatChannel
{

	
	//------------------------------------------------------------------------------------------------
	override bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver)
	{
		SCR_PlayerController senderPC = SCR_PlayerController.Cast(sender.GetOwner());		
		SCR_PlayerController receiverPC = SCR_PlayerController.Cast(receiver.GetOwner());
		if (!receiverPC || !senderPC)
			return false;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;
		
		Faction senderFaction = factionManager.GetPlayerFaction(senderPC.GetPlayerId());
		Faction receiverFaction = factionManager.GetPlayerFaction(receiverPC.GetPlayerId());
		
		// Compare factions
		if (senderFaction == receiverFaction)
			return true;
		
		// Receiver faction is not the same as sender faction
		return false;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(BaseChatComponent sender)
	{
		
		SCR_PlayerController senderPC = SCR_PlayerController.Cast(sender.GetOwner());
		if (!senderPC)
			return false;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;
		
		Faction senderFaction = factionManager.GetPlayerFaction(senderPC.GetPlayerId());		
		if (senderFaction)
			return true;
		else 
			return false;
	}
};