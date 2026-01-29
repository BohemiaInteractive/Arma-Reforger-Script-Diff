//------------------------------------------------------------------------------------------------
class SCR_SelectableTileButtonComponent : SCR_ButtonComponent 
{
	ref ScriptInvoker m_OnChanged = new ScriptInvoker();
	private bool m_bIsSelected = false;
	protected bool m_bIsTriggered = true;
	
	protected Widget m_wIcon;
	
	protected ref Color COLOR_CHECKED_TRUE = UIColors.CONTRAST_COLOR;
	protected ref Color COLOR_CHECKED_FALSE = UIColors.WHITE_DEFAULT;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wIcon = m_wRoot.FindAnyWidget("ContentImage");
		AnimateWidget.Color(m_wBackground, COLOR_BACKGROUND_DEFAULT, m_fAnimationRate);
		ColorizeCheckWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		SetSelected(!m_bIsSelected);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_bMouseOverToFocus)
			GetGame().GetWorkspace().SetFocusedWidget(w);
		
		PlaySound(m_sSoundHovered);
		
		if (GetGame().GetWorkspace().GetFocusedWidget() != w)
			AnimateWidget.Color(m_wBackground, COLOR_BACKGROUND_HOVERED, m_fAnimationRate);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		AnimateWidget.Color(m_wBackground, COLOR_BACKGROUND_DEFAULT, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		AnimateWidget.Opacity(m_wBackground, 1, m_fAnimationRate);
		PlaySound(m_sSoundHovered);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		AnimateWidget.Opacity(m_wBackground, 0, m_fAnimationRate);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuSelect()
	{
		if (!m_wRoot.IsEnabled())
			return;

		super.OnMenuSelect();
		ColorizeCheckWidget();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ColorizeWidgets(Color colorBackground, Color colorContent, float speed = -1)
	{
		if (speed < 0)
			speed = m_fAnimationRate;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set color of checked line 
	protected void ColorizeCheckWidget()
	{
		if (m_bIsSelected)
			AnimateWidget.Color(m_wIcon, COLOR_BACKGROUND_CLICKED, m_fAnimationRate);
		else
			AnimateWidget.Color(m_wIcon, COLOR_CONTENT_DEFAULT, m_fAnimationRate);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsSelected()
	{
		return m_bIsSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSelected(bool selected, bool notify = true)
	{
		if (m_bIsSelected == selected)
			return;
		
		m_bIsSelected = selected;
		
		ColorizeCheckWidget();
		
		if (notify)
			m_OnChanged.Invoke(this, m_wRoot, m_bIsSelected);
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowButton()
	{
		AnimateWidget.Opacity(m_wRoot, 1, m_fAnimationRate);
		AnimateWidget.Opacity(m_wBackground, 1, m_fAnimationRate);
	}
	
	//------------------------------------------------------------------------------------------------
	void HideButton()
	{
		AnimateWidget.Opacity(m_wBackground, 0, m_fAnimationRate);
		if (!m_bIsSelected)
			AnimateWidget.Opacity(m_wRoot, 0, m_fAnimationRate);
	}	
};