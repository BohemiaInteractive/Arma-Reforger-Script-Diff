/*!
Component for addons export submenu behavior.
Player can generate JSON and CLI text for current enabled addons and copy this text to clipboard.
*/

enum EAddonExportFormat
{
	JSON,
	CLI
}

class SCR_AddonsExportSubMenuComponent : SCR_SubMenuBase
{
	[Attribute("0", UIWidgets.ComboBox, "Used format for submenu", "", ParamEnumArray.FromEnum(EAddonExportFormat))]
	protected EAddonExportFormat m_iUsedFormat;
	
	protected ref SCR_AddonsExportSubMenuWidgets m_Widgets = new SCR_AddonsExportSubMenuWidgets();
	
	protected string m_sGeneratedText;
	
	protected SCR_InputButtonComponent m_NavCopyComponent;
	
	//---------------------------------------------------------------------------------------------------
	// Override 
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);
	}
	
	//---------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		if (!m_NavCopyComponent)
		{
			m_NavCopyComponent = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT, "#AR-Workshop_AddonsExportCopyBtn", true);
			m_NavCopyComponent.m_OnActivated.Insert(CopyToClipboard);
		}
		
		switch (m_iUsedFormat)
		{
			case EAddonExportFormat.JSON: GenerateJSONFormat(); break;
			case EAddonExportFormat.CLI: GenerateCLIFormat(); break;
		}
	}
	
	//---------------------------------------------------------------------------------------------------
	// Protected 
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	//! Display text in widgets 
	protected void DisplayText(string text)
	{
		m_Widgets.m_GenContent.SetText(text);
		m_Widgets.m_ScrollLayout.SetSliderPos(0,0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Generate text of addon in json config format 
	protected string GenerateAddonJson(SCR_WorkshopItem item)
	{
		return string.Format(
			"\t{\n\t\t\%1: %2,\n\t\t%3: %4,\n\t\t%5: %6\n\t}",
			qstr("modId"), qstr(item.GetId()),
			qstr("name"), qstr(item.GetName()),
			qstr("version"), qstr(item.GetCurrentLocalRevision().GetVersion()) );
	}
	
	// Puts quotes around a string
	static string qstr(string s)
	{ 
		return string.Format("\"%1\"", s);
	}
	
	//---------------------------------------------------------------------------------------------------
	// Actions 
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	//! Generate text for enbled addons in json format for server config
	void GenerateJSONFormat()
	{
		auto offlineAddons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		array<ref SCR_WorkshopItem> enabledAddons = SCR_AddonManager.SelectItemsBasic(offlineAddons, EWorkshopItemQuery.ENABLED);
		
		string s;
		
		if (!enabledAddons.IsEmpty())
		{
			s = s + GenerateAddonJson(enabledAddons[0]);	
			
			for (int i = 1; i < enabledAddons.Count(); i++)
				s = s + ",\n" + GenerateAddonJson(enabledAddons[i]);
		}
		
		m_sGeneratedText = s;
		DisplayText(m_sGeneratedText);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Generate text with list of enabled addons ids
	void GenerateCLIFormat()
	{
		string cli = "-addons ";
		
		auto offlineAddons = SCR_AddonManager.GetInstance().GetOfflineAddons();
		array<ref SCR_WorkshopItem> enabledAddons = SCR_AddonManager.SelectItemsBasic(offlineAddons, EWorkshopItemQuery.ENABLED);
		
		if (!enabledAddons.IsEmpty())
		{
			cli = cli + enabledAddons[0].GetId();
			for (int i = 1; i < enabledAddons.Count(); i++)
				cli = cli + ", " + enabledAddons[i].GetId();
		}
		
		m_sGeneratedText = cli;
		DisplayText(m_sGeneratedText);
	}
	
	//---------------------------------------------------------------------------------------------------
	//! Copy currently generated text to clipboard
	void CopyToClipboard()
	{
		System.ExportToClipboard(m_sGeneratedText);
	}
	
	//---------------------------------------------------------------------------------------------------
	// API 
	//---------------------------------------------------------------------------------------------------
	//---------------------------------------------------------------------------------------------------
	string GetGeneratedText()
	{
		return m_sGeneratedText;
	}
}