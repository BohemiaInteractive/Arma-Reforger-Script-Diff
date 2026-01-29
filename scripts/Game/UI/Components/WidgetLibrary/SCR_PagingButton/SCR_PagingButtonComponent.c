//------------------------------------------------------------------------------------------------
class SCR_PagingButtonComponent : SCR_ButtonBaseComponent
{
	[Attribute()]
	protected bool m_bIsPositive;

	[Attribute("false", UIWidgets.Auto, "Use action hint instead of an image")]
	bool m_bUseActionHint;

	[Attribute("MenuSearch")]
	string m_sActionName;

	protected string m_sBackgroundImageName = "BackgroundImage";
	protected string m_sFrontImageName = "Panel";
	protected string m_sActionTextName = "ActionText";

	protected ImageWidget m_wPanel;
	protected TextWidget m_wText;
	protected Widget m_wBackgroundImage;

	ref ScriptInvoker m_OnActivated = new ScriptInvoker(); // Returns -1 if negative, 1 if positive

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wBackgroundImage = w.FindAnyWidget(m_sBackgroundImageName);
		m_wPanel = ImageWidget.Cast(w.FindAnyWidget(m_sFrontImageName));
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sActionTextName));

		if (m_wPanel)
			m_wPanel.SetVisible(!m_bUseActionHint);

		if (m_wBackgroundImage)
		{
			m_wBackgroundImage.SetVisible(!m_bUseActionHint);
			m_wBackgroundImage.SetColor(m_BackgroundDefault);
		}
		
		if (m_wText)
		{
			m_wText.SetVisible(m_bUseActionHint);
			if (m_bUseActionHint)
			{
				m_wText.SetTextFormat("<action name='%1' scale='1.5'/>", m_sActionName);
			}
		}
		
		SetAction(m_sActionName);

		FlipImage(!m_bIsPositive);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		if (button != 0)
			return false;

		if (m_bIsPositive)
			m_OnActivated.Invoke(m_wRoot, 1);
		else
			m_OnActivated.Invoke(m_wRoot, -1);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void FlipImage(bool flip)
	{
		if (!m_wPanel || !m_wBackgroundImage)
			return;

		if (flip)
		{
			m_wPanel.SetFlags(WidgetFlags.FLIPU);
			m_wBackgroundImage.SetFlags(WidgetFlags.FLIPU);
		}
		else
		{
			m_wPanel.ClearFlags(WidgetFlags.FLIPU);
			m_wBackgroundImage.ClearFlags(WidgetFlags.FLIPU);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnMenuSelect()
	{
		MenuSelectBase();
		if (!m_wRoot.IsEnabled())
			return;

		// Bail if attached to menu but menu is not focused
		if (!IsParentMenuFocused())
			return;

		if (m_bIsPositive)
			m_OnActivated.Invoke(m_wRoot, 1);
		else
			m_OnActivated.Invoke(m_wRoot, -1);
	}

	//------------------------------------------------------------------------------------------------
	override void SetEnabled(bool enabled, bool animate = true)
	{
		super.SetEnabled(enabled, animate);
		if (m_wBackgroundImage && !m_bUseActionHint)
			m_wBackgroundImage.SetVisible(enabled);
	}

	// User API
	//------------------------------------------------------------------------------------------------
	bool IsPositive()
	{
		return m_bIsPositive;
	}

	//------------------------------------------------------------------------------------------------
	void SetPositive(bool positive)
	{
		m_bIsPositive = positive;
	}

	//------------------------------------------------------------------------------------------------
	void SetAction(string name)
	{
		if (!m_wText)
			return;

		GetGame().GetInputManager().RemoveActionListener(m_sActionName, EActionTrigger.DOWN, OnMenuSelect);
		m_sActionName = name;

		// Handle case of empty action
		name = name.Trim();
		if (name == string.Empty)
		{
			if (m_wText)
				m_wText.SetText(" ");
		}
		else
		{
			GetGame().GetInputManager().AddActionListener(m_sActionName, EActionTrigger.DOWN, OnMenuSelect);

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Hardcoded scale. Baaaaaaaaaad
	
			m_wText.SetTextFormat("<action name='%1' scale='1.5'/>", m_sActionName);
			
//---- REFACTOR NOTE END ----
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		if (!m_bUseColorization)
			return false;

		PlaySound(m_sSoundHovered);

		if (m_bUseActionHint)
			AnimateWidget.Color(m_wText, m_BackgroundHovered, m_fAnimationRate);
		else
			AnimateWidget.Color(m_wBackgroundImage, m_BackgroundHovered, m_fAnimationRate);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		if (!m_bUseColorization)
			return false;

		if (m_bUseActionHint)
			AnimateWidget.Color(m_wText, m_BackgroundDefault, m_fAnimationRate);
		else
			AnimateWidget.Color(m_wBackgroundImage, m_BackgroundDefault, m_fAnimationRate);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	string GetAction()
	{
		return m_sActionName;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_PagingButtonComponent GetPagingButtonComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		return SCR_PagingButtonComponent.Cast(SCR_PagingButtonComponent.GetComponent(SCR_ButtonBaseComponent, name, parent, searchAllChildren));
	}
};