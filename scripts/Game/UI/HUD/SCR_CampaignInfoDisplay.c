//------------------------------------------------------------------------------------------------
class SCR_CampaignInfoDisplay : SCR_InfoDisplayExtended
{
	protected static const int HUD_DURATION = 15000;
	protected static const int SIZE_NORMAL = 20;
	protected static const int SIZE_WINNER = 24;
	
	protected bool m_bInitDone;
	protected bool m_bPeriodicRefresh;
	
	protected SCR_GameModeCampaign m_Campaign;
	
	protected Widget m_wCountdownOverlay;
	
	protected ImageWidget m_wLeftFlag;
	protected ImageWidget m_wRightFlag;
	protected ImageWidget m_wWinScoreSideLeft;
	protected ImageWidget m_wWinScoreSideRight;
	
	protected RichTextWidget m_wLeftScore;
	protected RichTextWidget m_wRightScore;
	protected RichTextWidget m_wWinScore;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		m_Campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		
		if (m_Campaign)
			m_Campaign.GetOnMatchSituationChanged().Insert(UpdateHUD);
		
		return (m_Campaign != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_bInitDone)
			return;
		
		m_bInitDone = true;
		
		if (m_Campaign.IsTutorial())
			Show(false);
		
		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		m_wLeftFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideBlue"));
		m_wRightFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideRed"));
		m_wLeftScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreBlue"));
		m_wRightScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreRed"));
		m_wWinScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TargetScore"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		m_wWinScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		m_wWinScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));
		
		SCR_Faction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_Faction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
		
		m_wLeftFlag.LoadImageTexture(0, factionBLUFOR.GetFactionFlag());
		m_wRightFlag.LoadImageTexture(0, factionOPFOR.GetFactionFlag());
		
		UpdateHUD();
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_bPeriodicRefresh)
			return;
		
		UpdateHUD();
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void HideHUD()
	{
		Show(false, UIConstants.FADE_RATE_SLOW)
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHUDValues()
	{
		int winningFactionId = m_Campaign.GetWinningFactionId();
		int controlPointsBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetControlPointsHeld();
		int controlPointsOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetControlPointsHeld();
		int controlPointsThreshold = m_Campaign.GetControlPointTreshold();
		
		m_wLeftScore.SetText(controlPointsBLUFOR.ToString());
		m_wRightScore.SetText(controlPointsOPFOR.ToString());
		m_wWinScore.SetText(controlPointsThreshold.ToString());
		
		if (winningFactionId == -1)
		{
			m_wFlavour.SetVisible(false);
			m_wCountdownOverlay.SetVisible(false);
			m_wLeftScore.SetDesiredFontSize(SIZE_NORMAL);
			m_wRightScore.SetDesiredFontSize(SIZE_NORMAL);
			m_wWinScoreSideRight.SetColor(Color.FromInt(Color.WHITE));
			m_wWinScoreSideLeft.SetColor(Color.FromInt(Color.WHITE));
			m_wRightScore.SetColor(Color.FromInt(Color.WHITE));
			m_wLeftScore.SetColor(Color.FromInt(Color.WHITE));
			m_wWinScore.SetColor(Color.FromInt(Color.WHITE));
			SCR_PopUpNotification.GetInstance().Offset(false);
		}
		else
		{
			WorldTimestamp victoryTimestamp = m_Campaign.GetVictoryTimestamp();
			WorldTimestamp pauseTimestamp = m_Campaign.GetVictoryPauseTimestamp();
			bool isPaused = (pauseTimestamp != 0);
			float winCountdown;
			
			SCR_Faction factionOPFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.OPFOR);
			
			if (isPaused)
			{
				winCountdown = victoryTimestamp.DiffMilliseconds(pauseTimestamp);
				m_wCountdown.SetColor(Color.FromInt(Color.GRAY));
			}
			else
			{
				ChimeraWorld world = GetGame().GetWorld();
				winCountdown = victoryTimestamp.DiffMilliseconds(world.GetServerTimestamp());
				m_wCountdown.SetColor(Color.FromInt(Color.WHITE));
				m_bPeriodicRefresh = true;
			}
			
			winCountdown = Math.Max(0, Math.Ceil(winCountdown / 1000));
			Faction winner = GetGame().GetFactionManager().GetFactionByIndex(winningFactionId);
			string shownTime = SCR_FormatHelper.GetTimeFormatting(winCountdown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
			m_wCountdown.SetText(shownTime);
			SCR_PopUpNotification.GetInstance().Offset(true);
			
			m_wFlavour.SetTextFormat("#AR-ConflictHUD_FlavourText_IsWinning", winner.GetFactionName());
			m_wWinScore.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
			m_wWinScoreSideRight.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
			m_wWinScoreSideLeft.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
			
			if (winner == factionOPFOR)
			{
				m_wRightScore.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				m_wLeftScore.SetColor(Color.FromInt(Color.WHITE));
				m_wRightScore.SetDesiredFontSize(SIZE_WINNER);
				m_wLeftScore.SetDesiredFontSize(SIZE_NORMAL);
			}
			else
			{
				m_wLeftScore.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
				m_wRightScore.SetColor(Color.FromInt(Color.WHITE));
				m_wLeftScore.SetDesiredFontSize(SIZE_WINNER);
				m_wRightScore.SetDesiredFontSize(SIZE_NORMAL);
			}
			
			m_wCountdownOverlay.SetVisible(true);
			m_wFlavour.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateHUD()
	{
		m_bPeriodicRefresh = false;
		
		if (!m_wRoot || !m_bInitDone)
			return;
		
		GetGame().GetCallqueue().Remove(HideHUD);
		
		if (m_Campaign.IsTutorial())
			return;
		else
			Show(true, UIConstants.FADE_RATE_FAST);
		
		if (m_Campaign.GetIsMatchOver())
		{
			Show(false);
			return;
		}
		
		UpdateHUDValues();
		
		if (!m_bPeriodicRefresh && !m_Campaign.GetVictoryTimestamp())
			GetGame().GetCallqueue().CallLater(HideHUD, HUD_DURATION);
	}
};