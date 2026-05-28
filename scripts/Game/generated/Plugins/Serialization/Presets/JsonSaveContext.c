/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization_Presets
\{
*/

//! Container context saving in JSON format.
class JsonSaveContext: SaveContainerContext
{
	proto external string SaveToString();
	proto external bool SaveToFile(string fileName);
	proto external void SetMaxDecimalPlaces(int maxDecimalPlaces);
	proto external int GetMaxDecimalPlaces();
}

/*!
\}
*/
