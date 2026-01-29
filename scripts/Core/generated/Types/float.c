/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class float
{
	private void float();
	private void ~float();

	const float MAX = FLT_MAX;
	const float INFINITY = FLT_INF;

	/*!
	Float to string with fixed length, padded with zeroes.
	\param len \p int fixed length (overall including decimal part and floating point mark)
	\param lenDec \p int fixed length of decimal part
	\code
		float num = 123.456;
		string s1 = num.ToString(10, 4);
		string s2 = num.ToString();
		Print(s1);
		Print(s2);
		>> s1 = '00123.4560'
		>> s2 = '123.456'
	\endcode
	*/
	proto external string ToString(int len = -1, int lenDec = -1);
	/*!
	Return true when difference of two given float values is less or equal then epsilon.
	\code
		float a = 1.0;
		float b = 1.001;

		if (float.AlmostEqual(a,b))
			Print("are almost equal");

		>> 'are almost equal'
	\endcode
	*/
	static proto bool AlmostEqual(float a, float b, float epsilon = 0.0001);
	/*!
	Returns an integer that indicates the sign of a number.
	- For `floatNumber.Sign() < 0` result is -1.
	- For `floatNumber.Sign() == 0` result is 0.
	- For `floatNumber.Sign() > 0` result is 1.
	*/
	proto external int Sign();
}

/*!
\}
*/
