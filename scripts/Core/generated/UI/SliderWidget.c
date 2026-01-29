/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class SliderWidget: UIWidget
{
	//! Set min and max in a single call
	proto external void SetRange(float min, float max);
	proto external void SetMin(float min);
	proto external void SetMax(float max);
	proto external float GetMax();
	proto external float GetMin();
	proto external void SetStep(float step);
	proto external float GetStep();
	proto external float GetCurrent();
	proto external void SetCurrent(float curr);
}

/*!
\}
*/
