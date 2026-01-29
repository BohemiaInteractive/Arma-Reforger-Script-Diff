//! Deprecated button component. Still used in many prefabs, so it works, it's just stripped of most of the functionality

//------------------------------------------------------------------------------------------------
class SCR_ButtonComponent : SCR_ButtonBaseComponent
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sContent;

	protected Widget m_wOverlay;
	protected Widget m_wContent;

	protected ref Color COLOR_BACKGROUND_DEFAULT = UIColors.BACKGROUND_DEFAULT;
	protected ref Color COLOR_BACKGROUND_HOVERED = UIColors.BACKGROUND_HOVERED;
	protected ref Color COLOR_BACKGROUND_FOCUSED = UIColors.BACKGROUND_HOVERED;
	protected ref Color COLOR_BACKGROUND_CLICKED = UIColors.BACKGROUND_HOVERED;

	protected ref Color COLOR_CONTENT_DEFAULT = Color.White;
	protected ref Color COLOR_CONTENT_HOVERED = Color.White;
	protected ref Color COLOR_CONTENT_FOCUSED = Color.White;
	protected ref Color COLOR_CONTENT_CLICKED = Color.White;

	//ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	ref ScriptInvoker m_OnHover = new ScriptInvoker();
	ref ScriptInvoker<Widget> m_OnHoverLeave = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wOverlay = w.FindAnyWidget("Overlay");
		m_wBackground = w.FindAnyWidget("Background");

		m_wContent = w.FindAnyWidget("Content");
		if (m_wContent && m_sContent != string.Empty)
			SetContent(m_sContent);

		if (m_wBackground)
			m_wBackground.SetColor(COLOR_BACKGROUND_DEFAULT);

		if (m_wOverlay)
			m_wOverlay.SetColor(COLOR_CONTENT_DEFAULT);
	}

	/*
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;

		m_OnClicked.Invoke(w);
		return false;
	}
	*/

	/*
	//------------------------------------------------------------------------------------------------
	override protected void OnMenuSelect()
	{
		super.OnMenuSelect();
		if (!m_wRoot.IsEnabled())
			return;

		//ColorizeWidgets(COLOR_BACKGROUND_CLICKED, COLOR_CONTENT_CLICKED, m_fAnimationRate * 2);
		//GetGame().GetCallqueue().CallLater(ResetPadding, m_fAnimationTime * 500 + 1, false);
	}
	*/

	/*
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() == w)
			ColorizeWidgets(COLOR_BACKGROUND_CLICKED, COLOR_CONTENT_CLICKED);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() == w)
			ColorizeWidgets(COLOR_BACKGROUND_FOCUSED, COLOR_CONTENT_FOCUSED);
		else
			ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);

		return false;
	}
	*/

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		m_OnHover.Invoke();
		//if (GetGame().GetWorkspace().GetFocusedWidget() != w)
			//ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		m_OnHoverLeave.Invoke(w);
		//if (GetGame().GetWorkspace().GetFocusedWidget() != w)
			//ColorizeWidgets(COLOR_BACKGROUND_DEFAULT, COLOR_CONTENT_DEFAULT);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		m_OnFocus.Invoke(w);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ColorizeWidgets(Color colorBackground, Color colorContent, float speed = -1)
	{
		if (speed < 0)
			speed = m_fAnimationRate;

		AnimateWidget.Color(m_wBackground, colorBackground, speed);
		AnimateWidget.Color(m_wOverlay, colorContent, speed);
	}

	//------------------------------------------------------------------------------------------------
	void SetHorizontalAlign(LayoutHorizontalAlign horizontalAlign)
	{
		if (!m_wContent)
			return;

		TextWidget content = TextWidget.Cast(m_wContent);
		if (content)
			AlignableSlot.SetHorizontalAlign(content, horizontalAlign);
	}

	//------------------------------------------------------------------------------------------------
	void SetContent(string text)
	{
		if (!m_wContent)
			return;

		TextWidget content = TextWidget.Cast(m_wContent);
		if (content)
		{
			content.SetText(text);
			return;
		}

		ImageWidget img = ImageWidget.Cast(m_wContent);
		if (img)
		{
			img.LoadImageTexture(0, text);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetContentFromUIInfo(SCR_UIInfo info)
	{
		if (!m_wContent)
			return;

		if (!info.SetNameTo(TextWidget.Cast(m_wContent)))
			info.SetIconTo(ImageWidget.Cast(m_wContent));
	}

	//------------------------------------------------------------------------------------------------
	string GetContent()
	{
		return m_sContent;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_ButtonComponent GetButtonComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ButtonComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};