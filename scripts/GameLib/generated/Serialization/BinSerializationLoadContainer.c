/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

/*!
Implementation of BaseLoadContainer to read data from json format.
*/
//! Container for loading data in binary format
class BinSerializationLoadContainer: BaseSerializationLoadContainer
{
	//! See BinSerializationSaveContainer.ConfigureCompression for reference.
	proto external void ConfigureCompression(bool enabled);
	//! See BinSerializationSaveContainer.ConfigureNameValidation for reference.
	proto external void ConfigureNameValidation(bool enabled);
	//! See BinSerializationSaveContainer.ConfigureObjectSeeking for reference.
	proto external void ConfigureObjectSeeking(bool enabled);
	//! See BinSerializationSaveContainer.ConfigureSkippableObjects for reference.
	proto external void ConfigureSkippableObjects(bool enabled);
	proto external bool LoadFromFile(string fileName);
	proto external bool LoadFromContainer(notnull BinSerializationContainer container);
}

/*!
\}
*/
