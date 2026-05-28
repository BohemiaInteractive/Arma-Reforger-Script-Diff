/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization
\{
*/

//! Container for loading data in binary format
class BinaryLoadContainer: LoadContainer
{
	//! See BinarySaveContainer.ConfigureCompression for reference.
	proto external void ConfigureCompression(bool enabled);
	//! See BinarySaveContainer.ConfigureNameValidation for reference.
	proto external void ConfigureNameValidation(bool enabled);
	//! See BinarySaveContainer.ConfigureObjectSeeking for reference.
	proto external void ConfigureObjectSeeking(bool enabled);
	//! See BinarySaveContainer.ConfigureSkippableObjects for reference.
	proto external void ConfigureSkippableObjects(bool enabled);
	proto external bool LoadFromContainer(notnull BinaryDataContainer container);
	proto external bool LoadFromFile(string fileName);
}

/*!
\}
*/
