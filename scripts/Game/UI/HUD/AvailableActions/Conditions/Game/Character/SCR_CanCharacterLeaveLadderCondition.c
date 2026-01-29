//------------------------------------------------------------------------------------------------
//! Returns true if character can control action and it's not obstructed by:
//! Falling, swimming, vehicle
[BaseContainerProps()]
class SCR_CanCharacterLeaveLadderCondition : SCR_AvailableActionCondition
{
	/*
	[Attribute("0")]
	protected bool m_bIsAvailableInVehicle;
	*/

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		CharacterAnimationComponent animComp = data.GetAnimationComponent();
		if (!animComp)
			return false;
		// Command ladder is present only when character is using ladder
		CharacterCommandLadder ladderCMD = animComp.GetCommandHandler().GetCommandLadder();
		if (!ladderCMD)
			return false;
		int lrExitState = ladderCMD.CanExitLR();
		// TODO: once we can differentiate between left and right filter, we should diffrentiate the result ( now it shows A/D to Leave ladder )
		if (lrExitState & 0x1 || lrExitState & 0x2)
		{
			//Print("Can exit right");
			return GetReturnResult(true);
		}

		return GetReturnResult(false);
	}
};
