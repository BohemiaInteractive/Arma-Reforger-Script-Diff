//! This component handles server entry and visiualization of server data
class SCR_ServerBrowserEntryComponent : SCR_BrowserListMenuEntryComponent
{
	// Attributes
	[Attribute("999")]
	protected int m_iPingLimit;
	
	[Attribute(SCR_WorkshopUiCommon.ICON_DOWNLOAD)]
	protected string m_sTooltipDownloadIcon;
	
	[Attribute(UIConstants.ACTION_DISPLAY_ICON_SCALE_HUGE)]
	protected float m_fTooltipDownloadIconScale;

	[Attribute("", UIWidgets.Object)]
	protected ref array<ref ServerBrowserEntryProperty> m_aPingStates;
	
	// Const
	protected const string LAYOUT_CONTENT =		"HorizontalLayout";
	protected const string LAYOUT_LOADING =		"Loading";
	
	protected const string TEXT_WIDGET_QUEUE =	"Queue";
	
	protected const string BUTTON_JOIN = 		"JoinButton";
	protected const string BUTTON_PASSWORD = 	"PasswordButton";
	
	protected const string ICON_WARNING =		"VersionWarningIcon";
	protected const string ICON_UNJOINABLE = 	"JoinWarningIcon";
	protected const string ICON_MODDED = 		"ImageModded";
	protected const string ICON_PING = 			"ImgPing";
	
	protected const string FRAME_NAME = 		"FrameName";
	protected const string FRAME_SCENARIO = 	"FrameScenario";
	
	protected const string TOOLTIP_JOIN = 				"Join";
	protected const string TOOLTIP_VERSION_MISMATCH = 	"VersionMismatch";
	
	// Base
	protected Room m_RoomInfo;
	protected SCR_EServerEntryProperty m_iProperties;

	protected SCR_RoomModsManager m_ModsManager;
	protected string m_sPatchSize;
	protected bool m_bIsPatchSizeLoaded;
	protected int m_iHighestPing;
	
	// Widgets
	protected TextWidget m_wQueue;
	protected Widget m_wHorizontalContent;
	protected Widget m_wLoading;
	protected Widget m_wUnjoinableIcon;
	protected ImageWidget m_wImgPing;
	protected Widget m_wImageModded;
	protected Widget m_wJoinButton;
	protected Widget m_wPasswordButton;
	protected Widget m_wVersionWarningIcon;
	
	protected SCR_ModularButtonComponent m_JoinButton;
	protected SCR_ModularButtonComponent m_PasswordButton;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
			return;

		// Get wrappers
		m_wHorizontalContent = w.FindAnyWidget(LAYOUT_CONTENT);
		m_wLoading = w.FindAnyWidget(LAYOUT_LOADING);

		// Setup favorite button
		m_wVersionWarningIcon = w.FindAnyWidget(ICON_WARNING);
		m_wUnjoinableIcon = w.FindAnyWidget(ICON_UNJOINABLE);

		// Property images and buttons
		m_wImageModded = w.FindAnyWidget(ICON_MODDED);
		m_wJoinButton = w.FindAnyWidget(BUTTON_JOIN);
		m_wPasswordButton = w.FindAnyWidget(BUTTON_PASSWORD);

		// Mouse interaction buttons
		m_JoinButton = SCR_ModularButtonComponent.FindComponent(m_wJoinButton);
		if (m_JoinButton)
		{
			m_JoinButton.m_OnClicked.Insert(OnJoinInteractionButtonClicked);
			m_aMouseButtons.Insert(m_JoinButton);
		}

		m_PasswordButton = SCR_ModularButtonComponent.FindComponent(m_wPasswordButton);
		if (m_PasswordButton)
		{
			m_PasswordButton.m_OnClicked.Insert(OnJoinInteractionButtonClicked);
			m_aMouseButtons.Insert(m_PasswordButton);
		}

		// Ping
		m_wImgPing = ImageWidget.Cast(w.FindAnyWidget(ICON_PING));

		// Name
		SCR_HorizontalScrollAnimationComponent scrollComp;
		Widget frameName = w.FindAnyWidget(FRAME_NAME);
		if (frameName)
		{
			scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(frameName.FindHandler(SCR_HorizontalScrollAnimationComponent));
			if (scrollComp)
				m_aScrollAnimations.Insert(scrollComp);
		}

