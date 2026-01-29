//#define DEBUG_PLAYERPROFILE
class SCR_CareerProfileOverviewUI: SCR_SubMenuBase
{
	protected Widget m_wFirstColumnWidget;
	protected Widget m_wCareerSpecializationsWidget;
	protected Widget m_wProfileNotFound;
	
	protected Widget m_wHud;
	protected SCR_CareerProfileHUD m_HudHandler;
	protected SCR_CareerSpecializationsUI m_CareerSpecializationsHandler;
	protected SCR_LoadoutPreviewComponent m_LoadoutPreviewHandler;
		
	[Attribute("", UIWidgets.Object, category: "Loadout Manager")]
	protected ref SCR_BasePlayerLoadout m_PlayerLoadout;
	
	[Attribute(params: "Stats layout")]
	protected ResourceName m_StatsLayout;
	
	[Attribute(params: "Header of Stats layout")]
	protected ResourceName m_HeaderStatsLayout;
	
	[Attribute(params: "Stats Progression layout")]
	protected ResourceName m_ProgressionStatsLayout;
	
	protected ref SCR_PlayerData m_PlayerData;
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_wFirstColumnWidget = GetRootWidget().FindAnyWidget("FirstColumn");
		m_wCareerSpecializationsWidget = GetRootWidget().FindAnyWidget("CareerSpecializations0");
		m_wProfileNotFound = GetRootWidget().FindAnyWidget("ProfileNotFound");
		
		if (!m_wFirstColumnWidget || !m_wCareerSpecializationsWidget || !m_wProfileNotFound)
			return;
		
		m_wHud = m_wFirstColumnWidget.FindAnyWidget("CareerProfileHUD0");
		if (!m_wHud)
			return;
		
		m_HudHandler = SCR_CareerProfileHUD.Cast(m_wHud.FindHandler(SCR_CareerProfileHUD));
		if (!m_HudHandler)
			return;
		m_HudHandler.PrepareHUD("", "RankTitleText", "BackgroundUserPicture", "CharacterLoadout", "LevelProgress", "PlayerLevel", "ProgressBar0", "");
		
		Widget m_wCharacterLoadout = m_wHud.FindAnyWidget("CharacterLoadout");
		if (!m_wCharacterLoadout)
			return;
		m_LoadoutPreviewHandler = SCR_LoadoutPreviewComponent.Cast(m_wCharacterLoadout.FindHandler(SCR_LoadoutPreviewComponent));
		m_LoadoutPreviewHandler.SetPreviewedLoadout(m_PlayerLoadout);
		
		m_CareerSpecializationsHandler = SCR_CareerSpecializationsUI.Cast(m_wCareerSpecializationsWidget.FindHandler(SCR_CareerSpecializationsUI));
		if (!m_CareerSpecializationsHandler)
			return;
		
		if (!m_PlayerData)
			m_PlayerData = new SCR_PlayerData(0, false); //We use ID 0 to indicate that it's the local user
			
