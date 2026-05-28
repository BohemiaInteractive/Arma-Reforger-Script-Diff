/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization_Presets
\{
*/

//! Container context loading binary format.
class BinaryLoadContext: LoadContainerContext
{
	proto external void ConfigureCompression(bool enabled);
	proto external void ConfigureNameValidation(bool enabled);
	proto external void ConfigureObjectSeeking(bool enabled);
	proto external void ConfigureSkippableObjects(bool enabled);
	proto external bool LoadFromContainer(notnull BinaryDataContainer container);
	proto external bool LoadFromFile(string fileName);
}

/*!
\}
*/
