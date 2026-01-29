//------------------------------------------------------------------------------------------------
class SCR_XPInfoDisplay : SCR_InfoDisplayExtended
{
	protected static const int XP_INFO_DURATION = 10000;	//ms
	protected static const ResourceName RANK_ICON_IMAGESET = "{70E828A2F6EBE7D0}UI/Textures/Nametags/nametagicons.imageset";

	protected bool m_bInitDone;
	protected bool m_bIsInfoAllowed = true;

	protected SCR_XPHandlerComponent m_PlayerXPComponent;

	protected ref map<int, ref SCR_XPInfoWidgetData> m_Widgets = new map<int, ref SCR_XPInfoWidgetData>();
	
	protected string m_sCurrentRankName;
	protected string m_sCurrentRankIcon;
	
	protected string m_sNextRankIcon;
	
	protected SCR_ECharacterRank m_ECurrentRank;
	protected SCR_ECharacterRank m_EPrevRank;
	
	protected SCR_EXPRewards m_EReward;
	protected string m_sRewardName;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;
		
		m_PlayerXPComponent = SCR_XPHandlerComponent.Cast(gameMode.FindComponent(SCR_XPHandlerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		return (m_PlayerXPComponent != null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_bInitDone)
			return;
		
		m_bInitDone = true;
		
		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(owner.FindComponent(SCR_PlayerXPHandlerComponent));
		
		if (comp)
			comp.GetOnXPChanged().Insert(ShowXPInfo);
		
		SCR_TaskManagerUIComponent taskManager = SCR_TaskManagerUIComponent.GetInstance();
		if (taskManager)
			taskManager.GetOnTaskHUDVisible().Insert(ToggleXPInfo);
		
		SCR_XPInfoWidgetData hudXPWidget = new SCR_XPInfoWidgetData();
		hudXPWidget.InitWidgets(m_wRoot);
		AddNewWidgets(hudXPWidget, SCR_EXPInfoWidget.HUD);
	}
	
	void AddNewWidgets(notnull SCR_XPInfoWidgetData hudXPWidget, int id)
	{
		if (m_Widgets.Get(id))
			return;
		
		m_Widgets.Insert(id, hudXPWidget);
		
		if (m_sCurrentRankName == string.Empty)
			UpdateData();
		
		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (comp && factionManager)
		{
			int totalXP = comp.GetPlayerXP();
			
			hudXPWidget.UpdateXPTexts(false, false, 0, totalXP, false, m_EReward, m_sCurrentRankIcon, m_sCurrentRankName, m_sNextRankIcon, m_sRewardName);
			hudXPWidget.UpdateXPProgressBar(factionManager, m_ECurrentRank, m_EPrevRank, 0, totalXP, false);
		}
	}
	
	bool RemoveWidgetsById(int widgetDataId)
	{
		m_Widgets.Remove(widgetDataId);	
		
		return true;	
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSettingsChanged()
 	{               
 		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(m_HUDManager.GetInterfaceSettingsClass());
 		if (!interfaceSettings)
 	 		return;

		bool state;
		interfaceSettings.Get(m_sInterfaceSettingName, state);
		
		AllowShowingInfo(state);
	}
	
	//------------------------------------------------------------------------------------------------
	void AllowShowingInfo(bool allow)
	{
		m_bIsInfoAllowed = allow;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RecolorXPBar()
	{
		foreach(int id, SCR_XPInfoWidgetData widgetData: m_Widgets)
		{
			widgetData.RecolorXPBar();
		}
	}
 	
 	//------------------------------------------------------------------------------------------------
	protected void RecolorXPText()
	{
		foreach(int id, SCR_XPInfoWidgetData widgetData: m_Widgets)
		{
			widgetData.RecolorXPText();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideHUD()
	{
		Show(false, UIConstants.FADE_RATE_SLOW)
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowXPInfo(int totalXP, SCR_EXPRewards rewardID, int XP, bool volunteer, bool profileUsed, int skillLevel)
	{
		if (!m_bIsInfoAllowed)
			return;
		
		bool toggled = (XP == 0);
		bool nonSpecific = (rewardID == SCR_EXPRewards.UNDEFINED);
		bool notify = !nonSpecific && m_PlayerXPComponent.AllowNotification(rewardID);
		
		// XP has been added without specified source, don't show UI (i.e. XP sync upon reconnect)
		if (nonSpecific && !toggled)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			
		if (!faction)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!factionManager)
			return;
		
		m_ECurrentRank = factionManager.GetRankByXP(totalXP);
		SCR_ECharacterRank nextRank = factionManager.GetRankNext(m_ECurrentRank);
		
		m_EReward = rewardID;
		m_sCurrentRankIcon = faction.GetRankInsignia(m_ECurrentRank);
		m_sNextRankIcon = faction.GetRankInsignia(nextRank);
		m_sCurrentRankName = faction.GetRankName(m_ECurrentRank);
		m_sRewardName = m_PlayerXPComponent.GetXPRewardName(rewardID);
		m_EPrevRank = factionManager.GetRankByXP(totalXP - XP);
		
		foreach(int id, SCR_XPInfoWidgetData widgetData: m_Widgets)
		{
			widgetData.UpdateXPTexts(toggled, volunteer, XP, totalXP, notify, m_EReward, m_sCurrentRankIcon, m_sCurrentRankName, m_sNextRankIcon, m_sRewardName);
			widgetData.UpdateXPProgressBar(factionManager, m_ECurrentRank, m_EPrevRank, XP, totalXP, notify);
		}
		
		// Show skill info
		// Disabled for now, funcitonality is being transfered to SCR_PlayerData
		/*
		if (profileUsed && XP > 0)
		{
			LocalizedString skillName;
			
			switch (m_PlayerXPComponent.GetXPRewardSkill(rewardID))
			{
				case EProfileSkillID.WEAPON_HANDLER: {skillName = "#AR-Campaign_SkillWeaponSpecialist"; break;};
				case EProfileSkillID.DRIVER: {skillName = "#AR-Campaign_SkillDriver"; break;};
				case EProfileSkillID.SCOUT: {skillName = "#AR-Campaign_SkillScout"; break;};
				case EProfileSkillID.OPERATOR: {skillName = "#AR-Campaign_SkillOperator"; break;};
			}
			
			if (!skillName.IsEmpty())
				m_wSkill.SetTextFormat("#AR-Campaign_LevelInfo", skillName, skillLevel);
		}
		*/
		
		// We updated the UI even if we don't want to show it at this point
		// This way it's up to date when it gets toggled manually later
		if (!notify && !toggled)
			return;
		
		// Show the UI
		Show(true);
		
		if (!toggled)
		{
			// Hide and recolor the UI after a while
			GetGame().GetCallqueue().Remove(HideHUD);
			GetGame().GetCallqueue().Remove(RecolorXPBar);
			GetGame().GetCallqueue().Remove(RecolorXPText);
			GetGame().GetCallqueue().CallLater(HideHUD, XP_INFO_DURATION);
			GetGame().GetCallqueue().CallLater(RecolorXPBar, XP_INFO_DURATION);
			GetGame().GetCallqueue().CallLater(RecolorXPText, XP_INFO_DURATION);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ToggleXPInfo(bool visible)
	{
		// Make sure UI is not hidden automatically if shown a moment before
		GetGame().GetCallqueue().Remove(HideHUD);

		if (visible)
		{
			SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));
			if (!comp)
				return;

			ShowXPInfo(comp.GetPlayerXP(), SCR_EXPRewards.UNDEFINED, 0, false, false, 0);
		}
		else
		{
			Show(false, UIConstants.FADE_RATE_SUPER_FAST);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateData()
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
			
		if (!faction)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		
		if (!factionManager)
			return;
		
		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(m_PlayerController.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!comp)
			return;

		int totalXP = comp.GetPlayerXP();
		
		m_ECurrentRank = factionManager.GetRankByXP(totalXP);
		SCR_ECharacterRank nextRank = factionManager.GetRankNext(m_ECurrentRank);
		
		m_EReward = SCR_EXPRewards.UNDEFINED;
		m_sCurrentRankIcon = faction.GetRankInsignia(m_ECurrentRank);
		m_sCurrentRankName = faction.GetRankName(m_ECurrentRank);
		m_sRewardName = m_PlayerXPComponent.GetXPRewardName(m_EReward);
		m_EPrevRank = factionManager.GetRankByXP(totalXP);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Image set containing the rank icons
	static ResourceName GetRankIconImageSet()
	{
		return RANK_ICON_IMAGESET;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_XPInfoDisplay()
	{	
		SCR_TaskManagerUIComponent taskManager = SCR_TaskManagerUIComponent.GetInstance();
		if (taskManager)
			taskManager.GetOnTaskHUDVisible().Remove(ToggleXPInfo);
	}
};

enum SCR_EXPInfoWidget
{
	HUD,
	INVENTORY
}

class SCR_XPInfoWidgetData
{
	protected const string STRING_REWARD_VOLUNTEER = "#AR-Campaign_RewardBonus_Volunteer";
	
	protected TextWidget m_wTitle;
	protected TextWidget m_wRank;
	protected TextWidget m_wRankNoIcon;
	protected TextWidget m_wSkill;
	protected ImageWidget m_wRankIcon;
	protected ImageWidget m_wNextRankIcon;
	protected ImageWidget m_wNextRankArrowIcon;
	protected ProgressBarWidget m_wProgress;
	protected ProgressBarWidget m_wProgressDiff;
	
	void InitWidgets(notnull Widget root)
	{
		m_wTitle = TextWidget.Cast(root.FindAnyWidget("InfoText"));
		m_wRank = TextWidget.Cast(root.FindAnyWidget("Rank"));
		m_wRankIcon = ImageWidget.Cast(root.FindAnyWidget("RankIcon"));
		m_wNextRankIcon = ImageWidget.Cast(root.FindAnyWidget("NextRankIcon"));
		m_wNextRankArrowIcon = ImageWidget.Cast(root.FindAnyWidget("NextRankArrowIcon"));
		m_wRankNoIcon = TextWidget.Cast(root.FindAnyWidget("RankNoIcon"));
		m_wProgress = ProgressBarWidget.Cast(root.FindAnyWidget("XPProgress"));
		m_wProgressDiff = ProgressBarWidget.Cast(root.FindAnyWidget("XPProgressDiff"));		
		m_wSkill = TextWidget.Cast(root.FindAnyWidget("Skill"));
		
		m_wProgress.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
	}
	
	//------------------------------------------------------------------------------------------------
	void RecolorXPBar()
	{
		AnimateWidget.Color(m_wProgressDiff, Color.FromInt(UIColors.CONFIRM.PackToInt()), UIConstants.FADE_RATE_SLOW);
	}
 	
 	//------------------------------------------------------------------------------------------------
	void RecolorXPText()
	{
		AnimateWidget.Color(m_wTitle, Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()), UIConstants.FADE_RATE_SLOW);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateXPTexts(bool toggled, bool volunteer, int XP, int totalXP, bool notify, SCR_EXPRewards rewardID, string rankIconName, string rankText, string nextRankIconName, string rewardName)
	{
		if (rankIconName.IsEmpty())
		{
			m_wRankNoIcon.SetTextFormat(rankText);
			m_wRankIcon.SetVisible(false);
			m_wRank.SetTextFormat(string.Empty);
		}
		else
		{
			m_wRankNoIcon.SetText(string.Empty);
			m_wRankIcon.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), rankIconName);
			m_wRankIcon.SetColor(Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()));
			m_wRankIcon.SetVisible(true);
			m_wRank.SetTextFormat(rankText);
		}
		
		if (m_wNextRankIcon)
		{
			if (nextRankIconName.IsEmpty())
			{
				m_wNextRankIcon.SetVisible(false);
				m_wNextRankArrowIcon.SetVisible(false);
			}
			else
			{
				m_wNextRankIcon.SetVisible(true);
				m_wNextRankArrowIcon.SetVisible(true);
				
				m_wNextRankIcon.LoadImageFromSet(0, SCR_XPInfoDisplay.GetRankIconImageSet(), nextRankIconName);
				m_wNextRankIcon.SetColor(Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()));
			}
		}

		if (toggled || !notify)
		{
			m_wTitle.SetText(string.Empty);
			m_wProgressDiff.SetColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
		}
		else
		{
			if (volunteer)
				m_wTitle.SetTextFormat(STRING_REWARD_VOLUNTEER, rewardName);
			else
				m_wTitle.SetTextFormat(rewardName);

			// Show orange portion of the progress bar when XP gets added, red portion if substracted
			if (XP > 0)
			{
				m_wTitle.SetColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
				m_wProgressDiff.SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
			}
			else if (XP == 0)
			{
				m_wTitle.SetColor(Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()));
				m_wProgressDiff.SetColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
			}
			else
			{
				m_wTitle.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
				m_wProgressDiff.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateXPProgressBar(notnull SCR_FactionManager factionManager, SCR_ECharacterRank curRank, SCR_ECharacterRank prevRank, int XP, int totalXP, bool notify)
	{
		if (factionManager.GetRankNext(curRank) == SCR_ECharacterRank.INVALID)
		{
			// Player at max level, no gain to show
			m_wProgress.SetMin(0);
			m_wProgress.SetMax(1);
			m_wProgress.SetCurrent(1);
			m_wProgressDiff.SetMin(0);
			m_wProgressDiff.SetMax(1);
			m_wProgressDiff.SetCurrent(0);
		}
		else
		{
			if (factionManager.GetRankPrev(curRank) == SCR_ECharacterRank.INVALID && XP < 0)
			{
				// Player is renegade and losing XP, just show red bar
				m_wProgress.SetMin(0);
				m_wProgress.SetMax(1);
				m_wProgress.SetCurrent(0);
				m_wProgressDiff.SetMin(0);
				m_wProgressDiff.SetMax(1);
				m_wProgressDiff.SetCurrent(1);
			}
			else
			{
				int XPCurRank = factionManager.GetRequiredRankXP(curRank);
				int XPNextRank = factionManager.GetRequiredRankXP(factionManager.GetRankNext(curRank));

				if (curRank == prevRank)
				{
					if (factionManager.GetRankPrev(curRank) != SCR_ECharacterRank.INVALID)
					{
						// Standard XP change
						m_wProgress.SetMin(XPCurRank);
						m_wProgress.SetMax(XPNextRank);
						m_wProgressDiff.SetMin(XPCurRank);
						m_wProgressDiff.SetMax(XPNextRank);
					}
					else
					{
						// XP change as renegade, show progress towards a normal rank from current XP
						m_wProgress.SetMin(totalXP - XP);
						m_wProgress.SetMax(totalXP - XP + 1);
						m_wProgressDiff.SetMin(totalXP - XP);
						m_wProgressDiff.SetMax(XPNextRank);
					}

					// Progress bar setup
					if (notify)
					{
						if (XP > 0)
						{
							m_wProgress.SetCurrent(totalXP - XP);
							m_wProgressDiff.SetCurrent(totalXP);
						}
						else
						{
							m_wProgress.SetCurrent(totalXP);
							m_wProgressDiff.SetCurrent(totalXP - XP);
						}
					}
					else
					{
						m_wProgress.SetCurrent(totalXP);
					}
				}
				else
				{
					if (curRank > prevRank)
					{
						// Promotion
						m_wProgress.SetMin(0);
						m_wProgress.SetMax(1);
						m_wProgress.SetCurrent(0);
						m_wProgressDiff.SetMin(XPCurRank);
						m_wProgressDiff.SetMax(XPNextRank);
						m_wProgressDiff.SetCurrent(totalXP);
					}
					else
					{
						// Demotion
						m_wProgress.SetMin(XPCurRank);
						m_wProgress.SetMax(XPNextRank);
						m_wProgress.SetCurrent(totalXP);
						m_wProgressDiff.SetMin(0);
						m_wProgressDiff.SetMax(1);
						m_wProgressDiff.SetCurrent(1);
					}
				}
			}
		}
	}
}
