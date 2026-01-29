class SCR_StatsPanel_ServerName : SCR_StatsPanelBase
{
	protected string m_sServerName;
	protected const float OPACITY = 0.5;
	protected ServerInfo m_ServerInfo = GetGame().GetServerInfo();
	
	//------------------------------------------------------------------------------------------------
	override bool DisplayStartDrawInit(IEntity owner)
	{
		super.DisplayStartDrawInit(owner);
		
		if (m_ServerInfo != null)
			m_sServerName = m_ServerInfo.GetName();
		
		return !m_sServerName.IsEmpty();
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_Widgets = new SCR_StatsPanelWidgets();
		m_Widgets.Init(m_wRoot);
		
		Update(0);
		
		m_Widgets.m_wLabel.SetText(m_sServerName);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		//~ No need to update as server name is only set once
	}	
	
	//------------------------------------------------------------------------------------------------
	override protected void Update(float value)
	{		
		// Get visialization attributes based on the new state
		Color color = GUIColors.DEFAULT;
		Color colorGlow = GUIColors.DEFAULT_GLOW;
		string icon = m_sIconOK;
		
		// Set global color & opacity
		m_Widgets.m_wColorOpacity.SetOpacity(OPACITY);
		m_Widgets.m_wColorOpacity.SetColor(color);

		// Set icon & glow		
		m_Widgets.m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		m_Widgets.m_wIconGlow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, icon);
		m_Widgets.m_wIconGlow.SetColor(colorGlow);
		
		OverlayWidget textWidget = OverlayWidget.Cast(m_wRoot.FindAnyWidget("Text"));		
		textWidget.SetVisible(false);
	}	
}