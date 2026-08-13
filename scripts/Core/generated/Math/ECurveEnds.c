/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Math
\{
*/

//! Behavior at the ends of the curve.
enum ECurveEnds
{
	//! Free ends and extrapolation from the ends
	Open,
	//! Free ends, derivative may be non-zero but value is fixed outside the curve
	OpenFlat,
	//! Zero derivative at the ends, fixed value outside the curve
	Flat,
	//! Looped curve inside param range: y(max) = y(min), y'(max) = y'(min)
	Closed,
}

/*!
\}
*/
