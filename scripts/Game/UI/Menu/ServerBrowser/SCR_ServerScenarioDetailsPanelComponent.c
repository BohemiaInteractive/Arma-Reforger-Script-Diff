/*
Scenaraio panel with additional mods data.
Used mainly in server browser for displaying server(room) scenario and it's mods data
*/

class SCR_ServerScenarioDetailsPanelComponent : SCR_ScenarioDetailsPanelComponent
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "edds")]
	protected string m_sDefaultScenarioImg;
	
	// Widget Names 
	protected const string WIDGET_MODS_COUNT = "ModsCount";
	protected const string WIDGET_MODS_LOADING = "LoadingMods";
	protected const string WIDGET_MODS_ICON = "ModsSizeIcon";
	protected const string WIDGET_MODS_SIZE = "ModsSizeText";
	protected const string WIDGET_MODS_LOCKED_ICON = "ModsLockedIcon";
	
	// Localized strings 
	protected const string STR_VERSION_MISMATCH = "#AR-ServerBrowser_JoinModVersionMissmatch";
	protected const string STR_UNJOINABLE = "#AR-ServerBrowser_NoneServers";
	
	protected const string STR_DEFAULT_VERSION = "0.0.0.0";
	
	// Mods Widgets 
	protected ImageWidget m_wImgModsIcon;
	protected ImageWidget m_wImgModsLocked;
	protected TextWidget m_wTxtModsCount;
	protected TextWidget m_wModsSizeText;
	protected Widget m_wImageCrossplay;
	
	protected SCR_LoadingOverlay m_ModsLoading;

	// Server additional data 
	protected Room m_Room;
	protected SCR_RoomModsManager m_ModsManager;
	protected bool m_bHideScenarioImg; 
	
	//------------------------------------------------------------------------------------------------
	// Override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Get additional 
		m_wTxtModsCount = TextWidget.Cast(w.FindAnyWidget(WIDGET_MODS_COUNT));
		m_wModsSizeText = TextWidget.Cast(w.FindAnyWidget(WIDGET_MODS_SIZE));
		m_wImgModsIcon = ImageWidget.Cast(w.FindAnyWidget(WIDGET_MODS_ICON));
		m_wImgModsLocked = ImageWidget.Cast(w.FindAnyWidget(WIDGET_MODS_LOCKED_ICON));
		
		Widget wModsLoading = w.FindAnyWidget(WIDGET_MODS_LOADING);
		if (wModsLoading)
			m_ModsLoading = SCR_LoadingOverlay.Cast(wModsLoading.FindHandler(SCR_LoadingOverlay));
		
		m_wImageCrossplay = AddTypeDisplay("platform-crossplay", UIConstants.ICONS_IMAGE_SET, UIConstants.ICONS_GLOW_IMAGE_SET);
		
		if (!GetGame().InPlayMode())
			return;
		
		// Setup widgets 
		SetDefaultScenario("");
		HideModsWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetScenario(MissionWorkshopItem scenario)
	{
		if (m_BackendImageComponent)
		{
			m_BackendImageComponent.GetOnImageSelected().Remove(OnScenarioBackendImageSelected);
			m_BackendImageComponent.GetOnImageSelected().Insert(OnScenarioBackendImageSelected);
		}
		
		DisplayRoomDataScenario(m_Room);
		
		// Update scenario
		m_Scenario = scenario;	
		if (scenario)
		{
			SCR_MissionHeader header = SCR_MissionHeader.Cast(scenario.GetHeader());
			m_Header = header;
		}
		else
			m_Header = null;
		
		UpdateAllWidgets();
		
		// Set room version
		string strVersion = UIConstants.FormatVersion(STR_DEFAULT_VERSION);
		
		if (!m_Room || !m_Room.Joinable())
			strVersion = STR_UNJOINABLE + "!";

		else if (m_Room.GameVersion())
		{
			strVersion = UIConstants.FormatVersion(m_Room.GameVersion());
		
			if (m_Room.GameVersion() != GetGame().GetBuildVersion())
				strVersion += " - " + STR_VERSION_MISMATCH + "!";
		}
		
		m_CommonWidgets.m_wAuthorNameText.SetText(strVersion);
		
		m_CommonWidgets.m_wNameImg.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnScenarioBackendImageSelected()
	{
		if (m_BackendImageComponent)
			m_BackendImageComponent.GetOnImageSelected().Remove(OnScenarioBackendImageSelected);
		
		m_CommonWidgets.m_wLoadingOverlay.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHideScenarioImg(bool hideContent) { m_bHideScenarioImg = hideContent; }
	
	//------------------------------------------------------------------------------------------------
	//! Change image behavior 
	override protected void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		if (m_bHideScenarioImg)
			m_CommonWidgets.m_wImage.LoadImageTexture(0, m_sDefaultScenarioImg, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	// public functions 
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Find default scenario content by given tag 
	//! Should display fallback text when no scenario is selected for various reasons 
	void SetDefaultScenario(string contentTag)
	{
		// Get content 
		DetailsPanelContentPreset content = FallbackContentByTag(contentTag);
		if (!content)
		{
			#ifdef SB_DEBUG
			Print("[SCR_ServerScenarioDeatilsPanelComponent] No fallback content was found!");
			#endif
			return;	
		}

		// Set fallback image 
		if (!content.m_sImage.IsEmpty())
			m_CommonWidgets.m_wImage.LoadImageTexture(0, content.m_sImage);
		else
		{
			if (m_FallbackContent && m_FallbackContent.m_DefaultContent)
			{
				if (!m_FallbackContent.m_DefaultContent.m_sImage.IsEmpty())
					m_CommonWidgets.m_wImage.LoadImageTexture(0, m_FallbackContent.m_DefaultContent.m_sImage);
			}
		}
		
		// Correct image size 
		int sx, sy;
		m_CommonWidgets.m_wImage.GetImageSize(0, sx, sy);
		m_CommonWidgets.m_wImage.SetSize(sx, sy);
		m_CommonWidgets.m_wLoadingOverlay.SetVisible(false);
		
		// Title 
		string title = content.m_sTitle;

		m_CommonWidgets.m_wNameText.SetText(title);
		if (!content.m_sTitleImageName.IsEmpty())
		{
			m_CommonWidgets.m_wNameImg.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, content.m_sTitleImageName);
			m_CommonWidgets.m_wNameImg.SetColor(content.m_sTitleImageColor);
			m_CommonWidgets.m_wNameImg.SetVisible(true);
		}
		
		// Description 
		m_CommonWidgets.m_wDescriptionText.SetText(content.m_sDescription);
		
		// Show default description 
		m_CommonWidgets.m_wImage.SetVisible(true);
		m_CommonWidgets.m_wNameText.SetVisible(true);
		m_CommonWidgets.m_wDescriptionText.SetVisible(true);
		
		m_CommonWidgets.m_wAuthorNameText.SetVisible(false);
		
		if(m_wImageCrossplay)
			m_wImageCrossplay.SetVisible(false);
		
		HideModsWidgets();
	}

	//------------------------------------------------------------------------------------------------
	void DisplayRoomData(Room room, bool showMods = true)
	{
		DisplayRoomDataScenario(room);
		m_CommonWidgets.m_wTopSize.SetOpacity(1);
		
		// Display mods loading 
		HideModsWidgets();
		
		bool show = room.IsModded() && showMods;
		
		m_ModsLoading.SetShown(show);
		m_wImgModsLocked.SetVisible(false);
		
		if (!room.IsModded())
			return;
		
		// Loading for public modded server
		if (!room.PasswordProtected())
		{
			m_ModsLoading.SetShown(show);
		}
		else
		{
			// Warning for protected modded server
			if (!room.IsAuthorized())
			{
				m_ModsLoading.SetShown(false);
		
				m_wImgModsIcon.SetVisible(true);
				m_wImgModsLocked.SetVisible(true);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------
	protected void DisplayRoomDataScenario(Room room)
	{
		m_Room = room;
		
		// Check room 
		if (!m_Room)
			return;

		// Set room version
		string strVersion = UIConstants.FormatVersion(STR_DEFAULT_VERSION);
		if (room.GameVersion())
			strVersion = UIConstants.FormatVersion(room.GameVersion());
		
		// Scenario 
		string scenarioDescription;
		if (m_Scenario)
			scenarioDescription	= m_Scenario.Description();
		
		// Set content 
		string title = room.ScenarioName();
		//title.ToUpper();
		m_CommonWidgets.m_wNameText.SetText(title);
		m_CommonWidgets.m_wDescriptionText.SetText(scenarioDescription);
		m_CommonWidgets.m_wAuthorNameText.SetText(strVersion);
		
		// Version match 
		if (room.GameVersion() == GetGame().GetBuildVersion() && room.Joinable())
		{
			m_CommonWidgets.m_wAuthorNameText.SetColor(Color.FromInt(UIColors.SUB_HEADER.PackToInt()));
		}
		else
		{
			m_CommonWidgets.m_wAuthorNameText.SetColor(Color.FromInt(UIColors.WARNING.PackToInt()));
			
			string text = m_CommonWidgets.m_wAuthorNameText.GetText();
			if(room.Joinable())
				text += " - " + STR_VERSION_MISMATCH + "!";
			
			m_CommonWidgets.m_wAuthorNameText.SetText(text);
		}
		
		m_CommonWidgets.m_wNameText.SetVisible(true);
		m_CommonWidgets.m_wAuthorNameText.SetVisible(true);		
		m_CommonWidgets.m_wLoadingOverlay.SetVisible(true);
		
		if (m_wImageCrossplay)
			m_wImageCrossplay.SetVisible(m_Room.IsCrossPlatform());
	}
	
	//-----------------------------------------------------------------------------------
	void DisplayDefaultScenarioImage()
	{
		if (m_BackendImageComponent)
			m_BackendImageComponent.SetImage(null);
		
		m_CommonWidgets.m_wLoadingOverlay.SetVisible(false);
	}

	//-----------------------------------------------------------------------------------
	//! Hide all widgets displaying info about mods
	//! Used mostly when server is not modded or not selected 
	protected void HideModsWidgets()
	{	
		m_wTxtModsCount.SetVisible(false);
		m_wImgModsIcon.SetVisible(false);
		m_wModsSizeText.SetVisible(false);
		
		m_ModsLoading.SetShown(false);
	}
	
	//-----------------------------------------------------------------------------------
	//! Set mods to display in panel
	void DisplayMods()
	{
		//m_aMods = mods;
		
		// Check widgets  
		if (!m_wTxtModsCount)
			return;
		
		// Get values 
		array<ref SCR_WorkshopItem> allMods = m_ModsManager.GetRoomItemsScripted();
		
		int count = allMods.Count();
		m_ModsLoading.SetShown(false);
		
		// Show UGC restricted message
		if (count > 0 && !SCR_AddonManager.GetInstance().GetUgcPrivilege())
		{
			m_wModsSizeText.SetText("#AR-ServerBrowser_ContentNotAllowed");
			m_wModsSizeText.SetColor(Color.FromInt(UIColors.WARNING.PackToInt())); 
			m_wModsSizeText.SetVisible(true);
			return;
		}
		
		// Hide mods widget if no mods 
		bool show = (count != 0);
		
		m_wImgModsIcon.SetVisible(show);
		m_wTxtModsCount.SetVisible(show);
		m_wModsSizeText.SetVisible(show);
		m_wModsSizeText.SetColor(Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt())); 
		
		if (!show)
			return;
		
		string totalSize = m_ModsManager.GetModListSizeString(allMods);
		
		// Set Text 
		m_wTxtModsCount.SetText(count.ToString());
		m_wModsSizeText.SetText(totalSize);
		m_wModsSizeText.Update();
		
		// Check mods to update size 
		array<ref SCR_WorkshopItem> toUpdateMods = m_ModsManager.GetRoomItemsToUpdate();
		
		// Check to update mods count 
		if (toUpdateMods.IsEmpty())
			return;
	}
	
	//-----------------------------------------------------------------------------------
	//! Quick display mods count
	void DisplayModsCount(int count)
	{
		// Show if modded
		if (count != 0)
		{
			m_wTxtModsCount.SetText(count.ToString());
		
			// Set widgets 
			m_wTxtModsCount.SetVisible(true);
			m_wImgModsIcon.SetVisible(true);
			m_wModsSizeText.SetVisible(false);
		}
	}
	
	//-----------------------------------------------------------------------------------
	void SetModsManager(SCR_RoomModsManager manager)
	{ 
		m_ModsManager = manager;
	}
}