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
Curve class for float values. Expected to use as a property (using Attribute) and filled from config edited in dedicated Curve dialog in Workbench.
\ref UIWidgets.CurveDialog for property params details

\code
	class ConfigExample
	{
		[Attribute(uiwidget: UIWidgets.CurveDialog, params:"type=Akima, ends=OpenFlat, paramRange=0 1 fixed, valueRange=0 1 fixed")]
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
