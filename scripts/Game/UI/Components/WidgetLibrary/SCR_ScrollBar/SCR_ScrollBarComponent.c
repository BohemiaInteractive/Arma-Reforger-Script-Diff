//! Scripte WLib scroll bar 
//! Can be controlled indivualy and be used for multiple purporse 

//------------------------------------------------------------------------------------------------
class SCR_ScrollBarComponent : SCR_WLibComponentBase 
{
	protected string WIDGET_BACKGROUND = "Background";
	protected string WIDGET_HANDLER = "Handler";
	protected string WIDGET_FILLER_TOP = "FillerTop";
	protected string WIDGET_FILLER_BOTTOM = "FillerBottom";
	
	protected Widget m_wBackground;
	protected Widget m_wHandler;
	protected Widget m_wFillerTop;
	protected Widget m_wFillerBottom;
	
	protected bool m_IsClicked = false;
	protected int mouseX, mouseY = 0;
	
	//protected float m_fHeight = 20;
	
	// Scroll root properties 
	protected float m_fRootPosY;
	protected float m_fRootSizeY;
	
	// Normalized size and position
	protected float m_fHandlerAmount = 1;
	protected float m_fHandlerPositon = 0;
	
	protected ref SCR_ScrollBarHandleComponent m_Handle;
	
	ref ScriptInvoker m_OnScroll = new ScriptInvoker;
	
	//-------------------------------------
	// ScriptedWidgetComponent override
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Widgets 
		m_wBackground = w.FindAnyWidget(WIDGET_BACKGROUND);
		m_wFillerTop = w.FindAnyWidget(WIDGET_FILLER_TOP);
		m_wFillerBottom = w.FindAnyWidget(WIDGET_FILLER_BOTTOM);
		
		// Get handler widget 
		m_wHandler = w.FindAnyWidget(WIDGET_HANDLER);
		if (!m_wHandler)
			return;
		
		// Access handler component
		m_Handle = SCR_ScrollBarHandleComponent.Cast(m_wHandler.FindHandler(SCR_ScrollBarHandleComponent));
		if(m_Handle)
			m_Handle.m_OnDrag.Insert(DragHandler);
	}
	
	//-------------------------------------
	// Protected
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void DragHandler()
	{			
		// Handler hight 
		float handlerW, handlerH = 0;
		m_Handle.GetRootWidget().GetScreenSize(handlerW, handlerH);
		
		// Get vertical min a max 
		float top = handlerH/2;
		float bottom = m_fRootSizeY - handlerH/2;
		
		float clampHeight = bottom - top;
		
		// mouse pos 
		int mousePosX, mousePosY;
		WidgetManager.GetMousePos(mousePosX, mousePosY);
		
		// mouse pos to root 
		mousePosY =  mousePosY - m_fRootPosY - top - m_Handle.GetMouseOffsetY();
		
		// Mouse perc 
		float scrollY = mousePosY / clampHeight;
		scrollY = Math.Clamp(scrollY, 0, 1);
		
		// Move widget position 
		SetupPosition(m_fHandlerAmount, scrollY);
		
		// Invoke scroll move 
		m_OnScroll.Invoke(m_fHandlerPositon);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup base position and size properties
	protected void SetupScrollbarProperties()
	{
		float x;
		
		m_wRoot.GetScreenPos(x, m_fRootPosY);	
		m_wRoot.GetScreenSize(x, m_fRootSizeY);
	}
	
	//-------------------------------------
	// Public
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Set amount of handler fill in percent 
	void SetupHandlerFill(float fillAmount)
	{
		// Limit 
		fillAmount = Math.Clamp(fillAmount, 0, 1.0);
		
		// Setup base properties 
		SetupScrollbarProperties();
		
		// Handler check 
		if (!m_Handle)
			return;
		
		// Filling and size setup 
		m_fHandlerAmount = fillAmount;
		
		VerticalLayoutSlot.SetFillWeight(m_Handle.GetRootWidget(), fillAmount);
		m_Handle.SetSizeY(fillAmount * m_fRootSizeY);
		
		// Setup filling of top and botton fillers 
		SetupPosition(fillAmount, m_fHandlerPositon);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup fill of top and boom fillers by handler fill amount and current scroll position
	void SetupPosition(float fillAmount, float pos)
	{
		// Filler widgets check 
		if (!m_wFillerTop || !m_wFillerBottom)
			return;
		
		// Limits 
		fillAmount = Math.Clamp(fillAmount, 0.0, 1.0);
		pos = Math.Clamp(pos, 0.0, 1.0);
		
		// Pass position value 
		m_fHandlerPositon = pos;
		
		// Setup fillers 
		float fillersAmout = 1 - fillAmount;
		
		float fillTop = fillersAmout * pos;
		float fillBottom = fillersAmout * (1 - pos);
		
		// Apply fillers amout
		VerticalLayoutSlot.SetFillWeight(m_wFillerTop, fillTop);
		VerticalLayoutSlot.SetFillWeight(m_wFillerBottom, fillBottom);
		
		// Set handler position 
		float x,y;
		m_Handle.GetRootWidget().GetScreenPos(x, y);
		m_Handle.SetPositionY(y);
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO:@wernerjak - Replace this with functions above
	/*void SetSize(float view, float count)
	{
		if (!m_wHandler)
			return;
		
		m_fHandlerAmount = view / count;
		VerticalLayoutSlot.SetFillWeight(m_wHandler, m_fHandlerAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	// TODO:@wernerjak - Replace this with functions above
	void SetPosition(float pos, float view, float count)
	{
		int iPositions = count - view;
		
		//m_fHandlerPositon = (view / count) * (iPos / (count - view - 1));
		m_fHandlerPositon = pos / iPositions;
		
		float fill = 1 - m_fHandlerAmount;
		float fillTop = fill * m_fHandlerPositon;
		float fillBottom = fill * (1 - m_fHandlerPositon);
		
		if (m_wFillerTop)
			VerticalLayoutSlot.SetFillWeight(m_wFillerTop, fillTop);
		if (m_wFillerBottom)
			VerticalLayoutSlot.SetFillWeight(m_wFillerBottom, fillBottom);
		
		/*if(m_Handle)
		{
			m_Handle.SetPosition((int)pos);
		}*/
	//}
	
	//------------------------------------------------------------------------------------------------	
	void MoveHandlerPos(float pos) { SetupPosition(m_fHandlerAmount, pos); }

	//-------------------------------------
	// Get & Set
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	float GetHandlerPosition() { return m_fHandlerPositon; }
};
