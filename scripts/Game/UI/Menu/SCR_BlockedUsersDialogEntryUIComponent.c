class SCR_BlockedUsersDialogEntryUIComponent : SCR_ScriptedWidgetComponent
{	
	[Attribute("m_PlayerName")]
	protected string m_sPlayerNameWidgetName;
	
	[Attribute("m_PlatformIcon")]
	protected string m_sPlatformIconWidgetName;
	
	[Attribute("ProfileButton")]
	protected string m_sProfileButtonName;
	
	[Attribute("UnblockButton")]
	protected string m_sUnblockButtonName;
	
	protected RichTextWidget m_wPlayerName;
	protected ImageWidget m_wPlatfromIcon;
	protected ImageWidget m_wUnblockButtonIcon;
	
	protected string m_sPlayerName;
	protected PlatformKind m_ePlayerPlatform;
	
	protected SCR_ModularButtonComponent m_Button;
	
	protected SCR_ModularButtonComponent m_ProfileButton;
	protected SCR_ModularButtonComponent m_UnblockButton;
	
	//Debug variable
	bool m_bDebugIsPlaystation;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wPlayerName = RichTextWidget.Cast(m_wRoot.FindAnyWidget(m_sPlayerNameWidgetName));
		m_wPlatfromIcon = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sPlatformIconWidgetName));			
		
		m_ProfileButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget(m_sProfileButtonName).FindHandler(SCR_ModularButtonComponent));
		m_UnblockButton = SCR_ModularButtonComponent.Cast(m_wRoot.FindAnyWidget(m_sUnblockButtonName).FindHandler(SCR_ModularButtonComponent));
		
		m_wUnblockButtonIcon = ImageWidget.Cast(m_ProfileButton.GetRootWidget().FindAnyWidget("Icon"));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show the name of the player
	void SetPlayerName(string name)
	{
		if (!m_wPlayerName)
			return;
		
		m_sPlayerName = name;
		m_wPlayerName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetPlayerName()
	{
		return m_sPlayerName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Show the players platform
	void SetPlatfrom(PlatformKind platform)
	{
		if (!m_wPlatfromIcon)
			return;
		
		if (platform == PlatformKind.PSN)
		{
			m_wPlatfromIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME);
			m_wUnblockButtonIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME);
		}
		else
		{
			m_wPlatfromIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME);
			m_wUnblockButtonIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME);
		}
		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetButtonsVisibility(bool newVis)
	{
		//m_ProfileButton.SetVisible(newVis);
		m_UnblockButton.SetVisible(newVis);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ModularButtonComponent GetProfileButton()
	{
		return m_ProfileButton;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ModularButtonComponent GetUnblockButton()
	{
		return m_UnblockButton;
	}
	
	
	
	
}