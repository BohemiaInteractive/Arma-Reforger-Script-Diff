[ComponentEditorProps(category: "GameScripted/Voting", description: "")]
class SCR_VoterComponentClass : ScriptComponentClass
{
}

class SCR_VoterComponent : ScriptComponent
{
	protected PlayerController m_PlayerController;
	
	//------------------------------------------------------------------------------------------------
	//--- Public, anywhere
	//! Get local instance of this component.
	//! \return Local voter component
	static SCR_VoterComponent GetInstance()
	{
		if (GetGame().GetPlayerController())
			return SCR_VoterComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_VoterComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Instantly cast vote to the only ongoing voting.
	//! Ignored if number of votings is not exactly 1.
	static void InstantVote()
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		SCR_VoterComponent voterComponent = GetInstance();
		if (!voterComponent)
			return;
		
		array<EVotingType> votingTypes = {};
		array<int> votingValues = {};
		
		int count = manager.GetAllVotingsWithValue(votingTypes, votingValues, false, true);
		
		for (int i = count - 1; i >= 0; i--)
		{
			if (manager.HasAbstainedLocally(votingTypes[i], votingValues[i]))
			{
				votingTypes.Remove(i);
				votingValues.Remove(i);
				continue;
			}
			
			if (manager.IsLocalVote(votingTypes[i], votingValues[i]))
			{
				votingTypes.Remove(i);
				votingValues.Remove(i);
				continue;
			}
		}
		
		if (votingTypes.Count() == 1)
			voterComponent.Vote(votingTypes[0], votingValues[0]);

	}
	
	//------------------------------------------------------------------------------------------------
	//! Instantly cast remove vote to the only ongoing voting and will set self to abstaining as well
	//! Ignored if number of votings is not exactly 1.
	static void InstantRemoveAndAbstainVote()
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		SCR_VoterComponent voterComponent = GetInstance();
		if (!voterComponent)
			return;
		
		array<EVotingType> votingTypes = {};
		array<int> votingValues = {};
		
		int count = manager.GetAllVotingsWithValue(votingTypes, votingValues, false, true);
		
		for (int i = count - 1; i >= 0; i--)
		{
			if (manager.HasAbstainedLocally(votingTypes[i], votingValues[i]))
			{
				votingTypes.Remove(i);
				votingValues.Remove(i);
				continue;
			}
			
			if (manager.IsLocalVote(votingTypes[i], votingValues[i]))
			{
				votingTypes.Remove(i);
				votingValues.Remove(i);
				continue;
			}
		}
		
		if (votingTypes.Count() == 1)
			voterComponent.RemoveVote(votingTypes[0], votingValues[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	//--- Public, owner
	//! Vote!
	//! \param[in] type Type of the vote
	//! \param[in] value Voted value, depends on the type (e.g., for KICK it's player ID)
	void Vote(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		//~ Trying to start a vote but there is an active cooldown timer for the player
		if (!manager.IsVoting(type, value) && manager.GetCurrentVoteCooldownForLocalPlayer(type) > 0)
			return;
		
		manager.PreVoteLocal(type, value);
		
		Rpc(VoteServer, type, value);
		manager.VoteLocal(type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove previously cast vote.
	//! \param[in] type Type of the vote
	//! \param[in] value Voted value, depends on the type (e.g., for KICK it's player ID)
	void RemoveVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		//~ Abstain vote (Local only)
		AbstainVote(type, value);
		
		Rpc(RemoveVoteServer, type, value);
		manager.RemoveVoteLocal(type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Player abstained from voting. Eg: Did not vote yes or removes vote yes
	//! \param[in] type Type of the vote
	//! \param[in] value Voted value, depends on the type (e.g., for KICK it's player ID)
	void AbstainVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (!manager)
			return;
		
		//~ Vote is active so remove it
		if (manager.IsLocalVote(type, value))
		{
			Rpc(RemoveVoteServer, type, value);
			manager.RemoveVoteLocal(type, value);
		}
	
		manager.AbstainVoteLocally(type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if player did cast the vote.
	//! \param[in] type Type of the vote
	//! \param[in] value Voted value, depends on the type (e.g., for KICK it's player ID)
	//! \return True if the vote was cast
	bool DidVote(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			return manager.IsLocalVote(type, value);
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if player abstained from voting
	//! \param[in] type Type of the vote
	//! \param[in] value Voted value, depends on the type (e.g., for KICK it's player ID)
	//! \return True if the player abstained
	bool HasAbstained(EVotingType type, int value = SCR_VotingBase.DEFAULT_VALUE)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			return manager.HasAbstainedLocally(type, value);
		else
			return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//--- Protected, server
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void VoteServer(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			manager.Vote(m_PlayerController.GetPlayerId(), type, value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RemoveVoteServer(EVotingType type, int value)
	{
		SCR_VotingManagerComponent manager = SCR_VotingManagerComponent.GetInstance();
		if (manager)
			manager.RemoveVote(m_PlayerController.GetPlayerId(), type, value);
	}
	
	//------------------------------------------------------------------------------------------------
	//--- Overrides
	override void OnPostInit(IEntity owner)
	{
		m_PlayerController = PlayerController.Cast(owner);
		if (!m_PlayerController)
		{
			Debug.Error2("SCR_VoterComponent", "SCR_VoterComponent must be attached to PlayerController!");
			return;
		}
	}
}
