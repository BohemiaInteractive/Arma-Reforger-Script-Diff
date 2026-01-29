class SCR_SnackBarController : SCR_ScriptedWidgetComponent
{
	protected ref SCR_ActionMenuSnackBarWidgets m_Widgets = new SCR_ActionMenuSnackBarWidgets();

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_IconColor;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_ACTIVE_STANDBY), UIWidgets.ColorPicker)]
	protected ref Color m_TextColor;

	[Attribute(UIColors.GetColorAttribute(UIColors.DARK_GREY), UIWidgets.ColorPicker)]
	protected ref Color m_BackgroundColor;

	[Attribute("0.2", UIWidgets.Slider, params: "0 10 0.1")]
	protected float m_fSnackBarColorFadeInSpeed;

	[Attribute("0.6", UIWidgets.Slider, params: "0 10 0.1")]
	protected float m_fSnackBarFadeInSpeed;

	[Attribute("0.3", UIWidgets.Slider, params: "0 10 0.1")]
	protected float m_fSnackBarFadeOutSpeed;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);

		// Defaults
		SetColors(m_IconColor, m_TextColor, m_BackgroundColor);

		m_wRoot.SetVisible(false);
		m_wRoot.SetOpacity(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetFadeInSpeed()
	{
		return 1 / m_fSnackBarFadeInSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetColorFadeInSpeed()
	{
		return 1 / m_fSnackBarFadeInSpeed;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetFadeOutSpeed()
	{
		return 1 / m_fSnackBarFadeOutSpeed;
	}

	//------------------------------------------------------------------------------------------------
	void SetVisibility(bool show)
	{
		AnimateWidget.StopAllAnimations(m_wRoot);
		m_wRoot.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void AnimateShow()
	{
		AnimateWidget.StopAllAnimations(m_wRoot);
		AnimateWidget.Opacity(m_wRoot, 1, GetFadeInSpeed(), true);
		
		m_Widgets.m_wSnackBarBackgroundImgLeft.SetColor(UIColors.DARK_SAGE);
		m_Widgets.m_wSnackBarBackgroundImgMid.SetColor(UIColors.DARK_SAGE);
		m_Widgets.m_wSnackBarBackgroundImgRight.SetColor(UIColors.DARK_SAGE);
		
		AnimateWidget.Color(m_Widgets.m_wSnackBarBackgroundImgLeft, m_BackgroundColor, GetColorFadeInSpeed());
		AnimateWidget.Color(m_Widgets.m_wSnackBarBackgroundImgMid, m_BackgroundColor, GetColorFadeInSpeed());
		AnimateWidget.Color(m_Widgets.m_wSnackBarBackgroundImgRight, m_BackgroundColor, GetColorFadeInSpeed());
	}

	//------------------------------------------------------------------------------------------------
	void AnimateHide()
	{
		AnimateWidget.StopAllAnimations(m_wRoot);
		AnimateWidget.Opacity(m_wRoot, 0, GetFadeOutSpeed(), true);
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_Widgets.m_wSnackBarText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void SetIcon(string imageSet, string icon)
	{
		m_Widgets.m_wSnackBarIcon.LoadImageFromSet(0, imageSet, icon);
	}

	//------------------------------------------------------------------------------------------------
	void SetColors(notnull Color IconColor, notnull Color TextColor, Color BackgrounsColor)
	{
		m_IconColor = IconColor;
		m_TextColor = TextColor;
		m_BackgroundColor = BackgrounsColor;
		
		m_Widgets.m_wSnackBarIcon.SetColor(IconColor);
		m_Widgets.m_wSnackBarText.SetColor(TextColor);

		m_Widgets.m_wSnackBarBackgroundImgLeft.SetColor(BackgrounsColor);
		m_Widgets.m_wSnackBarBackgroundImgMid.SetColor(BackgrounsColor);
		m_Widgets.m_wSnackBarBackgroundImgRight.SetColor(BackgrounsColor);
	}
}
