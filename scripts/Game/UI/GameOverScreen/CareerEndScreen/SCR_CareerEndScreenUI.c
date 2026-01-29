//------------------------------------------------------------------------------------------------
class SCR_CareerEndScreenUI: SCR_BaseGameOverTabUIComponent 
{
	protected Widget m_wRootWidget;
	protected Widget m_wFirstColumnWidget;
	protected Widget m_wCareerSpecializationsWidget;
	protected Widget m_wProfileNotFound;
	
	protected Widget m_wHud;
	protected SCR_CareerProfileHUD m_HudHandler;
	protected SCR_CareerSpecializationsUI m_CareerSpecializationsHandler;
	
	[Attribute(params: "Stats layout")]
	protected ResourceName m_StatsLayout;
	
	[Attribute(params: "Header of Stats layout")]
	protected ResourceName m_HeaderStatsLayout;
	
	[Attribute(params: "Stats Progression layout")]
	protected ResourceName m_ProgressionStatsLayout;
	
	protected static ref SCR_PlayerData m_PlayerData;
	
	//------------------------------------------------------------------------------------------------
	override void GameOverTabInit(notnull SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		super.GameOverTabInit(endScreenUIContent);
		
		m_wFirstColumnWidget = m_wRootWidget.FindAnyWidget("FirstColumn");
		m_wCareerSpecializationsWidget = m_wRootWidget.FindAnyWidget("CareerSpecializations0");
		m_wProfileNotFound = m_wRootWidget.FindAnyWidget("ProfileNotFound");
		
		if (!m_wFirstColumnWidget || !m_wCareerSpecializationsWidget || !m_wProfileNotFound)
			return;
		
		m_wHud = m_wFirstColumnWidget.FindAnyWidget("CareerProfileHUD0");
		
		if (!m_wHud)
			return;
		
		m_HudHandler = SCR_CareerProfileHUD.Cast(m_wHud.FindHandler(SCR_CareerProfileHUD));
		if (!m_HudHandler)
			return;
		m_HudHandler.PrepareHUD("", "RankTitleText", "", "", "LevelProgress", "PlayerLevel", "ProgressBar0", "");
		
		m_CareerSpecializationsHandler = SCR_CareerSpecializationsUI.Cast(m_wCareerSpecializationsWidget.FindHandler(SCR_CareerSpecializationsUI));
		if (!m_CareerSpecializationsHandler)
			return;
		
		//~ Sets winning faction or any other text and icon linked to the end screen
		SetEndscreenVisualInfo(endScreenUIContent);
		
		if (!m_PlayerData)
		{
			SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
			if (!dataCollector)
			{
				Print ("SCR_CareerEndScreenUI: No data collector was found.", LogLevel.ERROR);
				return;
			}
			
			m_PlayerData = dataCollector.GetPlayerData(0, false);
			
			//If there's still no player data, we wait for the invoker on data received to let us now that we got the instance through rpl
			if (!m_PlayerData)
			{
				SCR_DataCollectorCommunicationComponent communicationComponent = SCR_DataCollectorCommunicationComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_DataCollectorCommunicationComponent));
				if (communicationComponent)
					communicationComponent.GetOnDataReceived().Insert(OnDataReceived);
			}
			else if (!m_PlayerData.IsDataProgressionReady())
				m_PlayerData.CalculateStatsChange();
		}
		
		FillFields();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRootWidget = w;
		
	}
	
	//------------------------------------------------------------------------------------------------
	//! Data was sent through RPL and now we can use it
	protected void OnDataReceived(SCR_PlayerData playerData)
	{
		m_PlayerData = playerData;
		m_PlayerData.CalculateStatsChange();
		FillFields();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillFields()
	{
		if (!m_PlayerData)
		{
			FillScreen(false);
			return;
		}
		if (!m_PlayerData.IsDataProgressionReady())
		{
			Print("SCR_CareerEndScreenUI: Array of EarntPoints from player's PlayerData object is empty.", LogLevel.ERROR);
			return;
		}
		
		FillScreen(!m_PlayerData.IsEmptyProfile());
	}
	
	//------------------------------------------------------------------------------------------------
	void FillScreen(bool profileDataFound)
	{
		if (profileDataFound)
		{
			m_wFirstColumnWidget.SetVisible(true);
			m_wCareerSpecializationsWidget.SetVisible(true);
			m_wProfileNotFound.SetVisible(false);
			FillHudAndStats();
			FillSpecializationsFrame();
		}
		else
		{
			m_wFirstColumnWidget.SetVisible(false);
			m_wCareerSpecializationsWidget.SetVisible(false);
			m_wProfileNotFound.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillHudAndStats()
	{
		int Level = m_PlayerData.GetStat(SCR_EDataStats.LEVEL_EXPERIENCE);
		
		m_HudHandler.SetPlayerLevel(Level / SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetProgressBarValue(Level % SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetPlayerRank(m_PlayerData.GetStat(SCR_EDataStats.RANK));
		m_HudHandler.SetRandomBackgroundPicture();
		
		array<float> EarntPoints = m_PlayerData.GetArrayEarntPoints();
		
		m_HudHandler.SetLevelProgressGain(EarntPoints[SCR_EDataStats.LEVEL_EXPERIENCE]);
		
		Widget GeneralStatsWidget = m_wFirstColumnWidget.FindAnyWidget("GeneralStatEntries");
		if (!GeneralStatsWidget)
			return;
		
		//warcrimes = Punishment * "AreThereWarCrimes?"
		float warCrimes = 0;
		if (m_PlayerData.GetStat(SCR_EDataStats.WARCRIMES) != 0)
			warCrimes = SCR_PlayerDataConfigs.WARCRIMES_PUNISHMENT;
		
		int minutes = (m_PlayerData.GetStat(SCR_EDataStats.SESSION_DURATION) - m_PlayerData.GetStat(SCR_EDataStats.SESSION_DURATION, false)) / 60;
		
		SCR_CareerUI.CreateProgressionStatEntry(GeneralStatsWidget, m_ProgressionStatsLayout, "#AR-CareerProfile_TimePlayed", EarntPoints[SCR_EDataStats.SESSION_DURATION] * warCrimes, EarntPoints[SCR_EDataStats.SESSION_DURATION], "#AR-CareerProfile_Minutes", ""+minutes);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillSpecializationsFrame()
	{
		m_CareerSpecializationsHandler.SetShowProgression(true);
		m_CareerSpecializationsHandler.FillSpecializations(m_PlayerData, m_StatsLayout, m_HeaderStatsLayout, m_ProgressionStatsLayout);
		m_CareerSpecializationsHandler.FillWarCrimes();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetEndscreenVisualInfo(notnull SCR_GameOverScreenUIContentData endScreenUIContent)
	{
		ImageWidget matchFlag = ImageWidget.Cast(m_wFirstColumnWidget.FindAnyWidget("MatchResultFlag"));
		RichTextWidget MatchFactionWinner = RichTextWidget.Cast(m_wFirstColumnWidget.FindAnyWidget("MatchFactionWinner"));
		if (!matchFlag || !MatchFactionWinner)
			return;
		
		//~ Set match done text
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(endScreenUIContent.m_sSubtitle))
			MatchFactionWinner.SetTextFormat(endScreenUIContent.m_sSubtitle, endScreenUIContent.m_sSubtitleParam);
		else 
			MatchFactionWinner.SetVisible(false);
		
		//~ Set flag icon
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(endScreenUIContent.m_sIcon))
			matchFlag.LoadImageTexture(0, endScreenUIContent.m_sIcon);
		else 
			matchFlag.SetVisible(false);
	}
};