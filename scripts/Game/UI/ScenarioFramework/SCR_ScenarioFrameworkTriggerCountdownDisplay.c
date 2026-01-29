//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_ScenarioFrameworkTriggerCountdownDisplay : SCR_InfoDisplayExtended
{
	protected bool m_bTriggerConditionsStatus;
	protected int m_iPlayersCountByFactionInside;
	protected int m_iPlayersCountByFaction;
	protected float m_fTempTimeSlice;
	protected float m_fActivationCountdownTimer;
	protected float m_fTempWaitTime;
	protected float m_fMinimumPlayersNeededPercentage;

	protected string m_sPlayerActivationNotificationTitle;
	protected Widget m_wInfoOverlay;
	protected Widget m_wCountdownOverlay;
	protected RichTextWidget m_wCountdown;
	protected RichTextWidget m_wFlavour;

	//------------------------------------------------------------------------------------------------
	//! Starts drawing the display
	override void DisplayStartDraw(IEntity owner)
	{
		SCR_CharacterTriggerEntity.s_OnTriggerUpdated.Insert(RetrieveTriggerInfo);
		SCR_CharacterTriggerEntity.s_OnTriggerUpdatedPlayerNotPresent.Insert(RetrieveCountInsideTrigger);
		m_wInfoOverlay = m_wRoot.FindAnyWidget("Info");
		m_wCountdownOverlay = m_wRoot.FindAnyWidget("Countdown");
		ImageWidget leftFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideBlue"));
		ImageWidget rightFlag = ImageWidget.Cast(m_wRoot.FindAnyWidget("FlagSideRed"));
		RichTextWidget leftScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreBlue"));
		RichTextWidget rightScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ScoreRed"));
		RichTextWidget winScore = RichTextWidget.Cast(m_wRoot.FindAnyWidget("TargetScore"));
		m_wCountdown = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CountdownWin"));
		m_wFlavour = RichTextWidget.Cast(m_wRoot.FindAnyWidget("FlavourText"));
		ImageWidget winScoreSideLeft = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveLeft"));
		ImageWidget winScoreSideRight = ImageWidget.Cast(m_wRoot.FindAnyWidget("ObjectiveRight"));

		m_wInfoOverlay.SetVisible(false);
		leftFlag.SetVisible(false);
		rightFlag.SetVisible(false);
		leftScore.SetVisible(false);
		rightScore.SetVisible(false);
		winScore.SetVisible(false);
		winScoreSideLeft.SetVisible(false);
		winScoreSideRight.SetVisible(false);

		ShowHUD(false);
		m_bIsEnabled = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Periodically updates the HUD
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		m_fTempTimeSlice += timeSlice;
		if (m_fTempTimeSlice > 1 && m_fTempWaitTime > -1)
			UpdateHUD();
	}

	//------------------------------------------------------------------------------------------------
	//! Toggles if HUD should be shown or hidden
	protected void ShowHUD(bool show)
	{
		m_wFlavour.SetVisible(show);
		m_wCountdown.SetVisible(show);
		m_wInfoOverlay.SetVisible(show);
		m_wCountdownOverlay.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles if HUD should be enabled or disabled
	void HandleHUD()
	{
		if (m_iPlayersCountByFactionInside > 0)
		{
			m_bIsEnabled = true;
		}
		else
		{
			m_bIsEnabled = false;
			ShowHUD(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handles if HUD should be enabled or disabled
	void RetrieveTriggerInfo(float activationCountdownTimer, float tempWaitTime, int playersCountByFactionInside, int playersCountByFaction, string playerActivationNotificationTitle, bool triggerConditionsStatus, float minimumPlayersNeededPercentage)
	{
		m_bTriggerConditionsStatus = triggerConditionsStatus;
		m_iPlayersCountByFactionInside = playersCountByFactionInside;
		m_iPlayersCountByFaction = playersCountByFaction;
		m_fActivationCountdownTimer = activationCountdownTimer;
		m_fTempWaitTime = tempWaitTime;
		m_sPlayerActivationNotificationTitle = playerActivationNotificationTitle;
		m_fMinimumPlayersNeededPercentage = minimumPlayersNeededPercentage;

		HandleHUD();
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves count inside trigger for turning off the HUD in cases where player owning this display is not inside
	void RetrieveCountInsideTrigger(int countInsideTrigger)
	{
		if (m_iPlayersCountByFactionInside == 0)
			return;

		m_iPlayersCountByFactionInside = countInsideTrigger;
		HandleHUD();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates HUD according to the values and conditions acquired from the trigger
	protected void UpdateHUD()
	{
		m_fTempTimeSlice = 0;
		
		int neededNumber = Math.Ceil(m_iPlayersCountByFaction * m_fMinimumPlayersNeededPercentage);
		if (neededNumber == 0)
			neededNumber = 1;
		
		string title = string.Format(WidgetManager.Translate(m_sPlayerActivationNotificationTitle), m_iPlayersCountByFactionInside, neededNumber);
		string formattedTitle = string.Format("<color rgba=\"226,168,80,255\">%1</color>", title);

		if (!m_bTriggerConditionsStatus)
		{
			if (m_iPlayersCountByFactionInside == 0)
			{
				ShowHUD(false);
				return;
			}
			m_wFlavour.SetText(formattedTitle);
			ShowHUD(true);
			m_wCountdownOverlay.SetVisible(false);
			return;
		}

		string shownTime = SCR_FormatHelper.GetTimeFormatting(m_fTempWaitTime, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES);
		m_wCountdown.SetText(shownTime);

		m_wFlavour.SetText(formattedTitle);
		ShowHUD(true);
		
		if (m_fTempWaitTime <= 0)
		{
			ShowHUD(false);
			m_bIsEnabled = false;
		}
	}
};
