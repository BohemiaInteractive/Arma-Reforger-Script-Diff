[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingGroupLeader : SCR_VotingReferendum
{
	protected SCR_AIGroup m_Group;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		if (!m_Group)
			return false;

		if (value == m_Group.GetLeaderID())
			return false;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!PlayerManager)
			return false;

		if (isOngoing && !playerManager.IsPlayerConnected(value))
			return false;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup localPlayerGroup = groupsManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
		if (!localPlayerGroup || localPlayerGroup != m_Group)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerCount()
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return 1;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(m_iValue);
		if (!playerGroup)
			return 1;

		return Math.Max(playerGroup.GetPlayerCount(), 1);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSendNotification(int value)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(value);
		if (!playerGroup)
			return false;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		SCR_AIGroup localPlayerGroup = groupsManager.GetPlayerGroup(localPlayerId);
		if (!localPlayerGroup)
			return false;

		return (localPlayerGroup == playerGroup);
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (m_Group)
		{
			m_Group.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
			m_Group.GetOnPlayerLeaderChanged().Remove(OnLeaderChanged);
		}

		if (winner == DEFAULT_VALUE)
			return;

		if (Replication.IsClient())
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup playerGroup = groupsManager.GetPlayerGroup(winner);
		if (!playerGroup)
			return;

		playerGroup.SetGroupLeader(winner);
	}

	//------------------------------------------------------------------------------------------------
	override void InitFromTemplate(SCR_VotingBase template, int startingPlayerID, int value, float remainingDuration)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManager)
			m_Group = groupsManager.GetPlayerGroup(value);

		if (m_Group)
		{
			m_Group.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
			m_Group.GetOnPlayerLeaderChanged().Insert(OnLeaderChanged);
		}

		super.InitFromTemplate(template, startingPlayerID, value, remainingDuration);
	}

	//------------------------------------------------------------------------------------------------
	//! Player removed from group, end the voting if removed player was the subject of the vote
	//! \param[in] aiGroup
	//! \param[in] playerId
	void OnPlayerRemoved(SCR_AIGroup aiGroup, int playerId)
	{
		if (aiGroup != m_Group || playerId != m_iValue)
			return;

		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return;

		votingManager.EndVoting(m_Type, m_iValue, EVotingOutcome.FORCE_FAIL);
	}

	//------------------------------------------------------------------------------------------------
	//! Group leader changed, end the voting if the new group leader is the subject of the vote
	//! \param[in] groupId
	//! \param[in] playerId
	void OnLeaderChanged(int groupId, int playerId)
	{
		if (groupId != m_Group.GetGroupID() || playerId != m_iValue)
			return;

		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			return;

		votingManager.EndVoting(m_Type, m_iValue, EVotingOutcome.FORCE_FAIL);
	}
}