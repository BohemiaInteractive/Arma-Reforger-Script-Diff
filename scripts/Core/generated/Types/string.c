/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Types
\{
*/

sealed class string
{
	private void string();
	private void ~string();

	static const string Empty;

	/*!
	Gets the ASCII code of a character in the string.
	\param index Index of the character, 0 by default.
	*/
	proto external int ToAscii(int index = 0);
	/*!
	Parses a float from a string.
	Any whitespaces at the beginning in front of a number in the string will be skipped.

	\param default  Will be returned when the parsing fails (e.g. there's no number to parse)
	\param offset   Number of characters in the string to skip
	\param parsed   Out param - number of characters read

	\return The parsed float (the default if the parsing failed).

	\code
		"42.7".ToFloat()    // returns 42.7
		" 42abc".ToFloat()  // returns 42.0
		"abc42.7".ToFloat() // returns 42.7
		"abc42.7".ToFloat(default: 17.6)  // returns 17.6
		"abc42.7".ToFloat(offset: 3)		  // returns 42.7
		"42.7".ToFloat(parsed: len)       // returns 42.7, len = 4
		" 42.7".ToFloat(parsed: len)      // returns 42.7, len = 5
		"0".ToFloat(parsed: len)          // returns  0.0, len = 1
		"abc".ToFloat(parsed: len)        // returns  0.0, len = 0
		"   ".ToFloat(default: 17.6, parsed: len)      // returns 17.6, len = 0
		"abc 42.7 abc".ToFloat(offset: 3, parsed: len) // returns 42.7, len = 5
	\endcode
	*/
	proto external float ToFloat(float default = 0.0, int offset = 0, out int parsed = -1);
	/*!
	Parses an integer from a string.
	Any whitespaces at the beginning in front of a number in the string will be skipped.

	\param default  Will be returned when the parsing fails (e.g. there's no number to parse)
	\param offset   Number of characters in the string to skip
	\param parsed   Out param - number of characters read

	\return The parsed integer (the default if the parsing failed).

	\code
		"42".ToInt()      // returns 42
		" 42abc".ToInt()  // returns 42
		"abc42".ToInt()   // returns 0
		"abc42".ToInt(default: 17)  // returns 17
		"abc42".ToInt(offset: 3)    // returns 42
		"42".ToInt(parsed: len)     // returns 42, len = 2
		" 42".ToInt(parsed: len)    // returns 42, len = 3
		" 42.7".ToInt(parsed: len)  // returns 42, len = 3
		"0".ToInt(parsed: len)      // returns 0,  len = 1
		"abc".ToInt(parsed: len)    // returns 0,  len = 0
		"   ".ToInt(default: 17, parsed: len)      // returns 17, len = 0
		"abc 42 abc".ToInt(offset: 3, parsed: len) // returns 42, len = 3
	\endcode
	*/
	proto external int ToInt(int default = 0, int offset = 0, out int parsed = -1);
	/*!
	Returns a vector from a string.
	\return \p vector Converted s as vector
	\code
		string str = "1 0 1";
		vector v = str.ToVector();
		Print(v);

		>> v = <1,0,1>
	\endcode
	*/
	proto external vector ToVector();
	/*!
	Substring of `str` from `start` position `len` number of characters. (Maximum output string size is limited to 8191 characters)
	\param start Position in \p str
	\param len Count of characters
	\return \p string - Substring of \p str
	\code
		string str = "Hello World";
		string strSub = str.Substring(2, 5);
		Print(strSub);

		>> strSub = llo W
	\endcode
	*/
	proto external string Substring(int start, int len);
	/*!
	Returns trimmed string with removed leading and trailing whitespaces. (Maximum output string size is limited to 8191 characters)
	\return \p string - Trimmed string
	\code
		string str = " Hello World "
		Print( str );
		Print( str.Trim() );

		>> ' Hello World '
		>> 'Hello World'
	\endcode
	*/
	proto external string Trim();
	/*!
	Builds a string using given format and arguments. (Maximum output string size is limited to 8191 characters)
	\param fmt    Formatting string - any string with special tokens %1 .. %9.
	\param param1 Replaces the "%1" token in the formatting string in the result
	\param param2 Replaces the "%2" token

	\return The resulting string, i.e. the formatting string with all the %1 ... %9 replaced.

	\code
		int a = 5;
		float b = 5.99;
		string c = "beta";
		string 	test = string.Format("Ahoj %1 = %3 , %2", a, b, c);
		Print(test);
		>> 'Ahoj 5 = 'beta' , 5.99'
	\endcode
	*/
	static proto string Format(string fmt, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	/*!
	Removes leading and trailing whitespaces in string. Returns length
	\return \p int - Count of chars
	\code
		string str = " Hello World ";
		int i = str.TrimInPlace();
		Print(str);
		Print(i);

		>> str = 'Hello World'
		>> i = 11
	\endcode
	*/
	proto external int TrimInPlace();
	/*!
	Returns length of string.
	\return \p int - Length of string
	\code
		string str = "Hello World";
		int i = str.Length();
		Print(i);

		>> i = 11
	\endcode
	*/
	proto external int Length();
	/*!
	Determines if string is empty.
	\return     True if empty, False otherwise.
	*/
	proto external bool IsEmpty();
	/*!
	Returns hash of string.
	\return \p int - Hash of string
	\code
		string str = "Hello World";
		int hash = str.Hash();
		Print(hash);
	\endcode
	*/
	proto external int Hash();
	/*!
	\brief Finds 'sample' in 'str'. Returns -1 when not found
		\param sample \p string Finding string
		\return \p int - Returns position where \p sample starts, or -1 when \p sample not found
		@code
			string str = "Hello World";
			Print( str.IndexOf( "H" ) );
			Print( str.IndexOf( "W" ) );
			Print( str.IndexOf( "Q" ) );

			>> 0
			>> 6
			>> -1
		@endcode
	*/
	proto external int IndexOf(string sample);
	/*!
	\brief Finds last 'sample' in 'str'. Returns -1 when not found
		\param sample \p string Finding string
		\return \p int - Returns position where \p sample starts, or -1 when \p sample not found
		@code
			string str = "Hello World";
			Print( str.IndexOf( "l" ) );

			>> 9
		@endcode
	*/
	proto external int LastIndexOf(string sample);
	/*!
	\brief Finds 'sample' in 'str' from 'start' position. Returns -1 when not found
		\param start \p int Start from position
		\param sample \p string Finding string expression
		\return \p int - Length of string \p s
		@code
			string str = "Hello World";
			Print( str.IndexOfFrom( 3, "H" ) );
			Print( str.IndexOfFrom( 3, "W" ) );
			Print( str.IndexOfFrom( 3, "Q" ) );

			>> -1
			>> 6
			>> -1
		@endcode
	*/
	proto external int IndexOfFrom(int start, string sample);
	/*!
	\brief Retunrs true if sample is substring of string
		\param sample \p string Finding string expression
		\return \p bool true if sample is substring of string
		@code
			string str = "Hello World";
			Print( str.Contains("Hello") );
			Print( str.Contains("Mexico") );

			>> true
			>> false
		@endcode
	*/
	proto external bool Contains(string sample);
	/*!
	Checks whether the string contains a given substring at a given position.

	\param sample The substring to test
	\param pos    Position to test for the substring (index of the string's character)
	\return true if the string contains the given sample at the given position, false otherwise.

	\code
		"Hello World".ContainsAt("Hello", 0); // true (maybe use string.StartsWith() instead?)
		"Hello World".ContainsAt("World", 6); // true
		"Hello World".ContainsAt("Hello", 6); // false
	\endcode
	*/
	proto external bool ContainsAt(string sample, int pos);
	/*!
	Checks whether the string begins with a given substring.

	\param sample The substring to test
	\return true if the string starts with the given sample, false otherwise.

	\code
		"Hello World".StartsWith("Hello");    // true
		"Hello World".StartsWith("World");    // false
	\endcode
	*/
	proto external bool StartsWith(string sample);
	/*!
	\brief Retunrs true if string ends with sample, otherwise return false
		\param sample \p string Finding string expression
		\return \p bool true if string ends with sample
		@code
			string str = "Hello World";
			Print( str.StartsWith("Hello") );
			Print( str.StartsWith("World") );

			>> false
			>> true
		@endcode
	*/
	proto external bool EndsWith(string sample);
	/*!
	\brief Compares with sample and returns an integer less than, equal to, or greater than zero if string is less than, equal to, or greater than sample.
		\param sample \p string to campare with
		\return \p bool less than, equal to, or greater than zero if string is less than, equal to, or greater than sample
		@code
			string str = "Hello";
			Print( str.Compare("Hello") );
			Print( str.Compare("heLLo") );
			Print( str.Compare("heLLo", false) );

			>> 0
			>> -1
			>> 0
		@endcode
	*/
	proto external int Compare(string sample, bool caseSensitive = true);
	/*!
	\brief Replace all occurrances of 'sample' in 'str' by 'replace'
		\param sample string to search in \p str
		\param replace string which replace \p sample in \p str
		\return \p int - number of occurrances of 'sample' in 'str'
		@code
		string test = "If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.";
		Print(test);
		int count = test.Replace("the", "*");
		Print(count);
		Print(test);

		>> string test = 'If the length of the C string in source is less than num, only the content up to the terminating null-character is copied.';
		>> int count =   4
		>> string test = 'If * length of * C string in source is less than num, only * content up to * terminating null-character is copied.'
		@endcode
	*/
	proto external int Replace(string sample, string replace);
	/*!
	\brief Changes string to lowercase. Returns length. Works with just ASCII characters
		\return \p int - Length of changed string
		@code
			string str = "Hello World";
			int i = str.ToLower();
			Print(str);
			Print(i);

			>> str = hello world
			>> i = 11
		@endcode
	*/
	proto external int ToLower();
	/*!
	\brief Changes string to uppercase. Returns length. Works with just ASCII characters
		\return \p int - Length of changed string
		@code
			string str = "Hello World";
			int i = str.ToUpper();
			Print(str);
			Print(i);

			>> str = HELLO WORLD
			>> i = 11
		@endcode
	*/
	proto external int ToUpper();
	/*!
	Splits string into array of strings separated by `delimiter`.
	\param delimiter Strings separator
	\param[out] outTokens Array with strings
	\param removeEmptyEntries If true removes empty strings from outTokens array
	\code
		array<string> strs = {};
		string line = "The;quick;brown;fox;jumps;over;the;;dog;";
		line.Split(";", strs, true);

		for ( int i = 0; i < strs.Count(); i++ )
		{
			Print(strs.Get(i));
		}

		>> 'The'
		>> 'quick'
		>> 'brown'
		>> 'fox'
		>> 'jumps'
		>> 'over'
		>> 'the'
		>> 'dog'
	\endcode
	*/
	proto external void Split(string delimiter, notnull out array<string> outTokens, bool removeEmptyEntries);
	static proto string ToString(void var, bool type = false, bool name = false, bool quotes = true);
	/*!
	\brief Gets n-th character from string
		\param index character index
		\return \p string character on index-th position in string
		@code
			string str = "Hello World";
			Print( str[4] ); // Print( str.Get(4) );

			>> 'o'
		@endcode
	*/
	proto external string Get(int index);
	/*!
	Checks whether a character at a given position is a digit.

	\param index position of the character in the string
	\return true iff the character at the position is from the "0" .. "9" range
	*/
	proto external bool IsDigitAt(int index);
	/*!
	Checks whether a character at a given position is a whitespace.
	A whitespace may be e.g. Space (0x20 " "), Tab (0x09 "\t"), New Line (0x09 "\r", 0x0a "\n") etc.

	\param index position of the character in the string
	\return true iff the character at the position is a whitespace
	*/
	proto external bool IsSpaceAt(int index);
	/*!
	\brief Returns internal type representation. Can be used in runtime, or cached in variables and used for faster inheritance checking
		\returns \p typename Type of class
		@code
			???
		@endcode
	*/
	proto external typename ToType();
}

/*!
\}
*/
