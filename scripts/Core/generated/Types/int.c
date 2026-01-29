/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class int
{
	private void int();
	private void ~int();

	const int MAX = 2147483647;
	const int MIN = -2147483648;
	const int INVALID_INDEX = -1;

	/*!
	Converts ASCII code to string.
	\return `string` - Converted `int`.
	\code
		int ascii_code = 77;
		string str = ascii_code.AsciiToString();
		Print(str);

		>> str = 'M'
	\endcode
	*/
	proto external string AsciiToString();
	/*!
	Integer to string with fixed length, padded with zeros.
	\param len \p int Fixed length of output string, number will be padded with '0' from left.
	\param hex \p bool If true, formats the value as hexadecimal. Otherwise, formats as decimal.
	\param prefix \p string Prefixes the hexadecimal output (e.g., "#FF00AA").
	\code
		int num = 123;
		string s1 = num.ToString(5);
		string s2 = num.ToString();
		string s3 = num.ToString(len: 4, hex: true, prefix: "#");
		Print(s1);
		Print(s2);
		Print(s3);
		>> s1 = '00123'
		>> s2 = '123'
		>> s3 = '#007B'
	\endcode
	*/
	proto external string ToString(int len = -1, bool hex = false, string prefix = string.Empty);
	/*!
	Returns an integer that indicates the sign of a number.
	- For `intNumber.Sign() < 0` result is -1.
	- For `intNumber.Sign() == 0` result is 0.
	- For `intNumber.Sign() > 0` result  is 1.
	*/
	proto external int Sign();
}

/*!
\}
*/
