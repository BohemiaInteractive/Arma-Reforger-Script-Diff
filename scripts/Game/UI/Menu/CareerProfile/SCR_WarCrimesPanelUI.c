//------------------------------------------------------------------------------------------------
class SCR_WarCrimesPanelUI: ScriptedWidgetComponent
{

	protected Widget m_wRootWidget;
	
	protected ImageWidget m_WarCrimesHeaderImage;
	protected RichTextWidget m_WarCrimesHeaderTitle;
	protected RichTextWidget m_WarCrimesHeaderSubTitle;
	
	[Attribute(defvalue: "0.302 0.302 0.302 1", desc: "War crimes header background color")]
	protected ref Color m_iWarCrimesHeaderBackgroundColor;
	
	[Attribute("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_TextureHeaderImageNoWarCrimes;
	
	[Attribute(defvalue: "1 1 1 1", desc: "No war crimes header image color")]
	protected ref Color m_iNoWarCrimesHeaderColor;
	
	[Attribute("HeaderImageNoWarCrimes")]
	protected string m_StringHeaderImageNoWarCrimes;
	
	[Attribute(params: "No war crimes title")]
	protected string m_sNoWarCrimesTitleString;
	
	[Attribute(params: "No war crimes subtitle")]
	protected string m_sNoWarCrimesSubtitleString;
	
	[Attribute("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_TextureHeaderImageWarCrimes;
	
	[Attribute(defvalue: "1 0 0 1", desc: "War crimes header image color")]
	protected ref Color m_iWarCrimesHeaderColor;
	
	[Attribute("HeaderImageNoWarCrimes")]
	protected string m_StringHeaderImageWarCrimes;
	
	[Attribute(params: "War crimes title")]
	protected string m_sWarCrimesTitleString;
	
	[Attribute(params: "War crimes subtitle")]
	protected string m_sWarCrimesSubtitleString;
	
	[Attribute(params: "War Crimes Entry layout")]
	protected ResourceName m_WarCrimesEntryLayout;
	
	[Attribute("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_TextureEntryImageWarCrimes;
	
	[Attribute(defvalue: "1 0 0 1", desc: "War crimes entry image color")]
	protected ref Color m_iWarCrimesEntryColor;
	
	[Attribute(defvalue: "0.19608 0.06667 0.06667 1", desc: "War crimes entry image color")]
	protected ref Color m_iWarCrimesEntryBackgroundColor;
	
	[Attribute("EntryImageNoWarCrimes")]
	protected string m_StringEntryImageWarCrimes;
	
	[Attribute(params: "Harming friendlies string")]
	protected string m_sHarmingFriendliesString;
	
	[Attribute(defvalue: "0.761 0.392 0.078 1", desc: "No War Crimes subtitle color")]
	protected ref Color m_iNoWarCrimesSubtitleColor;
	
	[Attribute(defvalue: "0.502 0 0 1", desc: "War Crimes subtitle color")]
	protected ref Color m_iWarCrimesSubtitleColor;
	
	protected bool m_bActiveWarCrimes = false;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wRootWidget = w;
		m_WarCrimesHeaderImage = ImageWidget.Cast(m_wRootWidget.FindAnyWidget("WarCrimesHeaderImage"));
		m_WarCrimesHeaderTitle = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget("WarCrimesTitle"));
		m_WarCrimesHeaderSubTitle = RichTextWidget.Cast(m_wRootWidget.FindAnyWidget("WarCrimesSubtitle"));
		WarCrimesActive(false);
		
		ImageWidget backgroundHeader = ImageWidget.Cast(m_wRootWidget.FindAnyWidget("Background1"));
		if (backgroundHeader)
			backgroundHeader.SetColor(m_iWarCrimesHeaderBackgroundColor);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateWarCrimeEntry(SCR_EWarCrimes crime)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		Widget container = m_wRootWidget.FindAnyWidget("WarCrimeEntriesContainer");
		if (!container)
			return;
		
		switch (crime)
		{
			case SCR_EWarCrimes.HARMINGFRIENDLIES:
				Widget entry = workspace.CreateWidgets(m_WarCrimesEntryLayout, container);
				if (!entry)
					return;
			
				RichTextWidget entryText = RichTextWidget.Cast(entry.FindAnyWidget("WarCrimeEntryText"));
				ImageWidget entryImage = ImageWidget.Cast(entry.FindAnyWidget("WarCrimeEntryImage"));
				if (!entryText || !entryImage)
					return;
			
				entryText.SetText(m_sHarmingFriendliesString);
				SCR_WLibComponentBase.SetTexture(entryImage, m_TextureEntryImageWarCrimes, m_StringEntryImageWarCrimes);
				entryImage.SetColor(m_iWarCrimesEntryColor);
			break;
			default: return;
		}
		
		if (!m_bActiveWarCrimes)
			WarCrimesActive(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void WarCrimesActive(bool warCriminal)
	{
		if (!m_WarCrimesHeaderImage || !m_WarCrimesHeaderTitle || !m_WarCrimesHeaderSubTitle)
				return;
		
		Widget warCrimesList = m_wRootWidget.FindAnyWidget("ListOfWarCrimes");
			if (!warCrimesList)
				return;
				
		if (warCriminal)
		{
			warCrimesList.SetVisible(true);
			m_bActiveWarCrimes = true;
			SCR_WLibComponentBase.SetTexture(m_WarCrimesHeaderImage, m_TextureHeaderImageWarCrimes, m_StringHeaderImageWarCrimes);
			m_WarCrimesHeaderImage.SetColor(m_iWarCrimesHeaderColor);
			m_WarCrimesHeaderTitle.SetText(m_sWarCrimesTitleString);
			m_WarCrimesHeaderSubTitle.SetText(m_sWarCrimesSubtitleString);
			m_WarCrimesHeaderSubTitle.SetColor(m_iWarCrimesSubtitleColor);
			return;
		}
		
		warCrimesList.SetVisible(false);
		m_bActiveWarCrimes = false;
		SCR_WLibComponentBase.SetTexture(m_WarCrimesHeaderImage, m_TextureHeaderImageNoWarCrimes, m_StringHeaderImageNoWarCrimes);
		m_WarCrimesHeaderImage.SetColor(m_iNoWarCrimesHeaderColor);
		m_WarCrimesHeaderTitle.SetText(m_sNoWarCrimesTitleString);
		m_WarCrimesHeaderSubTitle.SetText(m_sNoWarCrimesSubtitleString);
		m_WarCrimesHeaderSubTitle.SetColor(m_iNoWarCrimesSubtitleColor);
	}
}

enum SCR_EWarCrimes
{
	HARMINGFRIENDLIES
}