		if (m_PlayerData.IsDataReady())
			FillFields();
		else
			m_PlayerData.GetDataReadyInvoker().Insert(FillFields);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("CareerProfileContext");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillFields()
	{
		if (!m_PlayerData || !m_PlayerData.IsDataReady())
		{
			Print("SCR_CareerEndScreenUI: Array of EarntPoints from player's PlayerData object is empty.", LogLevel.ERROR);
			return;
		}
		
		#ifdef DEBUG_PLAYERPROFILE
			m_PlayerData.DebugCalculateStats();
		#endif
		
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
		#ifdef DEBUG_PLAYERPROFILE
			Print("SCR_CareerProfileOverviewUI: Debugging FillHudAndStats!", LogLevel.DEBUG);
		#endif
		
		int Level = m_PlayerData.GetStat(SCR_EDataStats.LEVEL_EXPERIENCE);
		
		m_HudHandler.SetPlayerLevel(Level / SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetProgressBarValue(Level % SCR_PlayerDataConfigs.XP_NEEDED_FOR_LEVEL);
		m_HudHandler.SetPlayerRank(m_PlayerData.GetStat(SCR_EDataStats.RANK));
		m_HudHandler.SetRandomBackgroundPicture();
		
		Widget RankStatsWidget = m_wFirstColumnWidget.FindAnyWidget("RankStatEntries");
		if (!RankStatsWidget)
			return;
		
		#ifdef DEBUG_PLAYERPROFILE
			array<float> EarntPoints = m_PlayerData.GetArrayEarntPoints();
		
			m_HudHandler.SetLevelProgressGain(EarntPoints[SCR_EDataStats.LEVELEXPERIENCE]);
		
			float warCrimes = (m_PlayerData.GetWarCrimes()) *  (SCR_PlayerDataConfigs.WARCRIMESPUNISHMENT);
			int minutes = (m_PlayerData.GetSessionDuration() - m_PlayerData.GetSessionDuration(false)) / 60;
			SCR_CareerUI.CreateProgressionStatEntry(RankStatsWidget, m_ProgressionStatsLayout, "#AR-CareerProfile_TimePlayed", Math.Floor(EarntPoints[SCR_EDataStats.SESSIONDURATION] * warCrimes), Math.Floor(EarntPoints[SCR_EDataStats.SESSIONDURATION]), "#AR-CareerProfile_Minutes", ""+Math.Floor(minutes));	
		#else
			int days, hours, minutes, seconds;
			SCR_DateTimeHelper.GetDayHourMinuteSecondFromSeconds(m_PlayerData.GetStat(SCR_EDataStats.SESSION_DURATION), days, hours, minutes, seconds);
			
			SCR_CareerUI.CreateStatEntry(RankStatsWidget, m_StatsLayout, "#AR-CareerProfile_TimePlayed", "#AR-CareerProfile_TimePlayed_TotalValue", ""+days, ""+hours, ""+minutes);
			SCR_CareerUI.CreateStatEntry(RankStatsWidget, m_StatsLayout, "#AR-CareerProfile_Deaths", "#AR-CareerProfile_Times", ""+Math.Floor(m_PlayerData.GetStat(SCR_EDataStats.DEATHS)));
			SCR_CareerUI.CreateHeaderStatEntry(RankStatsWidget, m_HeaderStatsLayout, "#AR-CareerProfile_Distance");
			SCR_CareerUI.CreateStatEntry(RankStatsWidget, m_StatsLayout, "#AR-CareerProfile_DistanceTravelled_ByFoot", "#AR-CareerProfile_KMs", ""+Math.Floor(m_PlayerData.GetStat(SCR_EDataStats.DISTANCE_WALKED)/1000));
			SCR_CareerUI.CreateStatEntry(RankStatsWidget, m_StatsLayout, "#AR-CareerProfile_DistanceTravelled_AsDriver", "#AR-CareerProfile_KMs", ""+Math.Floor(m_PlayerData.GetStat(SCR_EDataStats.DISTANCE_DRIVEN)/1000));
			SCR_CareerUI.CreateStatEntry(RankStatsWidget, m_StatsLayout, "#AR-CareerProfile_DistanceTravelled_AsPassenger", "#AR-CareerProfile_KMs", ""+Math.Floor(m_PlayerData.GetStat(SCR_EDataStats.DISTANCE_AS_OCCUPANT)/1000));
		#endif	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FillSpecializationsFrame()
	{
		#ifdef DEBUG_PLAYERPROFILE
			Print("SCR_CareerProfileOverviewUI: Debugging FillSpecializationsFrame!", LogLevel.DEBUG);
			m_CareerSpecializationsHandler.SetShowProgression(true);
		#else
			m_CareerSpecializationsHandler.SetShowProgression(false);
		#endif
		
		m_CareerSpecializationsHandler.FillSpecializations(m_PlayerData, m_StatsLayout, m_HeaderStatsLayout, m_ProgressionStatsLayout);
		m_CareerSpecializationsHandler.FillWarCrimes();
		
		/*
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		ControllerActive(device, device);
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(ControllerActive);
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ControllerActive(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (newDevice == EInputDeviceType.GAMEPAD || newDevice == EInputDeviceType.JOYSTICK)
		{
			Print("Device is gamepad or joystick now", LogLevel.DEBUG);
			//m_CareerSpecializationsHandler.SetLeftAndRightUIActivate(true);
		}
		else
		{
			if (oldDevice != EInputDeviceType.GAMEPAD || newDevice != EInputDeviceType.JOYSTICK)
				return;
			Print("Device is mouse or keyboard now", LogLevel.DEBUG);
			//m_CareerSpecializationsHandler.SetLeftAndRightUIActivate(false);
		}
	}
};