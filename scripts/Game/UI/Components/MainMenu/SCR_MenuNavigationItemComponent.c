class SCR_MenuNavigationItemComponent: SCR_ScriptedWidgetComponent
{
	protected const string TEXT_WIDGET_NAME = "SizeLayout.Overlay.Text";
	
	[Attribute("0")]
	protected bool m_bIsSelected;
	
	[Attribute("-1", UIWidgets.SearchComboBox, enumType: ChimeraMenuPreset)]
	protected ChimeraMenuPreset m_eMenuPreset;
	
	//------------------------------------------------------------------------------------------------
	void Init(ChimeraMenuPreset menuId, string title, bool isSelected = false)
	{
		m_eMenuPreset = menuId;
		m_bIsSelected = isSelected;
		
		TextWidget text = TextWidget.Cast(m_wRoot.FindWidget(TEXT_WIDGET_NAME));
		text.SetText(title);
		
		if (m_bIsSelected)
		{
			m_wRoot.SetFlags(WidgetFlags.NOFOCUS);
			text.SetColor(UIColors.CONTRAST_CLICKED);
			
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.Cast(m_wRoot.FindHandler(SCR_ModularButtonComponent));
			comp.SetAllEffectsEnabled(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ChimeraMenuPreset GetMenuPreset()
	{
		return m_eMenuPreset;
	}
}