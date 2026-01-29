class SCR_GroupLeaderVolunteerDialogUI : DialogUI
{
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return;

		int playerID = groupController.GetPlayerID();
		groupController.RequestGroupLeaderVote(playerID);

		super.OnConfirm();
	}
}