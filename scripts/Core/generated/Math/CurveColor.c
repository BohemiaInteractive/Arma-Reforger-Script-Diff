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
Curve class for Color values. Expected to use as a property (using Attribute) and filled from config edited in dedicated Curve dialog in Workbench.
\ref UIWidgets.CurveDialog for property params details

\code
	class ConfigExample
	{
		[Attribute(uiwidget: UIWidgets.CurveDialog, params:"type=Akima, ends=OpenFlat, paramRange=0 1 fixed, valueRange=0 1 fixed")]
		ref CurveColor m_Curve;
	}

	void UsageExample(ConfigExample c)
	{
		Color val = Color.Black;
		for (int i = 0; i < 10; i++)
		{
			c.m_Curve.Compute(0.1 * i, val);
			Print("[" + i + "] = " + val);
			}
	}
\endcode
*/
class CurveColor: Managed
{
	/*!
	Compute value at x.
	This method finds segment which x belongs to and performs local interpolation.
	*/
	proto external void Compute(float param, out notnull Color color);
	/*!
	Is the curve empty and thus using default value?
	*/
	proto external bool IsEmpty();
}

/*!
\}
*/
