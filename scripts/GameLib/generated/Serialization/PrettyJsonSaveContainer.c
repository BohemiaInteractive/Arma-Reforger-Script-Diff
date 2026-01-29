/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

//! Container for saving data in pretty json format
class PrettyJsonSaveContainer: BaseJsonSerializationSaveContainer
{
	//! Set pretty writer formatting options.
	proto external void SetFormatOptions(EPrettyFormatOptions options);
	/*!
	Set the character and count for indentation.

	\param indentChar Character for indentation. Must be whitespace character (' ', '\\t', '\\n', '\\r').
	\param indentCharCount Number of indent characters for each indentation level.
	*/
	proto external bool SetIndent(string indentChar, int indentCharCount);
}

/*!
\}
*/
