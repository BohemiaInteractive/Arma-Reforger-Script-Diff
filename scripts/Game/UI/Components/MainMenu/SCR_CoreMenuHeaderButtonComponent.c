//! Helper component for core menu header button prefabs
class SCR_CoreMenuHeaderButtonComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	string m_sIconName;

	[Attribute("1")]
	bool m_bShowCount;

	[Attribute("Label")]
	string m_sLabel;

	protected Widget m_wButtonWidget;
	protected SCR_ModularButtonComponent m_ButtonComponent;

	protected ImageWidget m_wIcon;

	protected Widget m_wCountIconFrame;
	protected TextWidget m_wCountText;

	protected TextWidget m_wLabel;

	protected const string BUTTON = "CoreMenuHeaderButton";
	protected const string COUNT_ICON_FRAME = "CountIconFrame";
	protected const string COUNT_TEXT = "CountText";
	protected const string ICON = "Icon";
	protected const string LABEL = "ButtonLabel";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wButtonWidget = w.FindAnyWidget(BUTTON);
		if (m_wButtonWidget)
			m_ButtonComponent = SCR_ModularButtonComponent.FindComponent(m_wButtonWidget);

		m_wCountIconFrame = w.FindAnyWidget(COUNT_ICON_FRAME);
		m_wCountText = TextWidget.Cast(w.FindAnyWidget(COUNT_TEXT));
		SetCountVisible(m_bShowCount);

		m_wLabel = TextWidget.Cast(w.FindAnyWidget(LABEL));
		SetLabelText(m_sLabel);

		m_wIcon = ImageWidget.Cast(w.FindAnyWidget(ICON));
		SetIcon(m_sIconName);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name
	void SetIcon(string name)
	{
		if (!m_wIcon)
			return;

		bool show = name != string.Empty;
		m_wIcon.SetVisible(show);

		if (!show)
			return;

		m_wIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, name);

		// Resize
		int x, y;
		m_wIcon.GetImageSize(0, x, y);
		m_wIcon.SetSize(x, y);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetCountText(string text)
	{
		if (!m_wCountText)
			return;

		m_wCountText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] visible
	void SetCountVisible(bool visible)
	{
		if (!m_wCountIconFrame)
			return;

		m_wCountIconFrame.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	void SetLabelText(string text)
	{
		if (!m_wLabel)
			return;

		m_wLabel.SetVisible(!text.IsEmpty());
		
		if (m_wLabel.IsVisible())
			m_wLabel.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetLabelText()
	{
		if (!m_wLabel)
			return string.Empty;

		return m_wLabel.GetText();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] visible
	//! \param[in] animate
	//! \param[in] rate
	void SetVisible(bool visible, bool animate = false, float rate = UIConstants.FADE_RATE_FAST)
	{
		if (!m_wRoot)
			return;
		
		if (animate)
			AnimateWidget.Opacity(m_wRoot, visible, rate, true);
		else
			m_wRoot.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ModularButtonComponent GetButton()
	{
		return m_ButtonComponent;
	}
}
