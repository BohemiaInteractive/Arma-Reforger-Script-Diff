/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class RichTextWidget: TextWidget
{
	proto external float GetContentHeight();
	proto external float GetLineWidth(int line);
	proto external int GetNumLines();
	/*!
	Sets certain lines visible/hidden.
	\param lineFrom starting line. inclusive.
	\param lineTo ending line. exclusive.
	\param visible Visibility of the lines. true = visible, false = hidden.
	*/
	proto external void  SetLinesVisibility(int lineFrom, int lineTo, bool visible);
	/*!
	Elides the text
	\param maxLines indicates max. line, rest of lines are discarded (hidden).
	\param maxWidth Indicates when the text will be elided. from 0.0 to 1.0.

	e.g. ElideText(GetNumLines(), 1.0, "...") the text will be elided with "..."
	when reaches maximum width of parent widget.

	only works for left-to-right text, will not elide correctly for right aligned or centered texts.
	*/
	proto external void  ElideText(int maxLines, float maxWidth, string obsolete = string.Empty /*obsolete*/);
}

/*!
\}
*/
