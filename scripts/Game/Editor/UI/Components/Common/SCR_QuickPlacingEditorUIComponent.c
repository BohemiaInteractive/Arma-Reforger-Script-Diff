//! @ingroup Editor_UI Editor_UI_Components

class SCR_QuickPlacingEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected static const float ANIMATION_THRESHOLD = 0.01;
	
	[Attribute()]
	private string m_sActiveWidgetName;
	
	[Attribute()]
	private string m_sExpandWidgetName;
	
	[Attribute(defvalue: "0.2", uiwidget: UIWidgets.Slider, params: "0 1 0.01")]
	private float m_fHideCoef;
	
	[Attribute(defvalue: "0.1")]
	private float m_fAnimationStrength;
	
	private Widget m_wExpandWidget;
	private float m_fDefaultOffset;
	private float m_fCurrentCoef;
	private bool m_bShown;
	
	//------------------------------------------------------------------------------------------------
	protected void Show(bool show)
	{
		if (show == m_bShown)
			return;
		
		m_bShown = show;
		if (show)
			SetCoef(1);
		else
			SetCoef(m_fHideCoef);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCoef(float coef, bool instant = false)
	{
		m_fCurrentCoef = coef;
		if (instant)
		{
			SetOffsets(1);
		}
		else
		{
			MenuRootBase menu = GetMenu();
			if (menu)
				menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetOffsets(float progress)
	{
		float offsetLeft, offsetTop, offsetRight, offsetBottom;
		FrameSlot.GetOffsets(m_wExpandWidget, offsetLeft, offsetTop, offsetRight, offsetBottom);
		
		offsetTop = Math.Lerp(offsetTop, m_fDefaultOffset * m_fCurrentCoef, progress);
		offsetBottom = Math.Lerp(offsetBottom, m_fDefaultOffset * (1 - m_fCurrentCoef), progress);
		
		FrameSlot.SetOffsets(m_wExpandWidget, offsetLeft, offsetTop, offsetRight, offsetBottom);
		
		return Math.AbsFloat(offsetTop - m_fDefaultOffset * m_fCurrentCoef) < ANIMATION_THRESHOLD;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float tDelta)
	{
		if (SetOffsets(tDelta / m_fAnimationStrength))
		{
			MenuRootBase menu = GetMenu();
			if (menu)
				menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnShortcut(Widget w)
	{
		Show(!m_bShown);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (w.GetName() == m_sActiveWidgetName && m_wExpandWidget)
			Show(true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (w.GetName() == m_sActiveWidgetName && m_wExpandWidget)
			Show(false);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_wExpandWidget = w.FindAnyWidget(m_sExpandWidgetName);
		if (!m_wExpandWidget)
			return;
		
		float offsetLeft, /* offsetTop, */ offsetRight, offsetBottom;
		FrameSlot.GetOffsets(m_wExpandWidget, offsetLeft, m_fDefaultOffset, offsetRight, offsetBottom);
		
		SetCoef(m_fHideCoef, true);

		ScriptInvoker onToggle = ButtonActionComponent.GetOnAction(w);
		if (onToggle)
			onToggle.Insert(OnShortcut);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		// do nothing
	}
}
