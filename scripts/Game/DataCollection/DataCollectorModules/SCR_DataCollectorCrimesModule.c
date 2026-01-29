[BaseContainerProps()]
class SCR_DataCollectorCrimesModule : SCR_DataCollectorModule
{
	//------------------------------------------------------------------------------------------------
	/*!
	List of IDs from DataCollector
	*/
	protected ref array<int> m_aPlayerIDs = {};

	//------------------------------------------------------------------------------------------------
	/*!
	Next index of player to evaluate. We do them sequentially in order to split them in frames
	*/
	protected int m_iNextIndex;

	//------------------------------------------------------------------------------------------------
	/*!
	after LAST_ACTION_THRESHOLD have passed since last action, evaluate buffer
	*/
	[Attribute("3", UIWidgets.Slider, desc: "Seconds after last action to evaluate buffer")]
	protected float m_fLatestActionThreshold;

	//------------------------------------------------------------------------------------------------
	/*!
	after TIME_ACCUMULATING_THRESHOLD seconds have passed, evaluate buffer
	*/
	[Attribute("5", UIWidgets.Slider, desc: "Max seconds of accumulating actions before evaluating them")]
	protected float m_fMaxTimeAccumulationThreshold;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Duration of LightBan bans. We use the index as the streak of lightbans performed to this player
	*/
	[Attribute()]
	protected ref array<int> m_aLightBanPunishments;
	
	//------------------------------------------------------------------------------------------------
	/*!
	How many minutes of playtime need to pass to consider this player not a reincident
	*/
	[Attribute("1800", UIWidgets.Slider, desc: "Seconds for reincidency of light severity")]
	protected int m_iSecondsOfReincidencyLightBan;

	//------------------------------------------------------------------------------------------------
	/*!
	Duration of HeavyBans. We use the index as the streak of heavyBans performed to this player
	*/
	[Attribute()]
	protected ref array<int> m_aHeavyBanPunishments;
	
	//------------------------------------------------------------------------------------------------
	/*!
	How many minutes of playtime need to pass to consider this player not a reincident
	*/
	[Attribute("14400", UIWidgets.Slider, desc: "Seconds for reincidency of heavy severity")]
	protected int m_iSecondsOfReincidencyHeavyBan;

	[Attribute("1", UIWidgets.CheckBox, desc: "Are War Crimes detection enabled?")]
	protected bool m_bWarCrimesEnabled;

	[Attribute("1", UIWidgets.CheckBox, desc: "Is the proportionality principle enabled?")]
	protected bool m_bWarCrimesProportionalityPrincipleEnabled;

	[Attribute("2", UIWidgets.Slider, desc: "Negative proportionality points for killing a friendly human-controlled soldier")]
	protected float MODIFIER_PROPORTIONALITY_FRIENDLY_KILLS;
	
	[Attribute("1", UIWidgets.Slider, desc: "Negative proportionality points for killing a friendly AI-controlled soldier")]
	protected float MODIFIER_PROPORTIONALITY_FRIENDLY_AI_KILLS;
	
	[Attribute("1.5", UIWidgets.Slider, desc: "Negative proportionality points for killing an enemy AI-controlled soldier")]
	protected float MODIFIER_PROPORTIONALITY_AI_KILLS;
	
	[Attribute("3", UIWidgets.Slider, desc: "Positive proportionality points for killing an enemy human-controlled soldier")]
	protected float MODIFIER_PROPORTIONALITY_KILLS;
	
