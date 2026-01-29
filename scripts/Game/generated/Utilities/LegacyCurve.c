/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Utilities
\{
*/

sealed class LegacyCurve
{
	private void LegacyCurve();
	private void ~LegacyCurve();

	/*!
	Computes curve. Knots array is used only for non-uniform curve types. For example,
	CatmullRom and CurveProperty2D.
	\code
		auto points = new array<vector>();
		points.Insert( Vector( 0, 0, 0) );
			points.Insert( Vector( 5, 0, 0) );
			points.Insert( Vector( 8, 3, 0) );
			points.Insert( Vector( 6, 1, 0) );

		float t = 0.5;
		vector result = LegacyCurve.Curve(ECurveType.CatmullRom, t, points);
	\endcode
	*/
	static proto vector Curve(ECurveType type, float param, notnull array<vector> points, array<float> knots = null);
	/*!
	Evaluates a single curve from multicurve object.
	\see LegacyCurve.Curve
	*/
	static proto vector Curve3(ECurveType type, float param, notnull Curve3 points, int curveIndex, array<float> knots = null);
}

/*!
\}
*/
