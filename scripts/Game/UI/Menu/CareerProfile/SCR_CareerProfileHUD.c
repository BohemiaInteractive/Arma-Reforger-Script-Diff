//------------------------------------------------------------------------------------------------
class SCR_CareerProfileHUD: ScriptedWidgetComponent
{
	protected Widget m_wRootWidget;
	protected SCR_ButtonTextComponent m_EditAppearance;
	
	[Attribute()]
	protected ref array<ResourceName> m_aBackgroundImages;
	
	[Attribute()]
	protected ref array<ResourceName> m_aRankImages;
	
	protected ImageWidget m_RankImageWidget;
	protected RichTextWidget m_RankLevel;
	protected RichTextWidget m_LevelProgress, m_PlayerLevel;
	protected ImageWidget m_BackgroundUserPictureImageWidget;
	protected SCR_WLibProgressBarComponent m_ProgressBar;
	
	[Attribute("0", "auto", "Min progress bar value")]
	protected int m_iMinBarValue;
	
	[Attribute("100", "auto", "Value to display when progress is 0")]
	protected int m_iMinRepresentativeValue;
	
	[Attribute("10000", "auto", "Max progress bar value")]
	protected int m_iMaxBarValue;
	
	[Attribute("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et")]
	protected ResourceName previewManagerResource;
	
	[Attribute("{5B1996C05B1E51A4}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_AR.et")]
	protected ResourceName loadoutResource;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wRootWidget = w;
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareHUD(string rankImageName, string rankLevelName, string backgroundUserPictureName, string CharacterLoadoutName, string LevelProgressName, string PlayerLevelName, string ProgressBarName, string EditAppearanceButtonName)
	{
		if (!m_wRootWidget)
			return;
		
		if (rankImageName.IsEmpty())
			m_RankImageWidget = ImageWidget.Cast(m_wRootWidget.FindAnyWidget(rankImageName));
		
		if (rankLevelName != "")
			m_RankLevel = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget(rankLevelName));
		
		if (backgroundUserPictureName != "")
			m_BackgroundUserPictureImageWidget = ImageWidget.Cast(m_wRootWidget.FindAnyWidget(backgroundUserPictureName));
		
		if (CharacterLoadoutName != "")
		{
			Widget characterLoadout = m_wRootWidget.FindAnyWidget(CharacterLoadoutName);
			if (characterLoadout)
			{
				SCR_LoadoutPreviewComponent characterLoadoutHandler = SCR_LoadoutPreviewComponent.Cast(characterLoadout.FindHandler(SCR_LoadoutPreviewComponent));
				if (characterLoadoutHandler)
				{
					ItemPreviewManagerEntity previewManager = characterLoadoutHandler.GetPreviewManagerEntity();
					ItemPreviewWidget previewWidget = characterLoadoutHandler.GetItemPreviewWidget();
					
					if (previewManager && previewWidget)
					{
						characterLoadoutHandler.SetPreviewManagerEntity(previewManager);
						characterLoadoutHandler.SetItemPreviewWidget(previewWidget);
						
						previewManager.SetPreviewItemFromPrefab(previewWidget, loadoutResource);
						characterLoadoutHandler.SetReloadLoadout(false);
					}
				}
			}
		}
		
		if (ProgressBarName != "")
		{
			m_ProgressBar = SCR_WLibProgressBarComponent.GetProgressBar(ProgressBarName, m_wRootWidget, true);
		
			if (!m_ProgressBar)
				return;
			
			m_ProgressBar.SetMin(m_iMinBarValue);
			m_ProgressBar.SetMax(m_iMaxBarValue);
			
			if (LevelProgressName != "")
				m_LevelProgress = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget(LevelProgressName));
			
			if (PlayerLevelName != "")
				m_PlayerLevel = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget(PlayerLevelName));
		}
		
		if (EditAppearanceButtonName != "")
		{
			m_EditAppearance = SCR_ButtonTextComponent.GetButtonText(EditAppearanceButtonName, m_wRootWidget);
			if (!m_EditAppearance)
				return;
			
			m_EditAppearance.m_OnClicked.Insert(OnEditAppearance);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEditAppearance()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.CreditsMenu); //To remove
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLevelProgressGain(float f)
	{
		RichTextWidget levelProgressGain = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget("LevelGained"));
		if (!levelProgressGain)
			return;
		
		if (f != 0)
		{
			levelProgressGain.SetText("+"+f);
			levelProgressGain.SetEnabled(true);
			levelProgressGain.SetVisible(true);
			return;
		}
		
		levelProgressGain.SetText("");
		levelProgressGain.SetEnabled(false);
		levelProgressGain.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerRank(int rank)
	{
		if (rank < 0) 
			return;
		else if (rank == 0)
			rank = 1;
		
		if (m_RankLevel)
			m_RankLevel.SetTextFormat("#AR-CareerProfile_RankTitle",rank);
		
		if (m_RankImageWidget && m_aRankImages && m_aRankImages.Count() >= rank)
			SCR_WLibComponentBase.SetTexture(m_RankImageWidget, m_aRankImages[rank-1], string.Empty);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRandomBackgroundPicture()
	{
		if (m_BackgroundUserPictureImageWidget && m_aBackgroundImages.Count()>0)
		{
			m_BackgroundUserPictureImageWidget.SetColor(Color.FromSRGBA(255,255,255,204)); //60% opacity
			SCR_WLibComponentBase.SetTexture(m_BackgroundUserPictureImageWidget, m_aBackgroundImages[Math.RandomInt(0,m_aBackgroundImages.Count())], string.Empty);
		}
			
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerLevel(int n)
	{
		if (m_PlayerLevel)
			m_PlayerLevel.SetTextFormat("#AR-CareerProfile_PlayerLv",n);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProgressBarValue(int n)
	{
		if (n < m_iMinBarValue || n > m_iMaxBarValue)
			return;
		
		if (m_LevelProgress)
			m_LevelProgress.SetText ("" + n + "/" + m_iMaxBarValue);
		
		if (m_ProgressBar)
		{
			if (n > m_iMinRepresentativeValue)
				m_ProgressBar.SetValue(n);
			else 
				m_ProgressBar.SetValue(m_iMinRepresentativeValue);
		}
	}
}