	//~ Used in voting to make sure there is a min ban duration (in seconds)
	static const int MIN_AUTO_BAN_DURATION = 300;
	
	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int playerID)
	{
		super.OnPlayerAuditSuccess(playerID, stuff);

		//If war crimes are not enabled, the module doesn't need to do anything. Keep the array of players empty and just return
		if (!m_bWarCrimesEnabled)
			return;
		
		m_aPlayerIDs.Insert(playerID);
		
		//Reset time out of the player in case they were previously disconnected as a punishment
		
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerID, false);
		if (!playerData)
			return;
		
		playerData.SetTimeOut(0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerID, IEntity controlledEntity = null)
	{
		super.OnPlayerDisconnected(playerID, controlledEntity);

		int i = m_aPlayerIDs.Find(playerID);

		//if player not found in the array, return
		if (i == -1)
			return;

		//Evaluate player no matter what, because they are leaving the server
		EvaluatePlayerCrimes(playerID, false);

		//Update their acceleration
		UpdateCriminalAcceleration(playerID);

		//Remove player from the array
		m_aPlayerIDs.RemoveOrdered(i);

		//Adjust index if necessary
		if (i < m_iNextIndex)
			m_iNextIndex--;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd()
	{
		for (int i = 0; i < m_aPlayerIDs.Count(); i++)
		{
			CheckPlayer(m_aPlayerIDs[i]);
		}
		m_aPlayerIDs.Clear();
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeTick)
	{
		if (m_aPlayerIDs.IsEmpty())
			return;

		m_fTimeSinceUpdate += timeTick;

		if (m_fTimeSinceUpdate < m_fUpdatePeriod)
			return;

		if (!m_aPlayerIDs.IsIndexValid(m_iNextIndex))
		{
			m_iNextIndex = 0;
			m_fTimeSinceUpdate = 0;
			return;
		}

		CheckPlayer(m_aPlayerIDs[m_iNextIndex]);

		m_iNextIndex++;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckPlayer(int playerId)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerId, false);
		if (!playerData)
			return;

		//Are we accumulating actions? If not, that's it for now. Process stats and reset
		int accumulationTick = playerData.GetAccumulatedActionsTick();
		if (accumulationTick == 0)
		{
			ProcessTemporalStats(playerData);
			return;
		}

		//It's been more than m_fLastActionThreshold seconds since last action or more than m_fMaxTimeAccumulationThreshold since started accumulating actions? If not, keep accumulating
		int currentTick = System.GetTickCount();
		int latestActionTick = playerData.GetLatestActionTick();
		if (currentTick - latestActionTick < m_fLatestActionThreshold * 1000 && currentTick - accumulationTick < m_fMaxTimeAccumulationThreshold * 1000)
			return;

		//Evaluation of war crimes
		EvaluatePlayerCrimes(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! First decrease the score up until now by doing Time * DecreasePerMinute
	//! Second add to the score given the current crimes: PointsOfCrime * CRIME_ACCELERATION_FACTOR
	protected void EvaluatePlayerCrimes(int playerId, bool evaluatePunishment = true)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerId, false);
		if (!playerData)
			return;

		//Calculate decreasing of criminal score based on time lapsed
		float decreasingOfScore = (System.GetTickCount() - playerData.GetLatestCriminalScoreUpdateTick()) * 0.001 * SCR_PlayerDataConfigs.GetInstance().GetScoreDecreasePerSecond();

		///////////////////////////////////////////
		// Analsing warcrime_harming_friendlies: //
		///////////////////////////////////////////

		float harmingFriendliesScore = EvaluateHarmingFriendlies(playerData) * playerData.GetStat(SCR_EDataStats.CRIME_ACCELERATION);

		///////////////////////////////
		// Done analysing war crimes //
		///////////////////////////////

		//Process temporal stats
		ProcessTemporalStats(playerData);

		//Update score
		UpdateCriminalScore(playerData, harmingFriendliesScore, decreasingOfScore);
		
		float currentScore = playerData.GetCriminalScore();
		
		if (harmingFriendliesScore > 0)
			Print("Player with id " + playerId + " got " + harmingFriendliesScore + " more criminal points. Current criminal score is " + currentScore, LogLevel.DEBUG);
		else
			return;

		//Kick or ban players
		if (evaluatePunishment && currentScore >= SCR_PlayerDataConfigs.GetInstance().GetScoreThreshold())
		{
			//Logic for kicking player
			KickPlayer(playerId, playerData, SCR_PlayerManagerKickReason.FRIENDLY_FIRE);
			return;
		}

		//Send educational hints to clients
		SendHints(playerId, currentScore);
	}

	//------------------------------------------------------------------------------------------------
	protected void KickPlayer(int playerId, notnull SCR_PlayerData playerData, SCR_PlayerManagerKickReason reason)
	{
		int durationInMinutes = RequestBanIfNecessary(playerId, playerData, reason);
		if (durationInMinutes == -1)
		{
			return;
		}
		else
		{
			Print("Player Manager is kicking a player with id " + playerId + ". Banning them for " + durationInMinutes + " minutes.", LogLevel.DEBUG);
			if (durationInMinutes == 0)
				ExecutePunishment(playerId, SCR_PlayerManagerKickReason.KICK, 0);
			else
				ExecutePunishment(playerId, SCR_PlayerManagerKickReason.TEMP_BAN, durationInMinutes * 60);
		}
		
		//GetGame().GetPlayerManager().KickPlayer(playerId, reason, durationInMinutes * 60);
	}

	//------------------------------------------------------------------------------------------------
	//! The difference between the types of kick or bans: heavy ban (requires backendapi ban), light ban or kick (duration difference).
	//! To choose the type of kick or ban:
	//! 1. Compare the current acceleration against the (maxAcceleration * typeOfBanRate)
	//! 2. Then store the current SessionDuration as the "previousPunishmentDuration" field in playerData
	//! 3. Update the streak field
	//! 4. Profit
	protected int RequestBanIfNecessary(int playerId, notnull SCR_PlayerData playerData, SCR_PlayerManagerKickReason reason)
	{
		if (m_aLightBanPunishments.IsEmpty() || m_aHeavyBanPunishments.IsEmpty())
		{
			Print("Incorrect setup on DataCollectorCrimesModule: Light or Heavy ban duration array is empty. Cannot ban players!", LogLevel.ERROR);
			return 0;
		}

		//Update their session duration to now
		playerData.CalculateSessionDuration();
		
		//Set up the necessary fields
		float acceleration = playerData.GetStat(SCR_EDataStats.CRIME_ACCELERATION);
		float score = playerData.GetCriminalScore();
		
		SCR_PlayerDataConfigs config = SCR_PlayerDataConfigs.GetInstance();
		int durationInMinutes;
		
		//Should this player be banned?
		//Lightban requires an acceleration of 4.8 or more
		if (config.GetMaxAcceleration() * config.GetBanEvaluationLight() <= acceleration)
		{
			//Yes, they should. Decide between light or heavy ban now.

			//Lightban variables
			int timeDiffLightBan = playerData.GetStat(SCR_EDataStats.SESSION_DURATION) - playerData.GetStat(SCR_EDataStats.LIGHTBAN_SESSION_DURATION);
			int streakLightBan = playerData.GetStat(SCR_EDataStats.LIGHTBAN_STREAK);
			int maxStreakLightBan = m_aLightBanPunishments.Count();
			
			//Heavyban variables
			int timeDiffHeavyBan = playerData.GetStat(SCR_EDataStats.SESSION_DURATION) - playerData.GetStat(SCR_EDataStats.HEAVYBAN_SESSION_DURATION);
			int streakHeavyBan = playerData.GetStat(SCR_EDataStats.HEAVYBAN_STREAK);
			int maxStreakHeavyBan = m_aHeavyBanPunishments.Count();
			
			//If It's the first ban issued to this player recently, only light ban is enough.
			if (timeDiffLightBan > m_iSecondsOfReincidencyLightBan && timeDiffHeavyBan > m_iSecondsOfReincidencyHeavyBan)
			{
				durationInMinutes = m_aLightBanPunishments[0];
				streakLightBan = 1;

				playerData.OverrideStat(SCR_EDataStats.LIGHTBAN_STREAK, streakLightBan);
				playerData.OverrideStat(SCR_EDataStats.LIGHTBAN_SESSION_DURATION, playerData.GetStat(SCR_EDataStats.SESSION_DURATION));
			}
			else
			//Else: Not a long enough time has passed since the last ban. This means this player has repeated an offense.
			{
				//If the streak of lightban is not big enough, or the acceleration is not big enough, lightban it is
				//Heavyban requires an acceleration of 6 and a lightban streak of at least 3 with current configs
				if (!(streakLightBan >= maxStreakLightBan && config.GetMaxAcceleration() * config.GetBanEvaluationHeavy() <= acceleration))
				{
					//Light ban is enough for this player because the streak and the acceleration don't meet (both of them) the necessary conditions

					durationInMinutes = m_aLightBanPunishments[Math.Min(streakLightBan, maxStreakLightBan - 1)];
					streakLightBan++;

					playerData.OverrideStat(SCR_EDataStats.LIGHTBAN_STREAK, streakLightBan);
					playerData.OverrideStat(SCR_EDataStats.LIGHTBAN_SESSION_DURATION, playerData.GetStat(SCR_EDataStats.SESSION_DURATION));
				}
				else
				{
					//Heavy ban it is
					if (timeDiffHeavyBan > m_iSecondsOfReincidencyHeavyBan)
					{
						durationInMinutes = m_aHeavyBanPunishments[0];
						streakHeavyBan = 1;
						playerData.OverrideStat(SCR_EDataStats.HEAVYBAN_STREAK, streakHeavyBan);
					}
					else
					{
						durationInMinutes = m_aHeavyBanPunishments[Math.Min(streakHeavyBan, maxStreakHeavyBan - 1)];
						streakHeavyBan++;
					}

					playerData.OverrideStat(SCR_EDataStats.HEAVYBAN_STREAK, streakHeavyBan);
					playerData.OverrideStat(SCR_EDataStats.HEAVYBAN_SESSION_DURATION, playerData.GetStat(SCR_EDataStats.SESSION_DURATION));

					BackendApi ba = GetGame().GetBackendApi();
					if (ba)
						ExecutePunishment(playerId, SCR_PlayerManagerKickReason.BAN, durationInMinutes * 60);
						//ba.PlayerBanCreate("Heavy ban", durationInMinutes * 60, playerId);
					
					Print("Banning service is heavybanning a player with id " + playerId + ". Banning them for " + durationInMinutes + " minutes.", LogLevel.DEBUG);
					//Player has been banned. We don't want the PlayerManager to ban them on top of the banning service. So we need to set durationInMinutes to an scape value
					//This implementation should be improved in the future, but for now we can only kick them for 0 minutes using the PlayerManager since its the banning service what banned them.
					durationInMinutes = -1;
				}
			}
		}
		//Not a ban. Just a kick
		else
		{
			int timeDiff = playerData.GetStat(SCR_EDataStats.SESSION_DURATION) - playerData.GetStat(SCR_EDataStats.KICK_SESSION_DURATION);
			int streak = playerData.GetStat(SCR_EDataStats.KICK_STREAK);

			if (timeDiff > 600)
				streak = 1;
			else
				streak ++;

			durationInMinutes = 0;

			playerData.OverrideStat(SCR_EDataStats.KICK_STREAK, streak);
			playerData.OverrideStat(SCR_EDataStats.KICK_SESSION_DURATION, playerData.GetStat(SCR_EDataStats.SESSION_DURATION));
		}

		return durationInMinutes;
	}

	//------------------------------------------------------------------------------------------------
	protected float EvaluateHarmingFriendlies(notnull SCR_PlayerData playerData)
	{
		array<float> accumulatedActions = playerData.GetAccumulatedActions();
		
		float harmingAlliesPoints = accumulatedActions[SCR_EDataStats.FRIENDLY_KILLS] * MODIFIER_PROPORTIONALITY_FRIENDLY_KILLS + accumulatedActions[SCR_EDataStats.FRIENDLY_AI_KILLS] * MODIFIER_PROPORTIONALITY_FRIENDLY_AI_KILLS;
		float proportionalityPoints = 0;

		if (m_bWarCrimesProportionalityPrincipleEnabled)
			proportionalityPoints = accumulatedActions[SCR_EDataStats.KILLS] * MODIFIER_PROPORTIONALITY_KILLS + accumulatedActions[SCR_EDataStats.AI_KILLS] * MODIFIER_PROPORTIONALITY_AI_KILLS;

		if (harmingAlliesPoints <= proportionalityPoints)
			return 0;

		playerData.AddStat(SCR_EDataStats.WARCRIME_HARMING_FRIENDLIES, harmingAlliesPoints - proportionalityPoints, false);
		playerData.AddStat(SCR_EDataStats.WARCRIMES, harmingAlliesPoints - proportionalityPoints, false);

		//Single friendly kill or multiple friendly kills?
		float PointsOfCrime;

		if (accumulatedActions[SCR_EDataStats.FRIENDLY_KILLS] + accumulatedActions[SCR_EDataStats.FRIENDLY_AI_KILLS] == 1)
			PointsOfCrime = SCR_PlayerDataConfigs.GetInstance().GetCrimePointsFriendlyKill();
		else
			PointsOfCrime = SCR_PlayerDataConfigs.GetInstance().GetCrimePointsFriendlyMultiKill();

		return PointsOfCrime * (harmingAlliesPoints - proportionalityPoints);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ExecutePunishment(int playerId, SCR_PlayerManagerKickReason reason, int durationInSeconds)
	{
		GetGame().GetDataCollector().GetPlayerData(playerId).SetTimeOut(durationInSeconds);
		
		if (SCR_PlayerDataConfigs.GetInstance().GetVotingSuggestionEnabled())
			VoteForKickOrBan(playerId, reason, durationInSeconds);
		else
			KickOrBanPlayer(playerId, reason, durationInSeconds);
	}
	
	//------------------------------------------------------------------------------------------------
	void KickOrBanPlayer(int playerId, SCR_PlayerManagerKickReason reason, int minimmumDuration)
	{
		int durationInSeconds = Math.Clamp(GetGame().GetDataCollector().GetPlayerData(playerId).GetTimeOut(), minimmumDuration, int.MAX);
		
		switch(reason)
		{
			case SCR_PlayerManagerKickReason.KICK:
			{
				GetGame().GetPlayerManager().KickPlayer(playerId, reason, 0);
				return;
			}
			case SCR_PlayerManagerKickReason.KICK_VOTED:
			{
				GetGame().GetPlayerManager().KickPlayer(playerId, reason, durationInSeconds);
				return;
			}	
			case SCR_PlayerManagerKickReason.TEMP_BAN: 
			{
				GetGame().GetPlayerManager().KickPlayer(playerId, reason, durationInSeconds);
				return;
			}
			case SCR_PlayerManagerKickReason.BAN:
			{
				GetGame().GetBackendApi().GetBanServiceApi().CreateBanPlayerId(null, playerId, "Heavy ban", durationInSeconds);
				GetGame().GetPlayerManager().KickPlayer(playerId, reason, 0);
				return;
			}
				
		}
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Vote for kicking or banning a player
	\param punishment Punishment type
	\param playerId id of the player
	\param reason reason for the kick
	\param minimmumDurationBackup minimmum duration for the ban. Used only if the voting fails
	*/
	void VoteForKickOrBan(int playerId, SCR_PlayerManagerKickReason reason, int minimmumDurationBackup)
	{
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager)
			KickOrBanPlayer(playerId, reason, minimmumDurationBackup);
		
		switch(reason)
		{
			case SCR_PlayerManagerKickReason.KICK:
			case SCR_PlayerManagerKickReason.KICK_VOTED:
				if (!votingManager.StartVoting(EVotingType.AUTO_KICK, 0, playerId))
					KickOrBanPlayer(playerId, reason, minimmumDurationBackup);
				return;
			case SCR_PlayerManagerKickReason.TEMP_BAN:
				if (!votingManager.StartVoting(EVotingType.AUTO_LIGHTBAN, 0, playerId))
					KickOrBanPlayer(playerId, reason, minimmumDurationBackup);
				return;
			case SCR_PlayerManagerKickReason.BAN:
				if (!votingManager.StartVoting(EVotingType.AUTO_HEAVYBAN, 0, playerId))
					KickOrBanPlayer(playerId, reason, minimmumDurationBackup);
				return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHints(int playerId, float harmingFriendliesScore)
	{
		if (playerId <= 0)
			return;

		if (harmingFriendliesScore > 0)
		{
			IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!playerController)
				return;

			SCR_KickHintComponent hintsComp = SCR_KickHintComponent.Cast(playerController.FindComponent(SCR_KickHintComponent));
			if (!hintsComp)
				return;

			hintsComp.NotifyClientCriminalScoreIncreased(SCR_ECrimeNotification.TEAMKILL, harmingFriendliesScore);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ProcessTemporalStats(notnull SCR_PlayerData playerData)
	{
		array<float> accumulatedActions = playerData.GetAccumulatedActions();

		//Add the stats to the m_aStats of the player since they are not "temporal" anymore
		for (int i = 0, count = accumulatedActions.Count(); i < count; i++)
		{
			float value = accumulatedActions[i];
			if (value != 0)
				playerData.AddStat(i, value, false);
		}
		
		playerData.ResetAccumulatedActions();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCriminalScore(notnull SCR_PlayerData playerData, float addToScore, float decreaseFromScore)
	{
		float currentScore = Math.Max(playerData.GetCriminalScore() - decreaseFromScore, 0);
		currentScore += addToScore;

		playerData.SetCriminalScore(currentScore);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCriminalAcceleration(int playerId)
	{
		SCR_PlayerData playerData = GetGame().GetDataCollector().GetPlayerData(playerId, false);
		if (!playerData)
			return;

		playerData.CalculateSessionDuration();
		float secondsSession = playerData.GetStat(SCR_EDataStats.SESSION_DURATION) - playerData.GetStat(SCR_EDataStats.SESSION_DURATION, false);

		float decreaseOfAcceleration, increaseOfAcceleration;

		if (secondsSession < 600)
			decreaseOfAcceleration = 0;
		else
			decreaseOfAcceleration = Math.Log10(secondsSession) - 1;
		
		SCR_PlayerDataConfigs config = SCR_PlayerDataConfigs.GetInstance();

		increaseOfAcceleration = playerData.GetCriminalScore() * config.GetScoreToAccelerationMultiplier();

		float currentAcceleration = Math.Max(playerData.GetStat(SCR_EDataStats.CRIME_ACCELERATION) - decreaseOfAcceleration, config.GetMinAcceleration());
		currentAcceleration = Math.Min(currentAcceleration + increaseOfAcceleration, config.GetMaxAcceleration());
		
		Print("Player with id " + playerId + " disconnecting. Old acceleration was " + playerData.GetStat(SCR_EDataStats.CRIME_ACCELERATION) + ", new acceleration is " + currentAcceleration, LogLevel.VERBOSE);
		
		playerData.OverrideStat(SCR_EDataStats.CRIME_ACCELERATION, currentAcceleration);
	}
};