		// Scenario
		Widget frameScenario = w.FindAnyWidget(FRAME_SCENARIO);
		if (frameScenario)
		{
			scrollComp = SCR_HorizontalScrollAnimationComponent.Cast(frameScenario.FindHandler(SCR_HorizontalScrollAnimationComponent));
			if (scrollComp)
				m_aScrollAnimations.Insert(scrollComp);
		}

		// Loading
		m_wLoading.SetVisible(false);

		// Get highest ping
		foreach (ServerBrowserEntryProperty pingState : m_aPingStates)
		{
			int ping = pingState.m_sValue.ToInt();
			if (m_iHighestPing < ping)
				m_iHighestPing = ping;
		}

		// Queue
		m_wQueue = TextWidget.Cast(w.FindAnyWidget(TEXT_WIDGET_QUEUE));
		
		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		super.OnTooltipShow(tooltip);

		SCR_ScriptedWidgetTooltipContentBase content = tooltip.GetContent();
		if (!content)
			return;
		
		string message = content.GetDefaultMessage();

		switch (tooltip.GetTag())
		{
			case TOOLTIP_VERSION_MISMATCH:
				SCR_VersionMismatchTooltipComponent comp = SCR_VersionMismatchTooltipComponent.FindComponent(content.GetContentRoot());
				if (comp && m_RoomInfo)
					comp.SetWrongVersionMessage(m_RoomInfo.GameVersion());
				break;

			case TOOLTIP_JOIN:
				if (m_bIsPatchSizeLoaded)
					message = string.Format(message, GetDownloadSizeMessage());

				content.SetMessage(message);
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateModularButtons()
	{
		// Password and Play buttons
		if (m_wJoinButton)
			m_wJoinButton.SetVisible(m_bFocused && !(m_iProperties & SCR_EServerEntryProperty.PASSWORD_PROTECTED) && !(m_iProperties & SCR_EServerEntryProperty.UNJOINABLE));

		SCR_ListEntryHelper.UpdateMouseButtonColor(m_JoinButton, m_bUnavailable, m_bFocused);
		SCR_ListEntryHelper.UpdateMouseButtonColor(m_PasswordButton, m_bUnavailable, m_bFocused);
		
		super.UpdateModularButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool result = super.OnFocus(w, x, y);
		
		DisplayQueue();
		
		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool result = super.OnFocusLost(w, x, y);
		
		if (m_ModsManager)
		{
			m_ModsManager.GetOnGetAllDependencies().Remove(OnServerDetailModsLoaded);
			m_bIsPatchSizeLoaded = false;
		}
		
		DisplayQueue();
		
		return result;
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnJoinInteractionButtonClicked()
	{
		if (m_OnMouseInteractionButtonClicked)
			m_OnMouseInteractionButtonClicked.Invoke("");
	}

	//------------------------------------------------------------------------------------------------
	//! Set text in cell by it's widget name
	//! \param[in] cellName
	//! \param[in] str
	protected void SetCellText(string cellName, string str, string widgetName = "Content")
	{
		Widget wCell = m_wRoot.FindAnyWidget(cellName);
		if (!wCell)
			return;

		TextWidget wText = TextWidget.Cast(wCell.FindAnyWidget(widgetName));
		if (wText)
			wText.SetText(str);
	}

	//------------------------------------------------------------------------------------------------
	//! Display number of current ping and add icon and color
	//! \param[in] ping
	protected void DisplayPing(int ping)
	{
		//TODO: the ping threshold are manually set in the layout. This values should be unified with the server browser threshold check and filters .conf 4F6F41C387ADC14E
		
		float lastHighest = m_iHighestPing + 1;
		ServerBrowserEntryProperty displayState;

		// No ping state
		if (ping == 0)
		{
			if (m_wImgPing)
				m_wImgPing.SetVisible(false);

			return;
		}

		// Go through available icons
		foreach (ServerBrowserEntryProperty pingState : m_aPingStates)
		{
			float pingFromStr = pingState.m_sValue.ToFloat();

			// Find smallest ping
			if (ping < pingFromStr && pingFromStr < lastHighest)
			{
				displayState = pingState;
				lastHighest = pingFromStr;
			}
		}

		// Set ping text
		string strPing = Math.Floor(ping).ToString();

		// Over limit
		if (ping > m_iHighestPing || ping < 0)
			displayState = m_aPingStates[m_aPingStates.Count() - 1];

		if (ping > m_iPingLimit)
			strPing = m_iPingLimit.ToString() + "#ENF-ComboModifier";

		SetCellText("Ping", strPing);

		// Set ping icon
		if (displayState && m_wImgPing)
		{
			m_wImgPing.SetVisible(true);
			m_wImgPing.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, displayState.m_sImageName);
			m_wImgPing.SetColor(displayState.m_Color);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DisplayQueue()
	{
		if (!m_wQueue || !m_RoomInfo)
			return;

		// Text
		string queue;		
		int queueSize = m_RoomInfo.GetQueueSize();
		if (queueSize > 0)
			queue = string.Format("(%1)", UIConstants.FormatUnitShortPlus(queueSize));
		
		m_wQueue.SetText(queue);
	
		// Color
		bool maxed = m_RoomInfo.GetQueueSize() >= m_RoomInfo.GetQueueMaxSize();
		Color color;
		
		if (m_bFocused)
		{
			if (maxed)
				color = UIColors.CopyColor(UIColors.WARNING);
			else
				color = UIColors.CopyColor(UIColors.NEUTRAL_INFORMATION);
		}
		
		else if (m_bUnavailable)
		{
			if (maxed)
				color = UIColors.CopyColor(UIColors.WARNING_DISABLED);
			else
				color = UIColors.CopyColor(UIColors.IDLE_DISABLED);
		}
			
		else
		{
			if (maxed)
				color = UIColors.CopyColor(UIColors.WARNING_DISABLED);
			else
				color = UIColors.CopyColor(UIColors.NEUTRAL_ACTIVE_STANDBY);
		}
			
		m_wQueue.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check properties and assign state of room
	protected void CheckRoomProperties()
	{
		m_iProperties = 0;

		// Client versions missmatch
		bool wrongVersion = m_RoomInfo.GameVersion() != GetGame().GetBuildVersion();
		bool restrictedUGC = m_RoomInfo.IsModded() && !SCR_AddonManager.GetInstance().GetUgcPrivilege();

		if (wrongVersion || restrictedUGC)
			m_iProperties |= SCR_EServerEntryProperty.VERSION_MISMATCH;

		// Locked with password
		if (m_RoomInfo.PasswordProtected())
			m_iProperties |= SCR_EServerEntryProperty.PASSWORD_PROTECTED;

		// Crossplay
		if (m_RoomInfo.IsCrossPlatform())
			m_iProperties |= SCR_EServerEntryProperty.CROSS_PLATFORM;

		// LAN - TODO@wernerjak - add local network check
		// m_iProperties |= SCR_EServerEntryProperty.LAN;

		// MODDED
		if (m_RoomInfo.IsModded())
			m_iProperties |= SCR_EServerEntryProperty.MODDED;

		// UNJOINABLE
		if (!m_RoomInfo.Joinable())
			m_iProperties |= SCR_EServerEntryProperty.UNJOINABLE;
	}

	//------------------------------------------------------------------------------------------------
	//! Create property images based on room setup
	protected void DisplayServerProperties()
	{
		// Check states
		if (m_wImageModded)
			m_wImageModded.SetVisible(m_iProperties & SCR_EServerEntryProperty.MODDED);

		if (m_wPasswordButton)
			m_wPasswordButton.SetVisible(m_iProperties & SCR_EServerEntryProperty.PASSWORD_PROTECTED && !(m_iProperties & SCR_EServerEntryProperty.UNJOINABLE));

		// Turn favorites button into warning or unjoinable icon
		if (!m_FavoriteButton || !m_wVersionWarningIcon || !m_wUnjoinableIcon)
			return;

		bool versionMismatch = m_iProperties & SCR_EServerEntryProperty.VERSION_MISMATCH;
		bool unjoinable = m_iProperties & SCR_EServerEntryProperty.UNJOINABLE;

		m_FavoriteButton.SetEnabled(!versionMismatch && !unjoinable);

		m_wVersionWarningIcon.SetVisible(versionMismatch && !unjoinable);
		m_wUnjoinableIcon.SetVisible(unjoinable);

		m_bUnavailable = versionMismatch || unjoinable;
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDownloadSizeMessage()
	{
		if (!m_ModsManager || !(m_iProperties & SCR_EServerEntryProperty.MODDED) || m_sPatchSize.IsEmpty())
			return string.Empty;

		string icon = string.Format("<image set='%1' name='%2' scale='%3'/>", UIConstants.ICONS_IMAGE_SET, m_sTooltipDownloadIcon, m_fTooltipDownloadIconScale.ToString());
		return string.Format("  [%1%2 ]", icon, m_sPatchSize);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnServerDetailModsLoaded(Room room)
	{
		if (!m_ModsManager || !(m_iProperties & SCR_EServerEntryProperty.MODDED))
			return;
		
		array<ref SCR_WorkshopItem> toUpdateMods = m_ModsManager.GetRoomItemsToUpdate();
		if (!toUpdateMods.IsEmpty())
			m_sPatchSize = m_ModsManager.GetModListPatchSizeString(toUpdateMods);
		else
			m_sPatchSize = string.Empty;
		
		m_bIsPatchSizeLoaded = true;
		UpdateTooltipJoinDownloadSizeMessage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateTooltipJoinDownloadSizeMessage()
	{
		if (!m_CurrentTooltip || !m_CurrentTooltip.IsValid(TOOLTIP_JOIN))
			return;

		SCR_ScriptedWidgetTooltipContentBase content = m_CurrentTooltip.GetContent();
		if (!content)
			return;
		
		string message = string.Format(content.GetDefaultMessage(), GetDownloadSizeMessage());
		content.SetMessage(message);
	}

	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Set room and display room info in entry
	//! \param[in] room
	void SetRoomInfo(Room room)
	{
		m_RoomInfo = room;

		// Visualize as empty?
		if (!m_RoomInfo)
			return;

		// Setup room properties
		CheckRoomProperties();
		DisplayServerProperties();

		// Name
		SetCellText("Name", m_RoomInfo.Name());

		// Scenario
		SetCellText("Scenario", m_RoomInfo.ScenarioName());

		// Player count
		SetCellText("Players", UIConstants.FormatValueOutOf(m_RoomInfo.PlayerCount(), m_RoomInfo.PlayerLimit()));
		
		// Queue
		DisplayQueue();
		
		// Favorite
		if (m_FavoriteButton)
			SetFavorite(m_RoomInfo.IsFavorite());

		// Ping
		DisplayPing(room.GetPing());
	}

	//------------------------------------------------------------------------------------------------
	//! Set button visuals and behaviour
	//! \param[in] enable
	void EmptyVisuals(bool enable)
	{
		m_wHorizontalContent.SetVisible(!enable);
		m_wLoading.SetVisible(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] modsManager
	void SetModsManager(SCR_RoomModsManager modsManager)
	{
		m_ModsManager = modsManager;
		
		if (m_ModsManager)
		{
			m_ModsManager.GetOnGetAllDependencies().Insert(OnServerDetailModsLoaded);
			m_bIsPatchSizeLoaded = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	Room GetRoomInfo()
	{
		return m_RoomInfo;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetIsModded()
	{
		return m_iProperties & SCR_EServerEntryProperty.MODDED;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] versionMismatch
	//! \param[out] unjoinable
	//! \return
	bool GetIsEnabled(out bool versionMismatch, out bool unjoinable)
	{
		versionMismatch = m_iProperties & SCR_EServerEntryProperty.VERSION_MISMATCH;
		unjoinable = m_iProperties & SCR_EServerEntryProperty.UNJOINABLE;
		return !versionMismatch && !unjoinable;
	}
}

[BaseContainerProps()]
class ServerBrowserEntryProperty
{
	[Attribute("1", UIWidgets.ComboBox, "", category:"Selector", ParamEnumArray.FromEnum(SCR_EServerEntryProperty))]
	SCR_EServerEntryProperty m_iPropertyState;

	[Attribute("", UIWidgets.EditBox, desc: "Image name to load")]
	string m_sImageName;

	[Attribute("", UIWidgets.EditBox, desc: "String value of property")]
	string m_sValue;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_Color;
}

enum SCR_EServerEntryProperty
{
	VERSION_MISMATCH	= 1 << 0,
	PASSWORD_PROTECTED	= 1 << 1,
	CROSS_PLATFORM		= 1 << 2,
	LAN					= 1 << 3,
	MODDED				= 1 << 4,
	UNJOINABLE			= 1 << 5,
}
