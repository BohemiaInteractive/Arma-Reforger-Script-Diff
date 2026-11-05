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
	/*!
	Sets certain lines visible/hidden.
	\param lineFrom starting line. inclusive.
	\param lineTo ending line. exclusive.
	\param visible Visibility of the lines. true = visible, false = hidden.
	*/
	proto external void  SetLinesVisibility(int lineFrom, int lineTo, bool visible);
}

/*!
\}
*/
