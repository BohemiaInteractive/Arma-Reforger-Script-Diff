
//------------------------------------------------------------------------------------------------
class SCR_ScrollBarHandleComponent : SCR_WLibComponentBase 
{
	protected bool m_bIsDragged = false;
	protected bool m_bIsHovered = false;
	
	// Stable 
	protected float m_fSizeY;
	
	protected float m_fPositionY;
	protected float m_fMouseOffsetY;
	
	protected ref Color COLOR_HANDLER_DEFAULT = UIColors.CONTRAST_COLOR;
	protected ref Color COLOR_HANDLER_HOVERED = Color.White;
	
	ref ScriptInvoker m_OnDrag = new ScriptInvoker;
	
	//-------------------------------------
	// Override 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Handler root setup
		m_wRoot = w;
		m_wRoot.SetColor(COLOR_HANDLER_DEFAULT);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		m_bIsHovered = true;
		m_wRoot.SetColor(COLOR_HANDLER_HOVERED);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{		
		super.OnMouseLeave(w, enterW, x, y);
		
		m_bIsHovered = true;
		
		if (m_bIsDragged)
			return false;
		
		m_wRoot.SetColor(COLOR_HANDLER_DEFAULT);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		// Accept only LMB as valid click
		if (button != 0)
			return false;
		
		m_bIsDragged = true;
		
		// Mouse offset 
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		m_fMouseOffsetY = mouseY - m_fPositionY - m_fSizeY/2;
		
		DragHandler();
		
		PlaySound(m_sSoundClicked);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		// Accept only LMB as valid click
		if (button != 0)
			return false;
		
		m_bIsDragged = false;
		
		if (!m_bIsHovered)
			m_wRoot.SetColor(COLOR_HANDLER_DEFAULT);
		
		PlaySound(m_sSoundClicked);
		return false;
	}
	
	//-------------------------------------
	// Protected
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Move with handler if handler is clicked 
	protected void DragHandler()
	{
		if(!m_bIsDragged)
			return;
		
		// Invoke move 
		m_OnDrag.Invoke();
		
		// Call drag 
		GetGame().GetCallqueue().CallLater(DragHandler, 0);	
	}
	
	//-------------------------------------
	// Get & Set
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetSizeY(float size) { m_fSizeY = size; }
	
	//------------------------------------------------------------------------------------------------
	float GetSizeY() { return m_fSizeY; }
	
	//------------------------------------------------------------------------------------------------
	void SetPositionY(float pos) { m_fPositionY = pos; }
	
	//------------------------------------------------------------------------------------------------
	float GetPositionY() { return m_fPositionY; }
	
	//------------------------------------------------------------------------------------------------
	float GetMouseOffsetY() {return m_fMouseOffsetY; }
};