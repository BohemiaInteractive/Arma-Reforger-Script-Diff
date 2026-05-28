/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization_Presets
\{
*/

//! Container context saving in prettified JSON format.
class PrettyJsonSaveContext: JsonSaveContext
{
	proto external void SetFormatOptions(EPrettyFormatOptions options);
	proto external bool SetIndent(string indentChar, int indentCharCount);
}

/*!
\}
*/
