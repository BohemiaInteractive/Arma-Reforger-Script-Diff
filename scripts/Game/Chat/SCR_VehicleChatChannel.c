class SCR_VehicleChatChannel: VehicleChatChannel
{
	override bool IsAvailable(BaseChatComponent sender)
	{
		PlayerController playerController = PlayerController.Cast(sender.GetOwner());
		if (!playerController)
			return super.IsAvailable(sender);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(playerController.GetControlledEntity());
		
		return character && character.IsInVehicle();
	}
};