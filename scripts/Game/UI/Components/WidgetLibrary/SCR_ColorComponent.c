enum UiStyleColors
{
	NORMAL,
	WARNING,
	ERROR,
};

class SCR_UIStyle
{
	static Color colorNormal = Color.Green;
	static Color colorWarning = Color.Yellow;
	static Color colorError = Color.Red;

	//------------------------------------------------------------------------------------------------
	static Color GetColorFromEnum(UiStyleColors color)
	{
		switch (color)
		{
			case UiStyleColors.NORMAL:
				return colorNormal;
			case UiStyleColors.WARNING:
				return colorWarning;
			case UiStyleColors.ERROR:
				return colorError;
			default :
				Debug.Error("Invalid color selected");
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	static string GetColorString(Color color)
	{
		return "0.5 0.5 0.5 0.5";
	}

	//------------------------------------------------------------------------------------------------
	static string GetColorString(UiStyleColors colorStyle)
	{
		return GetColorString(GetColorFromEnum(colorStyle));
	}
};

//! Minimalist progress bar
class SCR_ColorComponent : ScriptedWidgetComponent
{
	[Attribute("true")]
	protected bool m_bUseStyleValue;

	[Attribute("0", UIWidgets.ComboBox, "Color style to be applied", "", ParamEnumArray.FromEnum(UiStyleColors) )]
	protected UiStyleColors m_eColorValue;

	protected Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		if (m_bUseStyleValue)
			ApplyColorization();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyColorization()
	{
		Color c = SCR_UIStyle.GetColorFromEnum(m_eColorValue);
		m_wRoot.SetColor(c);
	}

	//------------------------------------------------------------------------------------------------
	bool IsUsingSyleValue()
	{
		return m_bUseStyleValue;
	}

	//------------------------------------------------------------------------------------------------
	void SetUsingStyleValue(bool enable)
	{
		m_bUseStyleValue = enable;
	}

	//------------------------------------------------------------------------------------------------
	bool GetUsedStyle()
	{
		return m_eColorValue;
	}

	//------------------------------------------------------------------------------------------------
	void SetUsedStyle(UiStyleColors style)
	{
		if (style == m_eColorValue)
			return;

		m_bUseStyleValue = style;
		ApplyColorization();
	}
};
