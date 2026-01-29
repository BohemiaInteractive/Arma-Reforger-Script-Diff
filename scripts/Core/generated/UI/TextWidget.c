/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

/*!
Widget representing text. Can be both single and multiline. Automatic wrapping is turned on by the WRAP_TEXT flag.
*/
sealed class TextWidget: Widget
{
	/*!
	Sets rotation of widget around defined center
	\param angle Angle in degrees
	*/
	proto external void SetRotation(float angle);
	//! Returns rotation of widget in degrees
	proto external float GetRotation();
	//! Sets pivot around which the widget will rotate
	proto external void SetPivot(float x, float y);
	/*!
	Sets top-left corner where text begins in widget. For RALIGN the horizontal offset is from the right.
	\param xoff Offset from left
	\param yoff Offset from top
	*/
	proto external void SetTextOffset(float xoff, float yoff);
	//! Sets given spacing between lines
	proto external void SetLineSpacing(float spacing);
	//! Sets desired (maximal) font size.
	proto external void SetDesiredFontSize(int size);
	//! Sets minimal font size.
	proto external void SetMinFontSize(int size);
	//! Sets both min font size and desired font size to the same value `size`.
	proto external void SetExactFontSize(int size);
	//! Returns whether the text wraps when it runs out of horizontal space
	proto external bool GetTextWrapping();
	//! Sets whether should the text wrap when it runs out of horizontal space
	proto external void SetTextWrapping(bool isWrapping);
	/*!
	Sets text sharpness multiplier. Useful for sharper/smoother edges.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetSharpness(float sharpness);
	/*!
	Gets current text sharpness
	*/
	proto external float GetSharpness();
	/*!
	Sets outline style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetOutline(int outlineSize, int argb = 0xFF000000);
	proto external int GetOutlineSize();
	proto external int GetOutlineColor();
	/*!
	Sets shadow style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.

	\note Shadow opacity is redundant, it will just multiply the shadowARGB alpha
	*/
	proto external void SetShadow(int shadowSize, int shadowARGB = 0xFF000000, float shadowOpacity = 1, float shadowOffsetX = 0, float shadowOffsetY = 0);
	proto external int GetShadowSize();
	/*!
	Returns shadow color encoded as ARGB
	*/
	proto external int GetShadowColor();
	/*!
	\deprecated Shadow opacity is redundant, it is just the ShadowColor alpha
	*/
	[Obsolete("Shadow opacity is redundant")]
	proto external float GetShadowOpacity();
	/*!
	Sets italic style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetItalic(bool italic);
	/*!
	Gets current italic style.
	*/
	proto external bool GetItalic();
	/*!
	Sets bold style.
	The underlying font implementation may not support it, if this is the case, it will do nothing.
	*/
	proto external void SetBold(bool bold);
	/*!
	Gets current bold style.
	*/
	proto external bool GetBold();
	/*!
	Sets force font. Disable font override by language settings.
	*/
	proto external void SetForceFont(bool force);
	/*!
	Gets current force font.
	*/
	proto external bool GetForceFont();
	/*!
	Sets font used for text drawing in this widget.
	\param fontResource Pointer to font. Always use nullptr when you want to set the Default font or work properly with increments and decrements of font reference.
	*/
	proto void SetFont(ResourceName fontResource);
	//! Returns text in this widget. This is NOT a cheap operation - needs to copy the text.
	proto string GetText();
	//! Sets text where given params are inserted to appropriate places in given string
	proto void SetTextFormat(string text, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! Returns text size in pixels in reference resolution
	proto void GetTextSize(out float sx, out float sy);
	proto void GetShadowOffset(out float sx, out float sy);
	//! Sets text for the widget. String-table entries are translated.
	proto external void SetText(string text);
}

/*!
\}
*/
