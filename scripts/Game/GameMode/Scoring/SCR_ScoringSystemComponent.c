class SCR_ScoringSystemComponentClass : SCR_BaseScoringSystemComponentClass
{
}

[BaseContainerProps()]
class IScoringAction
{
	[Attribute("0", UIWidgets.ComboBox, "Scoring mode", "", ParamEnumArray.FromEnum(EScoringActionMode) )]
	protected EScoringActionMode m_eScoringMode;

	//------------------------------------------------------------------------------------------------
	//! Called via SCR_ScoringSystemComponent if player score changes.
	//! \param[in] playerId PlayerId of target player
	//! \param[in] scoreInfo New scoring info of provided player
	void OnPlayerScoreChanged(int playerId, SCR_BaseScoringSystemComponent scoring);

	//------------------------------------------------------------------------------------------------
	//! Called via SCR_ScoringSystemComponent if faction score changes.
	//! \param[in] Faction Faction whose score changed
	//! \param[in] scoreInfo New scoring info of provided player
	void OnFactionScoreChanged(Faction faction, SCR_BaseScoringSystemComponent scoring);
}

//! Action type
enum EScoringActionMode
{
	SLM_Player,
	SLM_Faction,
	SLM_Any
}

//! Terminates the game once target score is met.
[BaseContainerProps()]
class EndGameAction : IScoringAction
{
	[Attribute("100", UIWidgets.EditBox, "Scoring limit", "")]
	protected int m_iScoreLimit;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetScoreLimit()
	{
		return m_iScoreLimit;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Execute(SCR_GameModeEndData endData)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode.IsRunning())
			return;
		
		gameMode.EndGameMode(endData);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerScoreChanged(int playerId, SCR_BaseScoringSystemComponent scoring)
	{
		if (m_eScoringMode != EScoringActionMode.SLM_Player && m_eScoringMode != EScoringActionMode.SLM_Any)
			return;

		int score = scoring.GetPlayerScore(playerId);
		if (score < m_iScoreLimit)
			return;

		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerId: playerId);
		Execute(endData);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFactionScoreChanged(Faction faction, SCR_BaseScoringSystemComponent scoring)
	{
		if (m_eScoringMode != EScoringActionMode.SLM_Faction && m_eScoringMode != EScoringActionMode.SLM_Any)
			return;

		int score = scoring.GetFactionScore(faction);
		if (score < m_iScoreLimit)
			return;
		
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: factionIndex);
		Execute(endData);
	}
}

//! This component implements logic of handling certain score limits.
class SCR_ScoringSystemComponent : SCR_BaseScoringSystemComponent
{
	//Score multipliers
	[Attribute("1", UIWidgets.EditBox, "Kill score multiplier", category: "Scoring: Multipliers")]
	protected int m_iKillScoreMultiplier;

	[Attribute("-1", UIWidgets.EditBox, "Teamkill score multiplier", category: "Scoring: Multipliers")]
	protected int m_iTeamKillScoreMultiplier;

	[Attribute("0", UIWidgets.EditBox, "Death score multiplier", category: "Scoring: Multipliers")]
	protected int m_iDeathScoreMultiplier;

	[Attribute("-1", UIWidgets.EditBox, "Suicide score multiplier", category: "Scoring: Multipliers")]
	protected int m_iSuicideScoreMultiplier;

	[Attribute("1", UIWidgets.EditBox, "Objective score multiplier", category: "Scoring: Multipliers")]
	protected int m_iObjectiveScoreMultiplier;
	
	[Attribute("", UIWidgets.Object, "List of actions executed when score changes.", category: "Scoring: Actions")]
	protected ref array<ref IScoringAction> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	//! \return maximum allowed score or -1 if undefined.
	override int GetScoreLimit() 
	{
		foreach (IScoringAction scoringAction : m_aActions)
		{
			EndGameAction possibleAction = EndGameAction.Cast(scoringAction);
			if (possibleAction)
				return possibleAction.GetScoreLimit();
		}
		
		return super.GetScoreLimit();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override int CalculateScore(SCR_ScoreInfo info)
	{
		int score = info.m_iKills 		* m_iKillScoreMultiplier +
					info.m_iTeamKills 	* m_iTeamKillScoreMultiplier +
					info.m_iDeaths		* m_iDeathScoreMultiplier +
					info.m_iSuicides 	* m_iSuicideScoreMultiplier +
					info.m_iObjectives 	* m_iObjectiveScoreMultiplier;
		
		if (score < 0)
			return 0;
		
		return score;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle and dispatch scoring logic for this event.
	protected override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);
		
		// Add death no matter what
		AddDeath(instigatorContextData.GetVictimPlayerID());
		
		Instigator instigator = instigatorContextData.GetInstigator();
		
		//~ Killed by AI
		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		SCR_ECharacterControlType victimControlType = instigatorContextData.GetVictimCharacterControlType();
		SCR_ECharacterControlType killerControlType = instigatorContextData.GetKillerCharacterControlType();
		
		//~ Score for killing self, killing enemy player and killing friendly player
		//~ Player killed self
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
		{
			//~ Possessed AI do not count the suicide
			if (victimControlType == SCR_ECharacterControlType.POSSESSED_AI)
				return;
			
			AddSuicide(instigatorContextData.GetVictimPlayerID());
			return;
		}
		//~ Killed by enemy player. 
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER))
		{
			//~ If killer is a possessed player it is not counted
			if (killerControlType == SCR_ECharacterControlType.POSSESSED_AI)
				return;
			
			AddKill(instigator.GetInstigatorPlayerID());
			return;
		}
		//~ Killed by friendly player. Score will still be deducted from friendly kills of admin and GMs
		if (instigatorContextData.DoesPlayerKillCountAsTeamKill(editorKillsCount: true))
		{
			AddTeamKill(instigator.GetInstigatorPlayerID());
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		super.OnGameModeEnd(data);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called on all machines when player score changes.
	protected override void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		super.OnPlayerScoreChanged(playerId, scoreInfo);
		
		foreach (IScoringAction action : m_aActions)
		{
			action.OnPlayerScoreChanged(playerId, this);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called on all machines when faction score changes.
	protected override void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		super.OnFactionScoreChanged(faction, scoreInfo);
		
		foreach (IScoringAction action : m_aActions)
		{
			action.OnFactionScoreChanged(faction, this);
		}
	}
}
