enum EDirection
{
	LEFT,
	UP,
	RIGHT,
	DOWN
};

class SCR_CollapseWidgetComponent : ScriptedWidgetComponent
{
	[Attribute("false")]
	protected bool m_bCollapsed;

	[Attribute("0", UIWidgets.ComboBox, "Direction of the collapse", "", ParamEnumArray.FromEnum(EDirection))]
	protected EDirection m_eCollapseDirection;

	[Attribute("0.5")]
	protected float m_fCollapseTime;

	[Attribute(desc: "Is widget slot a frame, or if it's inside a layout slot. FRAME SLOT NOT IMPLEMENTED!")]
	protected bool m_bIsFrameSlot;

	protected float m_fDefaultValue = float.MAX;
	protected Widget m_wRoot;
	protected float m_fTimeout = 50;

	//ref ScriptInvoker<bool> m_OnToggled = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		SetCollapseTime(m_fCollapseTime);

		if (!m_bCollapsed)
			return;
		
		// Hack - delayed start (if there should be animation)
		if (m_fCollapseTime != float.MAX)
		{
			float opacity = w.GetOpacity();
			w.SetOpacity(0);
			GetGame().GetCallqueue().CallLater(ShowWidgetDelayed, m_fTimeout, false, opacity);
		}
		else
		{
			SetCollapsed(m_bCollapsed, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowWidgetDelayed(float opacity)
	{
		AnimateWidget.Opacity(m_wRoot, opacity, UIConstants.FADE_RATE_FAST);
		SetCollapsed(true, false);
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget() 
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	void SetCollapsed(bool collapsed, bool animate = true)
	{
		if (m_bCollapsed == collapsed && animate)
			return;
		
		m_bCollapsed = collapsed;

		if (m_bIsFrameSlot)
			Print("CollapseWidgetComponent frameSlot not implemented", LogLevel.WARNING);
		else
			SetupLayoutAnimation(m_fCollapseTime, animate);
	}

	//------------------------------------------------------------------------------------------------
	bool IsCollapsed()
	{
		return m_bCollapsed;
	}

	//------------------------------------------------------------------------------------------------
	void SetCollapseTime(float time)
	{
		// Convert to collapse speed
		if (time <= 0)
			m_fCollapseTime = float.MAX;
		else
			m_fCollapseTime = 1 / time;
	}

	//------------------------------------------------------------------------------------------------
	float GetCollapseTime()
	{
		// Collapse speed can never be 0
		return 1 / m_fCollapseTime;
	}

	//------------------------------------------------------------------------------------------------
	void SetCollpseDirection(EDirection direction)
	{
		m_eCollapseDirection = direction;
	}

	//------------------------------------------------------------------------------------------------
	EDirection GetCollapseDirection()
	{
		return m_eCollapseDirection;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupLayoutAnimation(float speed, bool animate)
	{
		if (!m_wRoot)
			return;

		// Do not try to animate the collapse if animation is too fast
		if (m_fCollapseTime == float.MAX)
			animate = false;
		
		float left, top, right, bottom, w, h;
		AlignableSlot.GetPadding(m_wRoot, left, top, right, bottom);
		m_wRoot.GetScreenSize(w, h);
		if (w <= 0 || h <= 0)
		{
			//m_wRoot.Update();
			m_wRoot.GetScreenSize(w, h);
		}

		w = GetGame().GetWorkspace().DPIUnscale(w);
		h = GetGame().GetWorkspace().DPIUnscale(h);
		
		float paddings[4];

		switch (m_eCollapseDirection)
		{
			case EDirection.LEFT:
				if (m_bCollapsed)
				{
					if (m_fDefaultValue == float.MAX)
						m_fDefaultValue = left;

					paddings = {-w - right, top, right, bottom};
				}
				else
				{
					paddings = {m_fDefaultValue, top, right, bottom};
				}
			break;
			case EDirection.UP:
				if (m_bCollapsed)
				{
					if (m_fDefaultValue == float.MAX)
						m_fDefaultValue = top;

					paddings = {left, -h - bottom, right, bottom};
				}
				else
				{
					paddings = {left, m_fDefaultValue, right, bottom};
				}
			break;
			case EDirection.RIGHT:
				if (m_bCollapsed)
				{
					if (m_fDefaultValue == float.MAX)
						m_fDefaultValue = right;
				
					paddings = {left, top, -w - left, bottom};
				}
				else
				{
					paddings = {left, top, m_fDefaultValue, bottom};
				}
			break;
			case EDirection.DOWN:
				if (m_bCollapsed)
				{
					if (m_fDefaultValue == float.MAX)
						m_fDefaultValue = bottom;
				
					paddings = {left, top, right, -h - top};
				}
				else
				{
					paddings = {left, top, right, m_fDefaultValue};
				}
			break;
		}
		
		if (animate)
			AnimateWidget.Padding(m_wRoot, paddings, speed);
		else
			m_wRoot.SetVisible(!m_bCollapsed); // Do not attempt to set any padding, just hide the widget
			//AlignableSlot.SetPadding(m_wRoot, paddings[0], paddings[1], paddings[2], paddings[3]);
	}
};