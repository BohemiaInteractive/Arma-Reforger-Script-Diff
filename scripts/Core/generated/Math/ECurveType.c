/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Math
\{
*/

//! Type of interpolation between knots (how tangents are generated).
enum ECurveType
{
	//! C1-continuous spline (position, tangents)
	CatmullRom,
	//! Catmull-Rom-like spline, avoid it if possible
	Legacy,
	//! Catmull-Rom-like spline, avoid it if possible
	CurveProperty2D,
	//! Akima spline provides human-like interpolation (based on 6 people in the original paper)
	Akima,
	//! Snappier Akima spline which doesn't overshoot
	ModifiedAkima,
	//! C0-continuous spline (position)
	Linear,
	//! C2-continuous spline (position, tangents, acceleration)
	Natural,
}

/*!
\}
*/
