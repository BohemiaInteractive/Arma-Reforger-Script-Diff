class GroupChatChannel : BaseChatChannel
{
	//------------------------------------------------------------------------------------------------
	override bool IsDelivering(BaseChatComponent sender, BaseChatComponent receiver)
	{
		SCR_PlayerController senderPC =SCR_PlayerController.Cast(sender.GetOwner());		
		SCR_PlayerController receiverPC =SCR_PlayerController.Cast(receiver.GetOwner());
		if (!receiverPC || !senderPC)
			return false;
		
		SCR_PlayerControllerGroupComponent senderGroup = SCR_PlayerControllerGroupComponent.Cast(senderPC.FindComponent(SCR_PlayerControllerGroupComponent));
		SCR_PlayerControllerGroupComponent receiverGroup = SCR_PlayerControllerGroupComponent.Cast(receiverPC.FindComponent(SCR_PlayerControllerGroupComponent));
		
		if (!senderGroup || !receiverGroup)
			return false;
		
		return senderGroup.GetGroupID() == receiverGroup.GetGroupID();
	}
	
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(BaseChatComponent sender)
	{
		SCR_PlayerController senderPC = SCR_PlayerController.Cast(sender.GetOwner());
		if (!senderPC)
			return false;
		
		SCR_PlayerControllerGroupComponent senderGroupController = SCR_PlayerControllerGroupComponent.Cast(senderPC.FindComponent(SCR_PlayerControllerGroupComponent));
		
		return senderGroupController && senderGroupController.GetGroupID() != -1;
	}
};