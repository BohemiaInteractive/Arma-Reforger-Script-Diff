//! Base class for voting types.
//! Do not expose objects of this class outside of SCR_VotingManagerComponent!
//! Other systems should *always* go through the manager.
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingBase
{
	[Attribute("0", UIWidgets.ComboBox, "Type of the vote.", enums: ParamEnumArray.FromEnum(EVotingType))]
	protected EVotingType m_Type;
	
	[Attribute(desc: "When enabled, values are considered player IDs (e.g., a vote to KICK player).")]
	protected bool m_bIsValuePlayerID;

	[Attribute(defvalue: "1", desc: "Should vote be canceled when subject leaves the server.")]
	protected bool m_bCancelWhenSubjectLeavesTheServer;
	
	[Attribute("120", desc: "The voting will end after this duration (in seconds) and outcome will be evaluated.")]
	protected float m_fDuration;
	
	[Attribute("0.5", UIWidgets.Slider, "When percentual number of votes for single value is *larger* than this threshold, the voting will instantly end.\ne.g., in a session with 8 players and threshold 0.5, 5 votes are needed to win.", params: "0 1 0.01")]
	protected float m_fThreshold;
	
	[Attribute("0.5", UIWidgets.Slider, "Winner can be declared only if this percentage of players participated in the vote.\nEvaluated together with 'Min Votes', at least one has to pass.", params: "0 1 0.01")]
	protected float m_iMinParticipation;
	
	[Attribute("3", desc: "Winner can be declare only if at least this amount of players participated in the vote.\nEvaluated together with 'Min Participation', at least one has to pass.")]
	protected int m_iMinVotes;
	
	[Attribute(desc: "Visual representation of the voting.")]
	protected ref SCR_VotingUIInfo m_Info;
	
	[Attribute("0", desc: "Delay in seconds between when the player joined the server and when they are able to start this specific vote. 0 means this is ignored", params: "0 inf")]
	protected int m_iJoinServerVoteCooldown;
	
	[Attribute("0", desc: "If the server started fresh / restarted and players join it will check if the server was active for longer than this time (in seconds). If not it will not set the ServerVoteDelay time. Ignored if 0.", params: "0 inf")]
	protected int m_iServerRuntimeIgnoreJoinServerCooldown;
	
	[Attribute("0", desc: "Cooldown in seconds between instantiating votes. If a player has started a vote than they will no longer be able to do the same vote until the time has passed. Ignored if 0.", params: "0 inf")]
	protected int m_iVoteCooldownTime;
	
	[Attribute("0", desc: "When true, the subject of voting has voting timer always enabled during the voting progress.")]
	protected bool m_bAlwaysDisplayVoteSubjectVotingTimer;
	
	protected float m_fVoteCooldownTimeStamp = -1;
	protected bool m_bHasInitiatedVotingLocallyOnce;
	
	static const int DEFAULT_VALUE = -1;
	//! Value of the vote, that is used when vote ends in an alternative solution
	static const int ALTERNATIVE_VALUE = -2;
	
	protected int m_iVoteAuthorId;
	protected int m_iLocalValue = DEFAULT_VALUE;
	
	//~ How many players have voted in favour
	protected int m_iCurrentVoteCount;
	
	//~ Keep track of who voted (server only)
	protected ref array<int> m_aPlayersVoted_Server = {};
		
	//------------------------------------------------------------------------------------------------
	//! A player has voted to approve (server only)
	//! \param[in] playerID Id of player who voted
	//! \return True if player had not yet voted to approve
	bool AddPlayerVotedServer(int playerID)
	{
		if (m_aPlayersVoted_Server.Contains(playerID))
			return false;
		
		m_aPlayersVoted_Server.Insert(playerID);
		SetCurrentVoteCount(m_aPlayersVoted_Server.Count());
		
		//~ Vote debug
		Print("VOTING SYSTEM - Player '" + playerID + "' approved vote | Vote Type: '" + typename.EnumToString(EVotingType, m_Type) + "' | Vote value: '" + m_iLocalValue + "' | Count (" + GetCurrentVoteCount() + "/" + GetVoteCountRequired() + ")", LogLevel.DEBUG);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! A player has voted to remove or abstain (server only)
	//! \param[in] playerID Id of player who voted to remove or abstain
	//! \return True if player had previously voted to approve and the vote was removed
	bool RemovePlayerVotedServer(int playerID)
	{
		if (!m_aPlayersVoted_Server.Contains(playerID))
			return false;
		
		m_aPlayersVoted_Server.RemoveItem(playerID);
		SetCurrentVoteCount(m_aPlayersVoted_Server.Count());
		
		//~ Vote debug
		Print("VOTING SYSTEM - Player '" + playerID + "' removed vote | Vote Type: '" + typename.EnumToString(EVotingType, m_Type) + "' | Vote value: '" + m_iLocalValue + "' | Count (" + GetCurrentVoteCount() + "/" + GetVoteCountRequired() + ")", LogLevel.DEBUG);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the current amount of votes this vote has
	//! \param[in] currentVoteCount New current vote amount
	void SetCurrentVoteCount(int currentVoteCount)
	{
		m_iCurrentVoteCount = currentVoteCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Current total votes the vote has received
	int GetCurrentVoteCount()
	{
		return m_iCurrentVoteCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return The voting delay cooldown when a player joins the server. How long 
	int GetJoinServerVoteCooldown()
	{
		return m_iJoinServerVoteCooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get the server vote delay ignore time. It will not set a server vote delay time when the player joins if the server has started less seconds ago than this
	int GetServerRuntimeIgnoreJoinServerCooldown()
	{
		return m_iServerRuntimeIgnoreJoinServerCooldown;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get the cooldown amount between voting
	int GetVoteCooldownTime()
	{
		return m_iVoteCooldownTime;
	}

	//------------------------------------------------------------------------------------------------
	//! \return playerId of the player who started the vote
	int GetAuthorId()
	{
		return m_iVoteAuthorId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Return if the Vote has a cooldown set. Will use the join cooldown if not yet initiated voting
	bool HasCooldown()
	{
		if (!m_bHasInitiatedVotingLocallyOnce)
			return GetJoinServerVoteCooldown() > 0;
		
		return GetVoteCooldownTime() > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Get the last cooldown (Either when joining server or last voted) 
	float GetLocalCooldownTimeStamp()
	{
		return m_fVoteCooldownTimeStamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the time stamp for cooldowns for this voting
	//! param[in] New Time stamp to set
	void SetLocalCooldownTimeStamp(float newTimeStamp)
	{
		m_fVoteCooldownTimeStamp = newTimeStamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return True if a player has initiated voted locally at least once for this vote
	bool HasInitiatedVotingLocallyOnce()
	{
		return m_bHasInitiatedVotingLocallyOnce;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set Has initiated voting locally at least once to true
	void SetHasInitiatedVotingLocallyOnce()
	{
		m_bHasInitiatedVotingLocallyOnce = true;
	}
	 
	//------------------------------------------------------------------------------------------------
	//--- Public, server

	//------------------------------------------------------------------------------------------------
	//! Set vote of given player.
	//! \param[in] playerID Player ID
	//! \param[in] value Cast vote
	void SetVote(int playerID, int value = DEFAULT_VALUE);

	//------------------------------------------------------------------------------------------------
	//! Remove vote of given player.
	//! \param[in] playerID Player ID
	//! \return True if the voting should be cancelled
	bool RemoveVote(int playerID);

	//------------------------------------------------------------------------------------------------
	//!
	//! Remove all votes for given value.
	//! \param[in] value Value
	//! \return
	bool RemoveValue(int value);
	
	//------------------------------------------------------------------------------------------------
	//! Check if notification can be shown for this voting
	//! \param[in] value Voting value
	//! \return True if can send notification for this voting type
	bool CanSendNotification(int value)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Evaluate if the voting can end.
	//! \param[out] outcome Outcome of the vote (used only when returned value is true)
	//! \return True if it can end
	bool Evaluate(out EVotingOutcome outcome)
	{
		return m_fDuration <= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Get winner of the vote.
	//! \return Winning value
	int GetWinner()
	{
		return DEFAULT_VALUE;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if vote should be cancled when subject of the server leaves the server
	bool ShouldCancelWhenSubjectLeavesTheServer()
	{
		return m_bCancelWhenSubjectLeavesTheServer;
	}

	//------------------------------------------------------------------------------------------------
	//! Event called when the voting ends.
	//! \param[in] value Voting value
	//! \param[in] winner Winning value
	//!
	void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE);

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \return
	int GetPlayerVote(int playerID)
	{
		return DEFAULT_VALUE;
	}	

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Public, anywhere
	//------------------------------------------------------------------------------------------------
	//! Check if the voting is available in the current world.
	//! \param[in] value Potential voting value
	//! \param[in] isOngoing True if the voting was already initiated
	//! \return True when available
	bool IsAvailable(int value, bool isOngoing)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the voting is matching given params.
	//! \param[in] type Voting type
	//! \param[in] value Voting value
	//! \return
	bool IsMatching(EVotingType type, int value = DEFAULT_VALUE);

	//------------------------------------------------------------------------------------------------
	//!
	void Log();

	//------------------------------------------------------------------------------------------------
	//! Get name of the value.
	//! \param[in] value
	//! \return Value name
	string GetValueName(int value)
	{
		if (m_bIsValuePlayerID && value > 0)
			return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(value);
		else
			return value.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! Get voting type.
	//! \return Type
	EVotingType GetType()
	{
		return m_Type;
	}

	//------------------------------------------------------------------------------------------------
	//! Get voting value.
	//! Applicable only if the voting targets specific value, e.g., is referendum about certain outcome.
	//! \return Value
	EVotingType GetValue()
	{
		return DEFAULT_VALUE;
	}

	//------------------------------------------------------------------------------------------------
	//! Get voting UI representation
	//! \return UI info
	SCR_VotingUIInfo GetInfo()
	{
		return m_Info;
	}
	//------------------------------------------------------------------------------------------------
	//! \return Remaining time of the voting in seconds
	float GetRemainingDuration()
	{
		return m_fDuration;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the voting is about player IDs.
	//! \return True when it's about player IDs
	bool IsValuePlayerID()
	{
		return m_bIsValuePlayerID;
	}

	//------------------------------------------------------------------------------------------------
	//! Save local vote.
	//! It's purely informative, does not affect voting outcome!
	//! \param[in] value Voted value
	void SetVoteLocal(int value)
	{
		m_iLocalValue = value;
	}

	//------------------------------------------------------------------------------------------------
	//! Remove local vote.
	//! It is purely informative and does not affect voting outcome!
	void RemoveVoteLocal()
	{
		m_iLocalValue = DEFAULT_VALUE;
	}

	//------------------------------------------------------------------------------------------------
	//! Get local vote.
	//! It is purely informative and does not affect voting outcome!
	//! \return Voted value
	int GetLocalVote()
	{
		return m_iLocalValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetAlwaysDisplayVoteSubjectVotingTimer()
	{
		return m_bAlwaysDisplayVoteSubjectVotingTimer;
	}

	//------------------------------------------------------------------------------------------------
	//! Periodically update the voting.
	//! \param[in] timeSlice Time since the last update
	void Update(float timeSlice)
	{
		m_fDuration = Math.Max(m_fDuration - timeSlice, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialise voting from given template.
	//! \param[in] template Source template
	//! \param[in] startingPlayerID id of a player who started the vote
	//! \param[in] value Target value
	//! \param[in] remainingDuration Remaining time until the voting ends in seconds (-1 to use default)
	void InitFromTemplate(SCR_VotingBase template, int startingPlayerID, int value, float remainingDuration)
	{
		m_Type = template.m_Type;
		m_fThreshold = template.m_fThreshold;
		m_bIsValuePlayerID = template.m_bIsValuePlayerID;
		m_iMinParticipation = Math.Min(template.m_iMinParticipation, template.m_fThreshold); //--- Min participation cannot be stricter than threshold
		m_iMinVotes = template.m_iMinVotes;
		m_iVoteAuthorId = startingPlayerID;
		m_bCancelWhenSubjectLeavesTheServer = template.m_bCancelWhenSubjectLeavesTheServer;
		m_iVoteCooldownTime = template.m_iVoteCooldownTime;
		m_bAlwaysDisplayVoteSubjectVotingTimer = template.m_bAlwaysDisplayVoteSubjectVotingTimer;
		
		if (remainingDuration == -1)
			m_fDuration = template.m_fDuration;
		else
			m_fDuration = remainingDuration;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get total player count of players that are valid to vote for the issue
	//! \return Total count
	int GetPlayerCount()
	{
		return Math.Max(GetGame().GetPlayerManager().GetPlayerCount(), 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get total players needed to make sure the vote is successfull
	//! \return Total votes needed
	int GetVoteCountRequired()
	{
		float playersRequired = GetPlayerCount() * m_fThreshold;
		int playersRequiredInt = Math.Ceil(playersRequired);
		
		//~ Vote succeeds when vote count is GREATER than threshold. If players requered is == to playersRequired ceil than it means that there is 1 more player needed for the vote to succeed as otherwise the vote is equal
		if (playersRequired == playersRequiredInt)
			playersRequiredInt++;
		
		if (playersRequiredInt < m_iMinVotes)
			return m_iMinVotes;
		
		return playersRequiredInt;
	}
		
	//------------------------------------------------------------------------------------------------
	protected bool EvaluateParticipation(int voteCount)
	{
		return
			/*
				Absolute player limit
				With m_iMinVotes = 3, at least 3 players have to cast a vote. 
				If there are fewer than 3 players, total amount of players will be used instead (which means the vote has to be unanimous)
				Examples:
					Players		Votes needed
					1			1
					2			2
					3			3
					4			3
					5			3
					...			...
			*/
			voteCount >= Math.Min(m_iMinVotes, GetPlayerCount())
			/*
				Relative player limit
				With m_iMinParticipation = 0.5 (50%), at least half of players need to cast a vote.
				Examples:
					Players		Votes needed
					1			1
					2			1
					3			2
					4			2
					5			3
					...			...
			*/
			&& (float)(voteCount / GetPlayerCount()) >= m_iMinParticipation;
	}
};

//! Referendum about specific outcome.
//! Can have target value, e.g., player ID of whoever should be kicked.
//! Multiple votings of the same type can exist at the same type, e.g., KICK player 1 and KICK player 2.
//! The voting succeeds when proportion of votes is *larger* than given threshold and there was sufficient participation.
//! If time runs out, the voting is invalid.
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingReferendum : SCR_VotingBase
{
	protected ref set<int> m_aPlayerIDs = new set<int>();
	protected int m_iValue;
	
	//------------------------------------------------------------------------------------------------
	protected float GetRatio()
	{
		return (m_aPlayerIDs.Count() / GetPlayerCount());
	}

	//------------------------------------------------------------------------------------------------
	override bool AddPlayerVotedServer(int playerID)
	{
		if (!super.AddPlayerVotedServer(playerID))
			return false;

		PlayerManager playerMgr = GetGame().GetPlayerManager();
		string cliVal;
		if (playerMgr && System.GetCLIParam("logVoting", cliVal))
		{
			string serverLogMessage;
			if (m_bIsValuePlayerID)
				serverLogMessage += ", where the vote subject is player '" + playerMgr.GetPlayerName(m_iValue) + "' (with player id = " + m_iValue + ")";
			else
				serverLogMessage += ", with vote value = " + m_iValue;

			Print("VOTING SYSTEM - Player '" + playerMgr.GetPlayerName(playerID) + "' (with player id = " + playerID + ") voted in favor of the vote type = " + typename.EnumToString(EVotingType, m_Type) + serverLogMessage, LogLevel.NORMAL);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RemovePlayerVotedServer(int playerID)
	{
		if (!super.RemovePlayerVotedServer(playerID))
			return false;

		PlayerManager playerMgr = GetGame().GetPlayerManager();
		string cliVal;
		if (playerMgr && System.GetCLIParam("logVoting", cliVal))
		{
			string serverLogMessage;
			if (m_bIsValuePlayerID)
				serverLogMessage += ", where the vote subject is player '" + playerMgr.GetPlayerName(m_iValue) + "' (with player id = " + m_iValue + ")";
			else
				serverLogMessage += ", with vote value = " + m_iValue;

			Print("VOTING SYSTEM - Player '" + playerMgr.GetPlayerName(playerID) + "' (with player id = " + playerID + ") voted against or retracted his vote for the vote type = " + typename.EnumToString(EVotingType, m_Type) + serverLogMessage, LogLevel.NORMAL);
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetVote(int playerID, int value = DEFAULT_VALUE)
	{
		m_aPlayerIDs.Insert(playerID);
	}

	//------------------------------------------------------------------------------------------------
	override bool RemoveVote(int playerID)
	{
		int index = m_aPlayerIDs.Find(playerID);
		if (index >= 0)
		{
			m_aPlayerIDs.Remove(index);
			return m_aPlayerIDs.IsEmpty();
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool RemoveValue(int value)
	{
		return m_bIsValuePlayerID && m_iValue == value;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsMatching(EVotingType type, int value = DEFAULT_VALUE)
	{
		return m_Type == type && m_iValue == value;
	}

	//------------------------------------------------------------------------------------------------
	override bool Evaluate(out EVotingOutcome outcome)
	{
		return super.Evaluate(outcome) || (GetRatio() > m_fThreshold && EvaluateParticipation(m_aPlayerIDs.Count()));
	}

	//------------------------------------------------------------------------------------------------
	override int GetWinner()
	{
		if (GetRatio() > m_fThreshold && EvaluateParticipation(m_aPlayerIDs.Count()))
			return m_iValue; //--- Passed the threshold and minimum participation
		else
			return DEFAULT_VALUE; //--- Expired, didn't pass
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerVote(int playerID)
	{
		if (m_aPlayerIDs.Contains(playerID))
			return m_iValue;
		else
			return DEFAULT_VALUE;
	}

	//------------------------------------------------------------------------------------------------
	override int GetValue()
	{
		return m_iValue;
	}

	//------------------------------------------------------------------------------------------------
	override void Log()
	{
		PrintFormat("%1 (m_Type = %2, m_iValue = %3, m_iLocalValue = %3)", Type(), typename.EnumToString(EVotingType, m_Type), GetValueName(m_iValue), GetValueName(m_iLocalValue));
		foreach (int playerID: m_aPlayerIDs)
		{
			PrintFormat("    '%1'", GetGame().GetPlayerManager().GetPlayerName(playerID));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void InitFromTemplate(SCR_VotingBase template, int startingPlayerID, int value, float remainingDuration)
	{
		m_iValue = value;
		super.InitFromTemplate(template, startingPlayerID, value, remainingDuration);
	}
}

//! Election of specific type.
//! Once it starts, players can vote for multiple values.
//! The voting ends when proportion of votes for single value is *larger* than threshold, or if time expires.
//! If the vote had enough participation, the value with the most votes wins, otherwise the vote fails.
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingElection : SCR_VotingBase
{
	protected ref map<int, int> m_Votes = new map<int, int>(); // <playerID, value>
	protected int m_iHighestValue = DEFAULT_VALUE;
	protected int m_iHighestCount;

	//------------------------------------------------------------------------------------------------
	protected void UpdateHighestValue()
	{
		map<int, int> tally = new map<int, int>(); // <value, count>
		m_iHighestCount = 0;
		set<int> winners = new set<int>();
		foreach (int playerID, int value: m_Votes)
		{
			int count;
			if (tally.Find(value, count))
				count++;
			else
				count = 1;
			
			tally.Set(value, count);
			
			if (count > m_iHighestCount)
			{
				m_iHighestCount = count;
				winners.Clear();
				winners.Insert(value);
			}
			else if (count == m_iHighestCount)
			{
				winners.Insert(value);
			}
		}
		
		if (winners.IsEmpty())
			m_iHighestValue = DEFAULT_VALUE;
		else
			//--- In case of multiple leaders, pick one by random
			m_iHighestValue = winners[Math.RandomInt(0, winners.Count())];
	}

	//------------------------------------------------------------------------------------------------
	override void SetVote(int playerID, int value = DEFAULT_VALUE)
	{
		m_Votes.Set(playerID, value);
		UpdateHighestValue();
	}

	//------------------------------------------------------------------------------------------------
	override bool RemoveVote(int playerID)
	{
		m_Votes.Remove(playerID);
		UpdateHighestValue();
		return m_Votes.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override bool RemoveValue(int value)
	{
		for (int i = m_Votes.Count() - 1; i >= 0; i--)
		{
			if (m_Votes.GetElement(i) == value)
				m_Votes.RemoveElement(i);
		}
		UpdateHighestValue();
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsMatching(EVotingType type, int value = DEFAULT_VALUE)
	{
		return m_Type == type;
	}

	//------------------------------------------------------------------------------------------------
	override bool Evaluate(out EVotingOutcome outcome)
	{
		float ratio = (m_iHighestCount / GetPlayerCount());
		return super.Evaluate(outcome) || (ratio > m_fThreshold && EvaluateParticipation(m_Votes.Count()));
	}

	//------------------------------------------------------------------------------------------------
	override int GetWinner()
	{
		if (EvaluateParticipation(m_Votes.Count()))
			return m_iHighestValue;
		else
			return DEFAULT_VALUE; //--- Expired, didn't pass
	}

	//------------------------------------------------------------------------------------------------
	override int GetPlayerVote(int playerID)
	{
		int value;
		if (m_Votes.Find(playerID, value))
			return value;
		else
			return DEFAULT_VALUE;
	}

	//------------------------------------------------------------------------------------------------
	override void Log()
	{
		PrintFormat("%1 (m_Type = %2, m_iLocalValue = %3)", Type(), typename.EnumToString(EVotingType, m_Type), GetValueName(m_iLocalValue));
		PrintFormat("    Leading value: %1 with %2 votes", m_iHighestValue, m_iHighestCount);
		foreach (int playerID, int value: m_Votes)
		{
			PrintFormat("    %1 votes for  %2", GetGame().GetPlayerManager().GetPlayerName(playerID), GetValueName(value));
		}
	}
}
