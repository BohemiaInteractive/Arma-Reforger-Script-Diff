//! Handles the showing of end screen and has a config with gameover screens
[ComponentEditorProps(category: "GameScripted/GameOver", description: "")]
class SCR_GameOverScreenManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_GameOverScreenManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref SCR_GameOverScreenConfig m_GameOverScreenConfig;
	
	[Attribute("1", desc: "Delay (in seconds) to lock player input and display the actual end screen after end screen has faded in")]
	protected float m_fShowEndscreenDelay;
	
	[Attribute("{144919871D65F121}UI/layouts/HUD/GameOver/EndScreen/EndScreen.layout", params: "layout")]
	protected ResourceName m_sGameOverScreenBasePrefab;
	
	protected ResourceName m_sGameOverScreenOverwritePrefab;
	
	//Editor only
	protected bool m_bListeningToEditorCalledEndGame = false;
	protected EGameOverTypes m_iEditorSetGameOverType;
	protected ref array<int> m_aEditorSetFactions = {};
	
	//End game vars saved so it can be usesed in multiple function
	protected EGameOverTypes m_iEndGameType = EGameOverTypes.UNKNOWN;
	protected bool m_bIsFactionVictory = false;
	protected Faction m_FactionPlayer = null;
	protected int m_iPlayerId = -1;
	protected int m_iEndReason = 0;
	protected ref array<Faction> m_aWinningFactions = {};
	protected ref array<int> m_aWinningPlayers = {};
	protected bool m_bIsPlayingGameOverAudio = false;
	
	static const ref ScriptInvoker s_OnEndGame = new ScriptInvoker();
	
	protected Widget m_EndScreenFade;
	
	//------------------------------------------------------------------------------------------------
	//! \return GameoverScreen Config
	SCR_GameOverScreenConfig GetGameOverConfig()
	{
		return m_GameOverScreenConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets which background will be used in the game over screen
	//! param[in] overWriteLayout Layout that will be used instead of basic game over screen
	void SetGameOverScreenOverWrite(ResourceName overWriteLayout)
	{
		m_sGameOverScreenOverwritePrefab = overWriteLayout;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when end game is called on GameMode
	//! Fade the endgame screen but retain control
	//! Once the fade in is done the end screen will be shown
	protected void StartEndGameFade()
	{	
		SetGameOverVariables(GetGameMode().GetEndGameData());
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return;
		
		ResourceName gameOverscreen = m_sGameOverScreenBasePrefab;
		
		SCR_BaseGameOverScreenInfo gameOverScreenInfo;
		if (m_GameOverScreenConfig.GetGameOverScreenInfo(m_iEndGameType, gameOverScreenInfo))
			m_sGameOverScreenOverwritePrefab = gameOverScreenInfo.GetBaseGameOverScreenOverwrite();
		
		if (!m_sGameOverScreenOverwritePrefab.IsEmpty())
			gameOverscreen = m_sGameOverScreenOverwritePrefab;
		
		m_EndScreenFade = hudManager.CreateLayout(gameOverscreen, EHudLayers.ALWAYS_TOP, 0);
		SCR_FadeUIComponent fadeComponent = SCR_FadeUIComponent.Cast(m_EndScreenFade.FindHandler(SCR_FadeUIComponent));
		
		if (!fadeComponent)
		{
			Debug.Error2("SCR_GameOverScreenManagerComponent", "StartEndGameFade, could not find FadeUIComponent!");
			return;
		}

		fadeComponent.GetOnFadeDone().Insert(OnEndScreenFadeDone);
		fadeComponent.FadeIn();
		
		// Hide other HUD elements
		hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~(EHudLayers.HIGH));
		
		//Play game over audio if has any and non are playing
		if (m_bIsPlayingGameOverAudio)
			return;

		if (!gameOverScreenInfo)
		{
			Print(string.Format("SCR_GameOverScreenManagerComponent could not find end screen: %1, to play audio", typename.EnumToString(EGameOverTypes, m_iEndGameType)), LogLevel.WARNING);
			return;
		}
		
		if (!gameOverScreenInfo.HasOptionalParams())
			return;
		
		if (m_bIsFactionVictory)
			s_OnEndGame.Invoke(gameOverScreenInfo.GetOneShotAudio(m_FactionPlayer, m_aWinningFactions));
		else
			s_OnEndGame.Invoke(gameOverScreenInfo.GetOneShotAudio(m_iPlayerId, m_aWinningPlayers));
		
		if (gameOverScreenInfo.PauseGameOnGameOverScreen())
		{
			ChimeraWorld world = GetGame().GetWorld();
			if (world)
				world.PauseGameTime(true);
		}
		
		m_bIsPlayingGameOverAudio = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! End screen fade is done show gameover dialog when delay is done
	//! \param[in] fadeComponent
	//! \param[in] isFadeIn
	protected void OnEndScreenFadeDone(SCR_FadeUIComponent fadeComponent, bool isFadeIn)
	{
		//Close all dialogs
		GetGame().GetMenuManager().CloseAllMenus();
		GetGame().GetCallqueue().CallLater(ShowGameOverScreen, m_fShowEndscreenDelay * 1000, false, null);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call end screen using SCR_GameModeEndData. This is generally called after fade but can also be called instantly if desired
	//! \param[in] endData end game data. Will get endgame date from gamemode if non given
	//! \return SCR_GameOverScreenUIComponent game over screen widget
	SCR_GameOverScreenUIComponent ShowGameOverScreen(SCR_GameModeEndData endData = null)
	{		
		//Remove the fade UI
		if (m_EndScreenFade)
		{
			m_EndScreenFade.RemoveFromHierarchy();
			
			// Unhide other HUD elements which we hid previously when we started the fade
			SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
			if (hudManager)
				hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() | EHudLayers.HIGH);
		}
				
		if (!endData)
			endData = GetGameMode().GetEndGameData();
		
		if (m_iEndGameType == EGameOverTypes.UNKNOWN)
			SetGameOverVariables(endData);
		
		SCR_BaseGameOverScreenInfo gameOverScreenInfo;
				
		//Get game over type
		if (!m_GameOverScreenConfig.GetGameOverScreenInfo(m_iEndGameType, gameOverScreenInfo))
		{
			Print(string.Format("'SCR_GameOverScreenConfig': Could not find '%1' endscreen in array!", typename.EnumToString(EGameOverTypes, m_iEndGameType)), LogLevel.NORMAL);
			return null;
		}
		
		if (gameOverScreenInfo.HasOptionalParams())
		{
			//Show end screen
			if (m_bIsFactionVictory)
			{
				if (!m_bIsPlayingGameOverAudio)
					s_OnEndGame.Invoke(gameOverScreenInfo.GetOneShotAudio(m_FactionPlayer, m_aWinningFactions));
				
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), gameOverScreenInfo, endData, gameOverScreenInfo.GetTitle(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetSubtitle(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetDebriefing(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetImage(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetIcon(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetVignetteColor(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetTitleParam(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetSubtitleParam(m_FactionPlayer, m_aWinningFactions), gameOverScreenInfo.GetDebriefingParam(m_FactionPlayer, m_aWinningFactions));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_FactionPlayer, m_aWinningFactions);
				return gameOverScreen;
			}
			else 
			{
				if (!m_bIsPlayingGameOverAudio)
					s_OnEndGame.Invoke(gameOverScreenInfo.GetOneShotAudio(m_iPlayerId, m_aWinningPlayers));

				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), gameOverScreenInfo, endData, gameOverScreenInfo.GetTitle(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetSubtitle(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetDebriefing(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetImage(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetIcon(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetVignetteColor(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetTitleParam(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetSubtitleParam(m_iPlayerId, m_aWinningPlayers), gameOverScreenInfo.GetDebriefingParam(m_iPlayerId, m_aWinningPlayers));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_iPlayerId, m_aWinningPlayers);
				return gameOverScreen;
			}
		}
		else
		{
			if (m_bIsFactionVictory)
			{
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), gameOverScreenInfo, endData, gameOverScreenInfo.GetTitle(m_FactionPlayer, m_aWinningFactions));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_FactionPlayer, m_aWinningFactions);
				return gameOverScreen;
			}
			else 
			{
				SCR_GameOverScreenUIComponent gameOverScreen = OpenGameOverScreen(gameOverScreenInfo.GetGameOverContentLayout(), gameOverScreenInfo, endData, gameOverScreenInfo.GetTitle(m_iPlayerId, m_aWinningPlayers));
				gameOverScreenInfo.GameOverUICustomization(gameOverScreen, m_iPlayerId, m_aWinningPlayers);
				return gameOverScreen;
			}
		}
		
		//Makes sure set game over varriables are set again next time end game is called
		m_iEndGameType = EGameOverTypes.UNKNOWN;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set all variables needed to set the correct game over screen
	//! \param[in] endData
	protected void SetGameOverVariables(SCR_GameModeEndData endData = null)
	{
		m_iEndGameType = EGameOverTypes.UNKNOWN;
		m_bIsPlayingGameOverAudio = false;
		m_FactionPlayer = null;
		m_iPlayerId = -1;
		m_iEndReason = 0;
		m_aWinningFactions.Clear();
		m_aWinningPlayers.Clear();
		m_bIsFactionVictory = false;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		if (!endData)
			endData = GetGameMode().GetEndGameData();
		
		if (endData != null) 
		{		
			m_iEndReason = endData.GetEndReason();

			if (factionManager)
			{
				array<int> winningFactionIds = {};
				endData.GetFactionWinnerIds(winningFactionIds);
				
				if (winningFactionIds)
				{
					Faction factionVictor;
					
					foreach (int id: winningFactionIds)
					{
						factionVictor = factionManager.GetFactionByIndex(id);
						if (factionVictor)
							m_aWinningFactions.Insert(factionVictor);
					}
				}
			}
				
			endData.GetWinnerIds(m_aWinningPlayers);
			
			m_bIsFactionVictory = (m_aWinningFactions != null && !m_aWinningFactions.IsEmpty());
		}
		
		if (playerManager)
		{
			SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (pc)
			{
				m_iPlayerId = pc.GetPlayerId();
				
				SCR_FactionManager scrFactionManager = SCR_FactionManager.Cast(factionManager);
				if (scrFactionManager)
					m_FactionPlayer = scrFactionManager.GetLocalPlayerFaction();
				else
					m_FactionPlayer = null;
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_bIsFactionVictory && m_iEndReason == EGameOverTypes.EDITOR_FACTION_DRAW && gameMode.GetRemainingTime() <= 0 && m_aWinningFactions.IsEmpty())
		{
			SCR_BaseScoringSystemComponent scoringSystem = gameMode.GetScoringSystemComponent();
			if (scoringSystem)
			{
				array<Faction> factions = {};
				int count = GetGame().GetFactionManager().GetFactionsList(factions);
				
				SCR_Faction scrfaction;
				for (int i = count - 1; i >= 0; i--)
				{
					scrfaction = SCR_Faction.Cast(factions[i]);
					if (!scrfaction.IsPlayable())
						factions.RemoveOrdered(i);
				}
				
				if (factions.Count() == 2)
				{
					int scoreA = scoringSystem.GetFactionScore(factions[0]);
					int scoreB = scoringSystem.GetFactionScore(factions[1]);

					if (scoreA > scoreB)
					{
						m_bIsFactionVictory = true;
						m_aWinningFactions.Insert(factions[0]);
						m_iEndReason = EGameOverTypes.ENDREASON_SCORELIMIT;
					}
					else if (scoreB > scoreA)
					{
						m_bIsFactionVictory = true;
						m_aWinningFactions.Insert(factions[1]);
						m_iEndReason = EGameOverTypes.ENDREASON_SCORELIMIT;
					}
					else 
					{
						m_iEndReason = EGameOverTypes.FACTION_DRAW;
					}
				}
			}
		}
		
		//Get the type of game over using given data
		m_iEndGameType = GetGameOverType(m_iEndReason, m_iPlayerId, m_aWinningPlayers, m_FactionPlayer, m_aWinningFactions, m_bIsFactionVictory);
	}
	
	//---- REFACTOR NOTE START: SCR_GameModeEndData and this method use different values for endReason which is unified with these enums. Not ideal, should be done in the SCR_GameModeEndData ----
	//------------------------------------------------------------------------------------------------
	//! This method checks if the player is part of the winning faction (if any), if it is the winning player (if any) and return the correct GameOverType (Eg: Won, lost ect)
	//! \param[in] endReason
	//! \param[in] playerId
	//! \param[in] winningPlayers
	//! \param[in] factionPlayer
	//! \param[in] factionsVictor
	//! \param[in] isFactionVictory
	//! \return
	protected EGameOverTypes GetGameOverType(int endReason, int playerId, array<int> winningPlayers, Faction factionPlayer, array<Faction> factionsVictor, bool isFactionVictory)
	{
		//// ==== Default end reasons ==== \\\\
		
		//-1: ENDREASON_UNDEFINED: Undefined
		if (endReason == EGameOverTypes.ENDREASON_UNDEFINED)
		{
			return EGameOverTypes.NEUTRAL;
		}
		//-2: ENDREASON_TIMELIMIT: Time limit reached
		else if (endReason == EGameOverTypes.ENDREASON_TIMELIMIT)
		{
			if (isFactionVictory)
			{	
				if (!factionPlayer)
					return EGameOverTypes.FACTION_NEUTRAL;
				else if (factionsVictor.Contains(factionPlayer))
					return EGameOverTypes.FACTION_VICTORY_TIME;
				else 
					return EGameOverTypes.FACTION_DEFEAT_TIME;	
			}
			else 
			{
				if (winningPlayers.Contains(playerId))
					return EGameOverTypes.DEATHMATCH_VICTORY_TIME;
				else 
					return EGameOverTypes.DEATHMATCH_DEFEAT_TIME;
			}
			
		}
		//-3: ENDREASON_SCORELIMIT: Score limit reached
		else if (endReason == EGameOverTypes.ENDREASON_SCORELIMIT)
		{
			if (isFactionVictory)
			{	
				if (!factionPlayer)
					return EGameOverTypes.FACTION_NEUTRAL;
				else if (factionsVictor.Contains(factionPlayer))
					return EGameOverTypes.FACTION_VICTORY_SCORE;
				else 
					return EGameOverTypes.FACTION_DEFEAT_SCORE;	
			}
			else 
			{
				if (winningPlayers.Contains(playerId))
					return EGameOverTypes.DEATHMATCH_VICTORY_SCORE;
				else 
					return EGameOverTypes.DEATHMATCH_DEFEAT_SCORE;
			}
		}
		//-4: ENDREASON_DRAW
		else if (endReason == EGameOverTypes.ENDREASON_DRAW)
		{
			if (isFactionVictory)
			{
				if (!factionPlayer)
					return EGameOverTypes.FACTION_NEUTRAL;
				else 
					return EGameOverTypes.FACTION_DRAW;
			}
			else 
			{
				return EGameOverTypes.DEATHMATCH_DRAW;
			}
		}
		//// ==== Shared End Reasons ==== \\\\
		else if (endReason == EGameOverTypes.SERVER_RESTART)
		{
			return EGameOverTypes.SERVER_RESTART;
		}
		//// ==== Editor End Reasons ==== \\\\
		//Editor Victory
		else if (endReason == EGameOverTypes.EDITOR_FACTION_VICTORY)
		{
			if (!factionPlayer)
				return EGameOverTypes.EDITOR_FACTION_NEUTRAL;
			else if (factionsVictor.Contains(factionPlayer))
				return EGameOverTypes.EDITOR_FACTION_VICTORY;
			else 
				return EGameOverTypes.EDITOR_FACTION_DEFEAT;
		}
		//Editor Draw
		else if (endReason == EGameOverTypes.EDITOR_FACTION_DRAW)
		{
			if (!factionPlayer)
				return EGameOverTypes.EDITOR_FACTION_NEUTRAL;
			else 
				return EGameOverTypes.EDITOR_FACTION_DRAW;
		}
		//// ==== Specific end reasons ==== \\\\
		else 
		{
			return endReason;
		}
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! This will only return the valid gameover type once game over is called!
	//! \return GameOver Type
	EGameOverTypes GetCurrentGameOverType()
	{
		return m_iEndGameType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Open end screen
	//! \param[in] contentLayout
	//! \param[in] gameOverInfo
	//! \param[in] endData
	//! \param[in] title title of game over screen
	//! \param[in] subtitle subtitle of game over screen
	//! \param[in] debriefing debriefing text of game over screen
	//! \param[in] imageTexture Image shown in game over screen
	//! \param[in] icon
	//! \param[in] vignetteColor will set the color of the gameover screen background overlay
	//! \param[in] titleParam title param %1
	//! \param[in] subtitleParam subtitle param %1
	//! \param[in] debriefingParam debriefing param %1
	//! \return Widget endscreen widget
	SCR_GameOverScreenUIComponent OpenGameOverScreen(ResourceName contentLayout, SCR_BaseGameOverScreenInfo gameOverInfo, SCR_GameModeEndData endData, LocalizedString title = string.Empty, LocalizedString subtitle = string.Empty, LocalizedString debriefing = string.Empty, ResourceName imageTexture = ResourceName.Empty, ResourceName icon = ResourceName.Empty, Color vignetteColor = null, string titleParam = string.Empty, string subtitleParam = string.Empty, string debriefingParam = string.Empty)
	{
		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		if (!hudManager)
			return null;
		
		//This is just temporary hopefully. This bypasses a lot of the code here and in other files. If one wants to change the layout, do it in Menu Config.
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.EndgameScreen, 0, true);
		
		ResourceName gameOverscreen = m_sGameOverScreenBasePrefab;
		
		if (!m_sGameOverScreenOverwritePrefab.IsEmpty())
			gameOverscreen = m_sGameOverScreenOverwritePrefab;
		
		Widget screen = GetGame().GetWorkspace().CreateWidgets(gameOverscreen, menu.GetRootWidget());
				
		SCR_GameOverScreenUIComponent screenUIComponent = SCR_GameOverScreenUIComponent.Cast(screen.FindHandler(SCR_GameOverScreenUIComponent));
		
		if (screenUIComponent)
		{
			SCR_GameOverScreenUIContentData content = new SCR_GameOverScreenUIContentData(contentLayout, gameOverInfo, endData, title, subtitle, debriefing, imageTexture, icon, vignetteColor, titleParam, subtitleParam, debriefingParam);
			screenUIComponent.InitGameOverScreen(content);
		}

		return screenUIComponent;
	}
	
	//// ==== Editor Attribute Game over logic ==== \\\

	//------------------------------------------------------------------------------------------------
	protected void OnEditorEndGameApplyDelayDone()
	{
		m_bListeningToEditorCalledEndGame = false;
		
		SCR_BaseGameMode gamemode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gamemode)
			return;
		
		SCR_BaseGameOverScreenInfo gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(m_iEditorSetGameOverType);
		if (!gameOverInfo)
		{
			gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(EGameOverTypes.EDITOR_NEUTRAL);
			//~ Could not find game over so used EDITOR_NEUTRAL
			if (gameOverInfo)
			{
				m_iEditorSetGameOverType = EGameOverTypes.EDITOR_NEUTRAL;
				Print(string.Format("SCR_GameOverScreenManagerComponent could not find end screen: %1, so EDITOR_NEUTRAL is used instead", typename.EnumToString(EGameOverTypes, m_iEditorSetGameOverType)), LogLevel.ERROR);
			}
			//~ Could not find EDITOR_NEUTRAL either
			else 
			{
				//~ Try to find NEUTRAL
				gameOverInfo = m_GameOverScreenConfig.GetGameOverInfo(EGameOverTypes.NEUTRAL);
				if (gameOverInfo)
				{
					m_iEditorSetGameOverType = EGameOverTypes.NEUTRAL;
					Print(string.Format("SCR_GameOverScreenManagerComponent could not find end screen: %1 and EDITOR_NEUTRAL could not be found so NEUTRAL is used instead", typename.EnumToString(EGameOverTypes, m_iEditorSetGameOverType)), LogLevel.ERROR);
				}
				//~ Even NEUTRAL could not be found for some reason
				else 
				{
					Print(string.Format("SCR_GameOverScreenManagerComponent could not find end screen: %1 nor EDITOR_NEUTRAL or NEUTRAL. Check if the config is set up correctly!", typename.EnumToString(EGameOverTypes, m_iEditorSetGameOverType)), LogLevel.ERROR);
					return;
				}
			}
		}
		
		SCR_BaseGameOverScreenInfoEditor optionalEditorParam = gameOverInfo.GetEditorOptionalParams();
		
		if (!optionalEditorParam || !optionalEditorParam.m_bNeedsPlayableFactions)
			gamemode.EndGameMode(SCR_GameModeEndData.CreateSimple(m_iEditorSetGameOverType));
		else
			gamemode.EndGameMode(SCR_GameModeEndData.Create(m_iEditorSetGameOverType, null, m_aEditorSetFactions));
		
		//Clear vars
		m_aEditorSetFactions.Clear();
		m_iEditorSetGameOverType = EGameOverTypes.EDITOR_NEUTRAL;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the gameover type for editor gameover used in the gameover screen
	//! \param[in] gameOverType EGameOverTypes gameover type
	void SetEditorGameOverType(EGameOverTypes gameOverType)
	{
		m_iEditorSetGameOverType = gameOverType;
		
		if (!m_bListeningToEditorCalledEndGame)
		{
			m_bListeningToEditorCalledEndGame = true;
			GetGame().GetCallqueue().CallLater(OnEditorEndGameApplyDelayDone, 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set the factions for editor gameover used in the gameover screen (Generally winning factions)
	//! \param[in] factions array of factions
	void SetEditorGameOverFactions(notnull array<int> factions)
	{
		m_aEditorSetFactions = factions;
		
		if (!m_bListeningToEditorCalledEndGame)
		{
			m_bListeningToEditorCalledEndGame = true;
			GetGame().GetCallqueue().CallLater(OnEditorEndGameApplyDelayDone, 1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// When game ends
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		//If called on init HUD Manager might not exist so call it with a delay
		if (!GetGame().GetHUDManager())
		{
			GetGame().GetCallqueue().CallLater(OnInitDelayedEndGame, 1000);
			return;
		}
		StartEndGameFade();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInitDelayedEndGame()
	{
		StartEndGameFade();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
	}
}

class SCR_GameOverScreenUIContentData
{
	ResourceName m_sGameOverLayout;
	SCR_BaseGameOverScreenInfo m_GameOverInfo;
	ref SCR_GameModeEndData m_EndGameData;
	LocalizedString m_sTitle;
	LocalizedString m_sSubtitle; 
	LocalizedString m_sDebriefing; 
	ResourceName m_sImageTexture; 
	ResourceName m_sIcon; 
	Color m_cVignetteColor; 
	string m_sTitleParam; 
	string m_sSubtitleParam; 
	string m_sDebriefingParam;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] gameOverLayout
	//! \param[in] gameOverInfo
	//! \param[in] endGameData
	//! \param[in] title
	//! \param[in] subtitle
	//! \param[in] debriefing
	//! \param[in] imageTexture
	//! \param[in] icon
	//! \param[in] vignetteColor
	//! \param[in] titleParam
	//! \param[in] subtitleParam
	//! \param[in] debriefingParam
	void SCR_GameOverScreenUIContentData(ResourceName gameOverLayout, SCR_BaseGameOverScreenInfo gameOverInfo, SCR_GameModeEndData endGameData, LocalizedString title = string.Empty, LocalizedString subtitle = string.Empty, LocalizedString debriefing = string.Empty, ResourceName imageTexture = ResourceName.Empty, ResourceName icon = ResourceName.Empty, Color vignetteColor = null, string titleParam = string.Empty, string subtitleParam = string.Empty, string debriefingParam = string.Empty)
	{
		m_sGameOverLayout = gameOverLayout;
		m_GameOverInfo = gameOverInfo;
		m_EndGameData = endGameData;
		m_sTitle = title;
		m_sSubtitle = subtitle;
		m_sDebriefing = debriefing;
		m_sImageTexture = imageTexture;
		m_sIcon = icon;
		m_cVignetteColor = vignetteColor;
		m_sTitleParam = titleParam;
		m_sSubtitleParam = subtitleParam;
		m_sDebriefingParam = debriefingParam;
	}
}
