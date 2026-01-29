/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

//! Container for saving data in binary format
class BinSerializationSaveContainer: BaseSerializationSaveContainer
{
	/*!
	Enable bitstuffing and value compression for data size reduction.
	Leave disable when attempting to debug the data.
	Enabled by default. Disabled by default in developer builds
	*/
	proto external void ConfigureCompression(bool enabled);
	/*!
	> DEVELOPER BUILDS ONLY <
	Write each named variables name into the data stream to allow validation of correct read order.
	Enabled by default in developer builds. Parameter will do nothing in other builds.
	*/
	proto external void ConfigureNameValidation(bool enabled);
	/*!
	When enabled it adds meta data to allow StartObject("name") calls to seek to their starting position,
	making it possible to ignore prior properties or to exit the sub object and re-seek to its start for a retry.
	*/
	proto external void ConfigureObjectSeeking(bool enabled);
	/*!
	Adds neccessary meta data while writing objects so on load invalid/incomplete object reads can be skipped,
	while still making reads of the remaining binary stream possible. Can be disabled to decrease data size if successful read is guranteed,
	or loss of the data stream is an acceptable use-case.
	*/
	proto external void ConfigureSkippableObjects(bool enabled);
	proto external bool SaveToFile(string fileName);
	proto external ref BinSerializationContainer SaveToContainer();
}

/*!
\}
*/
