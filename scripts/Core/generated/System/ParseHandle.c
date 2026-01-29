/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class ParseHandle: pointer
{
	/*!
	Parses line from file referenced in `parser` and tokenizes it to the `outParsedTokens` array
	The token separator is always a space character
	\return Number of tokens on line
	*/
	proto external int ParseLine(int lineNumber, out array<string> outParsedTokens);
	proto external void EndParse();
}

/*!
\}
*/
