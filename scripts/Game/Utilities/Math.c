/*!
\defgroup Math Math
Math library
\{
*/

/*!
Represents a single 2D curve, where in each vector only the `x` and `y` coordinates are used.

It is intended to be used as a property in following way:
\code
	[Attribute("", UIWidgets.GraphDialog)]
	Curve m_Curve;
\endcode

It may then be evaluated with:
\code
	LegacyCurve.Curve(ECurveType.CatmullRom, t, m_Curve);
\endcode
*/
class Curve : array<vector>
{
	// Empty class just to distinguish this array<vector> from the others
}

/*!
Represents three 2D curves, where every four floats next to each other in the underlying
array are organized as `(x, y0, y1, y2)`. %Curve points share the `x` value, and
then three separate `y` values are present, one for each curve.

It is indended to be used as a property in following way:
\code
	[Attribute("", UIWidgets.GraphDialog)]
	Curve3 m_Curve;
\endcode

It may then be evaluated with following code, where `i` is index of the curve to be evaluated,
ranging from 0 to 2:
\code
	Math3D.Curve3(ECurveType.CatmullRom, t, m_Curve, i);
\endcode
*/
class Curve3 : array<float>
{
	/*!
	\return Vector in format `(x, y, 0)`.
	*/
	vector GetPoint(int pointIndex, int curveIndex)
	{
		vector result;
		result[0] = this[pointIndex * 4];
		result[1] = this[pointIndex * 4 + curveIndex + 1];
		return result;
	}

	void SetPointValue(int pointIndex, int curveIndex, float value)
	{
		this[pointIndex * 4 + curveIndex + 1] = value;
	}
}

/*!
\}
*/