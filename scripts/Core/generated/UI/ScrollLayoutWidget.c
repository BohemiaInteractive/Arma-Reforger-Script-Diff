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
ScrollLayoutWidget class
*/
sealed class ScrollLayoutWidget: UIWidget
{
	proto external void ScrollToView(float x, float y, float width, float height);
	/*!
	Get the position of the slider in each direction from 0 (not scrolled at all) to 1 (scrolled all the way)
	*/
	proto void GetSliderPos(out float x, out float y);
	/*!
	Set the position of the slider in each direction. Clamped between 0 (not scrolled at all) to 1 (scrolled all the way).
	For ScrollLayoutWidget scrolling in only one direction the slider position in the other direction can be safely set to 0.
	E.g., for vertical scroll layout, set y to desired scroll amount and x to anything, for example 0.
	*/
	proto external void SetSliderPos(float x, float y);
	/*!
	Return slider position in screen space pixels. For more info see GetSliderPos
	*/
	proto void GetSliderPosPixels(out float x, out float y);
	/*!
	Set slider position in screen space pixels. For more info see SetSliderPos
	*/
	proto external void SetSliderPosPixels(float x, float y);
}

/*!
\}
*/
