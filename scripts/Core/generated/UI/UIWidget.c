/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class UIWidget: Widget
{
	proto external void SetStyle(string styleName);
	proto external void SetDefaultStyle();
	/*
	float GetTextSize() const
		SCR_METHOD("float GetTextSize()")
	{
		return m_FontPreset.m_Properties.m_fHeight;
	}

	void SetTextSize(float fontHeight)
		SCR_METHOD("void SetTextSize(float fontHeight)")
	{
		m_FontPreset.m_Properties.m_fHeight = fontHeight;
	}
	*/
	proto external void SetTextColor(int color);
	proto external int GetTextColor();
	/*!
	Sets text sharpness multiplier. Useful for sharper/smoother edges.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextSharpness(float sharpness);
	/*!
	Gets current text sharpness
	*/
	proto external float GetTextSharpness();
	/*!
	Sets outline style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextOutline(int outlineSize, int argb = 0xFF000000);
	proto external int GetTextOutlineSize();
	proto external int GetTextOutlineColor();
	/*!
	Sets shadow style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.

	\note Shadow opacity is redundant, it will just multiply the shadowARGB alpha
	*/
	proto external void SetTextShadow(int shadowSize, int shadowARGB = 0xFF000000, float shadowOpacity = 1.0, float shadowOffsetX = 0.0, float shadowOffsetY = 0.0);
	proto external int GetTextShadowSize();
	/*!
	Returns shadow color encoded as ARGB
	*/
	proto external int GetTextShadowColor();
	/*!
	\deprecated Shadow opacity is redundant
	*/
	[Obsolete("Shadow opacity is redundant")]
	proto external float GetTextShadowOpacity();
	proto external float GetTextShadowOffsetX();
	proto external float GetTextShadowOffsetY();
	/*!
	Sets italic style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextItalic(bool italic);
	/*!
	Gets current italic style.
	*/
	proto external bool GetTextItalic();
	/*!
	Sets bold style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetTextBold(bool bold);
	/*!
	Gets current bold style.
	*/
	proto external bool GetTextBold();
}

/*!
\}
*/
