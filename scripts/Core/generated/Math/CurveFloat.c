/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Math
\{
*/

/*!
Curve class for float values. Expected to use as a property (using Attribute) and filled from config edited in dedicated Curve dialog in Workbench. But you can also initialize the curve programmatically using InitSpline or InitCustomSpline.
\ref UIWidgets.CurveDialog for property params details

\code
	class ConfigExample
	{
		[Attribute(params:"type=Akima, ends=OpenFlat, paramRange=0 1 fixed, valueRange=0 1 fixed")]
		ref CurveFloat m_Curve;
	}

	void UsageExample(ConfigExample c)
	{
		for (int i = 0; i < 10; i++)
		{
			float val = c.m_Curve.Compute(0.1 * i);
			Print("[" + i + "] = " + val);
			}
	}
\endcode
*/
class CurveFloat: Managed
{
	/*!
	Initialize as a spline from values and automatically compute tangents.
	\param knots					Knot parametrization, can be nullptr to assign uniform parametrization.
												Knots must be sorted, each following value greater than previous.
	\param values					Values at knots.
	\param type						Type of interpolation between knots (how tangents are generated).
	\param ends						Behavior at the ends of the curve.
	\param paramMin				The knot param range, also defines the period for closed curves, infinite by default.
	\param paramMax				The knot param range, also defines the period for closed curves, infinite by default.
	\param valueMin				The range for curve value clamping, infinite by default.
	\param valueMax				The range for curve value clamping, infinite by default.
	*/
	proto external void InitSpline(notnull array<float> knots, notnull array<float> values, ECurveType type = ECurveType.Akima, ECurveEnds ends = ECurveEnds.Open, float paramMin = -float.MAX, float paramMax = float.MAX, float valueMin = -float.MAX, float valueMax = float.MAX);
	/*!
	Initialize as a spline from values and assign custom tangents.
	\param knots					Knot parametrization, can be nullptr to assign uniform parametrization.
												Knots must be sorted, each following value greater than previous.
	\param values					Values at knots.
	\param tangentsL			Gradient at knots to the left.
	\param tangentsR			Gradient at knots to the right.
	\param ends						Behavior at the ends of the curve.
	\param paramMin				The knot param range, also defines the period for closed curves, infinite by default.
	\param paramMax				The knot param range, also defines the period for closed curves, infinite by default.
	\param valueMin				The range for curve value clamping, infinite by default.
	\param valueMax				The range for curve value clamping, infinite by default.
	*/
	proto external void InitCustomSpline(notnull array<float> knots, notnull array<float> values, notnull array<float> tangentsL, notnull array<float> tangentsR, ECurveEnds ends = ECurveEnds.Open, float paramMin = -float.MAX, float paramMax = float.MAX, float valueMin = -float.MAX, float valueMax = float.MAX);
	/*!
	Compute value at x.
	This method finds segment which x belongs to and performs local interpolation.
	*/
	proto external float Compute(float param);
	/*!
	Is the curve empty and thus using default value?
	*/
	proto external bool IsEmpty();
}

/*!
\}
*/
