[EntityEditorProps(category: "GameScripted/GameMode", description: "Takes care of player penalties, kicks, bans etc.", color: "0 0 255 255")]
class SCR_LocalPlayerPenaltyClass : Managed
{
}

class SCR_LocalPlayerPenalty : Managed
{
	//[Attribute("3", desc: "Penalty score for killing a friendly player.")]
	protected int m_iFriendlyPlayerKillPenalty;
	
	//[Attribute("1", desc: "Penalty score for killing a friendly AI.")]
	protected int m_iFriendlyAIKillPenalty;
	
	//[Attribute("10", desc: "Penalty score limit for a kick from the match.")]
	protected int m_iKickPenaltyLimit;
	
	//[Attribute("1800", desc: "Ban duration after a kick (in seconds, -1 for a session-long ban).")]
	protected int m_iBanDuration;
	
	//[Attribute("900", desc: "How often penalty score subtraction happens (in seconds).")]
	protected int m_iPenaltySubtractionPeriod;
	
	//[Attribute("2", desc: "How many penalty points get substracted after each subtraction period.")]
	protected int m_iPenaltySubtractionPoints;
	
	protected static SCR_LocalPlayerPenalty s_Instance;
	protected static const int EVALUATION_PERIOD = 1000;
	
	protected ref array<ref SCR_LocalPlayerPenaltyData> m_aPlayerPenaltyData = {};
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void OnPlayerConnected(int playerId)
	{
		//GetPlayerPenaltyData creates the PlayerPenaltyStandaloneData for this playerId if it doesn't exist
		GetPlayerPenaltyData(playerId);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity
	//! \param[in] killerEntity
	//! \param[in] instigator
	//! \param[in] instigatorContextData Holds data of victim and killer
	void OnControllableDestroyed(IEntity entity, IEntity killerEntity, Instigator instigator, notnull SCR_InstigatorContextData instigatorContextData)
	{
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER || (m_iFriendlyAIKillPenalty == 0 && m_iFriendlyPlayerKillPenalty == 0))
			return;
		
		//~ Kill is not a teamkill and is not a warcrime
		if (!instigatorContextData.DoesPlayerKillCountAsTeamKill() && !instigatorContextData.IsEnemyKillPunished(SCR_EDisguisedKillingPunishment.WARCRIME))
			return;
		
		SCR_ECharacterControlType victimControlType = instigatorContextData.GetVictimCharacterControlType();
		int killerPlayerId = instigator.GetInstigatorPlayerID();
				
		//~ Character killed by player (Unlimited editor players and admins do not get punished)
		switch (victimControlType)
		{
			//~ When killed a player or player that is GM/Admin
			case SCR_ECharacterControlType.PLAYER:
			case SCR_ECharacterControlType.UNLIMITED_EDITOR:
			{
				SCR_LocalPlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(killerPlayerId);
				
				if (playerPenaltyData)
					playerPenaltyData.AddPenaltyScore(m_iFriendlyPlayerKillPenalty);
				
				break;
			}
			//~ When killing friendly AI or possessed AI (Possessed AI are treated as normal AI as they are hidden to the player)
			case SCR_ECharacterControlType.AI:
			case SCR_ECharacterControlType.POSSESSED_AI:
			{
				SCR_LocalPlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(killerPlayerId);
				
				if (playerPenaltyData)
					playerPenaltyData.AddPenaltyScore(m_iFriendlyAIKillPenalty);
				
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	static SCR_LocalPlayerPenalty GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] veh
	//! \param[in] gunner
	//! \return
	//IT SHOULD NOT BE STATIC. TODO: IMPROVE THIS
	static SCR_ChimeraCharacter GetInstigatorFromVehicle(IEntity veh, bool gunner = false)
	{
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
			
		if (!compartmentManager)
			return null;
		
		array<BaseCompartmentSlot> compartments = new array <BaseCompartmentSlot>();
		
		for (int i = 0, cnt = compartmentManager.GetCompartments(compartments); i < cnt; i++)
		{
			BaseCompartmentSlot slot = compartments[i];
			
			if (slot && (!gunner && slot.Type() == PilotCompartmentSlot) || (gunner && slot.Type() == TurretCompartmentSlot))
				return SCR_ChimeraCharacter.Cast(slot.GetOccupant());
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_LocalPlayerPenaltyData GetPlayerPenaltyData(int playerId)
	{
		if (playerId == 0)
			return null;
		
		SCR_LocalPlayerPenaltyData playerPenaltyData;

		// Check if the client is reconnecting
		//Also if there's going to be many entries, the search might prove very expensive.
		for (int i = 0, count = m_aPlayerPenaltyData.Count(); i < count; i++)
		{
			if (m_aPlayerPenaltyData[i].GetPlayerId() == playerId)
			{
				playerPenaltyData = m_aPlayerPenaltyData[i];
				break;
			}
		}
		
		// Client reconnected, return saved data
		if (playerPenaltyData)
			return playerPenaltyData;
		
		// Check validity of playerId before registering new data
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		
		if (!pc)
		{
			Print(string.Format("SCR_PlayerPenaltyComponent: No player with playerId %1 found.", playerId), LogLevel.ERROR);
			return null;
		}
		
		// First connection, register new data
		playerPenaltyData = new SCR_LocalPlayerPenaltyData();
		playerPenaltyData.SetPlayerId(playerId);
		m_aPlayerPenaltyData.Insert(playerPenaltyData);
		
		return playerPenaltyData;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EvaluatePlayerPenalties()
	{
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		for (int i = 0, count = m_aPlayerPenaltyData.Count(); i < count; i++)
		{
			SCR_LocalPlayerPenaltyData playerPenaltyData = m_aPlayerPenaltyData[i];
			
			// Periodically forgive a portion of penalty score, don't go below zero
			if (playerPenaltyData.GetPenaltyScore() > 0 && playerPenaltyData.GetNextPenaltySubtractionTimestamp().Less(currentTime))
			{
				int forgivenScore;
				
				if (m_iPenaltySubtractionPoints > playerPenaltyData.GetPenaltyScore())
					forgivenScore = playerPenaltyData.GetPenaltyScore();
				else
					forgivenScore = m_iPenaltySubtractionPoints;
				
				playerPenaltyData.AddPenaltyScore(-forgivenScore);
			}
			
			int playerId = playerPenaltyData.GetPlayerId();
			
			// Player is not connected
			if (!GetGame().GetPlayerManager().GetPlayerController(playerId))
				continue;
			
			// Player is host
			if (playerId == SCR_PlayerController.GetLocalPlayerId())
				continue;
			
			// Check penalty limit for kick / ban
			if (m_iKickPenaltyLimit > 0 && playerPenaltyData.GetPenaltyScore() >= m_iKickPenaltyLimit)
			{
				// TODO: Use callback from backend instead
				KickPlayer(playerId, m_iBanDuration, SCR_PlayerManagerKickReason.FRIENDLY_FIRE);
				continue;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \param[in] duration
	//! \param[in] reason
	void KickPlayer(int playerId, int duration, SCR_PlayerManagerKickReason reason)
	{	
		SCR_LocalPlayerPenaltyData playerPenaltyData = GetPlayerPenaltyData(playerId);
		
		if (playerPenaltyData)
			playerPenaltyData.AddPenaltyScore(-playerPenaltyData.GetPenaltyScore());
		
		GetGame().GetPlayerManager().KickPlayer(playerId, reason, duration);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPenaltySubtractionPeriod()
	{
		return m_iPenaltySubtractionPeriod * 1000;	// Converting s to ms
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] friendlyPlayerKillPenalty
	//! \param[in] friendlyAIKillPenalty
	//! \param[in] penaltyLimit
	//! \param[in] banDuration
	//! \param[in] penaltySubtractionPeriod
	//! \param[in] penaltySubtractionPoints
	void SCR_LocalPlayerPenalty(int friendlyPlayerKillPenalty, int friendlyAIKillPenalty, int penaltyLimit, int banDuration, int penaltySubtractionPeriod, int penaltySubtractionPoints)
	{		
		m_iFriendlyPlayerKillPenalty = friendlyPlayerKillPenalty;
		m_iFriendlyAIKillPenalty = friendlyAIKillPenalty;
		m_iKickPenaltyLimit = penaltyLimit;
		m_iBanDuration = banDuration;
		m_iPenaltySubtractionPeriod = penaltySubtractionPeriod;
		m_iPenaltySubtractionPoints = penaltySubtractionPoints;
		
		if (m_iBanDuration < m_iPenaltySubtractionPeriod)
			Print("SCR_PlayerPenaltyComponent: Ban duration is shorter than Penalty substraction period. This will cause the player to remain banned until their penalty is substracted.", LogLevel.WARNING);
		
		s_Instance = this;
		//Looping every EVALUATION_PERIOD seconds and don't need other EOn events
		GetGame().GetCallqueue().CallLater(EvaluatePlayerPenalties, EVALUATION_PERIOD, true);
	}
}

class SCR_LocalPlayerPenaltyData
{
	protected int m_iPlayerId;
	protected int m_iPenaltyScore;
	protected WorldTimestamp m_fNextPenaltySubtractionTimestamp;
	protected SCR_PlayerManagerKickReason m_eKickReason = SCR_PlayerManagerKickReason.DISRUPTIVE_BEHAVIOUR;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	void SetPlayerId(int playerId)
	{
		m_iPlayerId = playerId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetPlayerId()
	{
		return m_iPlayerId;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] points
	void AddPenaltyScore(int points)
	{
		m_iPenaltyScore += points;
		
		// Start the timer on penalty substraction when player was penalized while the timer was stopped
		ChimeraWorld world = GetGame().GetWorld();
		WorldTimestamp currentTime = world.GetServerTimestamp();
		if ((points > 0 && m_fNextPenaltySubtractionTimestamp.Less(currentTime)) || (points < 0 && m_iPenaltyScore > 0))
			m_fNextPenaltySubtractionTimestamp = currentTime.PlusMilliseconds(SCR_LocalPlayerPenalty.GetInstance().GetPenaltySubtractionPeriod());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetPenaltyScore()
	{
		return m_iPenaltyScore;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] timestamp
	void SetNextPenaltySubstractionTimestamp(WorldTimestamp timestamp)
	{
		m_fNextPenaltySubtractionTimestamp = timestamp;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	WorldTimestamp GetNextPenaltySubtractionTimestamp()
	{
		return m_fNextPenaltySubtractionTimestamp;
	}
}
