[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_VoteGameMasterCondition : SCR_BaseEditorHintCondition
{
	[Attribute("0", UIWidgets.ComboBox, "Type of tracked vote.", enums: ParamEnumArray.FromEnum(EVotingType))]
	protected EVotingType m_Type;
	
	[Attribute()]
	protected bool m_bShowInPlayerList;
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingStart(EVotingType type, int value)
	{
		if (type == m_Type && IsPlayerListMenu())
			Activate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		if (type == m_Type && IsPlayerListMenu())
			Deactivate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVoteLocal(EVotingType type, int value)
	{
		if (type == m_Type && IsPlayerListMenu())
			Deactivate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerListMenu(bool isOpened)
	{
		if (!isOpened)
			return;
		
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (votingManager && votingManager.IsVoting(m_Type))
			Activate();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPlayerListMenu()
	{
		if (!m_bShowInPlayerList)
			return true;
		
		MenuManager menuManager = GetGame().GetMenuManager();
		return !menuManager || menuManager.FindMenuByPreset(ChimeraMenuPreset.PlayerListMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return;
		
		votingManager.GetOnVotingStart().Insert(OnVotingStart);
		votingManager.GetOnVotingEnd().Insert(OnVotingEnd);
		votingManager.GetOnVoteLocal().Insert(OnVoteLocal);
		votingManager.GetOnRemoveVoteLocal().Insert(OnVotingStart);
		
		SCR_PlayerListMenu.GetOnPlayerListMenu().Insert(OnPlayerListMenu);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return;
		
		votingManager.GetOnVotingStart().Remove(OnVotingStart);
		votingManager.GetOnVotingEnd().Remove(OnVotingEnd);
		votingManager.GetOnVoteLocal().Remove(OnVoteLocal);
		votingManager.GetOnRemoveVoteLocal().Remove(OnVotingStart);
		
		SCR_PlayerListMenu.GetOnPlayerListMenu().Remove(OnPlayerListMenu);
	}
}
