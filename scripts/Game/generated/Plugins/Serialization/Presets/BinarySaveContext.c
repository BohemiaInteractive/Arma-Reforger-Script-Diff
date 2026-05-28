/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization_Presets
\{
*/

//! Container context saving in binary format.
class BinarySaveContext: SaveContainerContext
{
	proto external void ConfigureCompression(bool enabled);
	proto external void ConfigureNameValidation(bool enabled);
	proto external void ConfigureObjectSeeking(bool enabled);
	proto external void ConfigureSkippableObjects(bool enabled);
	proto external ref BinaryDataContainer SaveToContainer();
	proto external bool SaveToFile(string fileName);
}

/*!
\}
*/
