/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

//! Scripted save container for custom handling of storage
class ScriptedSerializationSaveContainer: BaseSerializationSaveContainer
{
	//! Reset the container to re-use for new data.
	event protected void Reset();
	//! Data extension of the serialized format (e.g. txt, csv, yml) if stored in a filesystem.
	event protected string GetDataExtension() {return ".txt";};
	//! Export the data as string to be written to disk etc.
	event protected string ExportToString() {return "";};
	/*!
	Objects are serialized as sequence of:
	StartObject()
	WriteKey("myValue");
	WriteInt(42);
	WriteKey("anotherValue")
	WriteFloat(13.37);
	...
	EndObject();

	There may be sub-objects/arrays/maps. These will be written using a sequence of WriteKey(...) + StartObject/Array/Map().
	Each nested object/array/map will be terminated by EndObject/Array/Map();
	*/
	event protected bool StartObject() {return false;};
	event protected bool EndObject() {return false;};
	/*!
	Arrays don't have named values and as such are only a sequence of value writes.
	Arrays may be nested, in which case StartArray is called while already inside an array.
	Sets are considered arrays for serialization.
	*/
	event protected bool StartArray(int count) {return false;};
	event protected bool EndArray() {return false;};
	/*!
	Maps are key value pairs which are written similar to objects, as sequence of WriteKey(key) + WriteXXX(value);
	Keys are converted automatically to strings for serialization for cases such as map<int, float>.
	The value type of the map is known one the first value write is called.
	*/
	event protected bool StartMap(int count) {return false;};
	event protected bool EndMap() {return false;};
	//! Key writer for named properties in objects or map keys.
	event protected bool WriteKey(string key) {return false;};
	//! Typed value writers
	event protected bool WriteNull() {return false;};
	event protected bool WriteBool(bool value) {return false;};
	event protected bool WriteString(string value) {return false;};
	event protected bool WriteInt(int value) {return false;};
	event protected bool WriteFloat(float value) {return false;};
	event protected bool WriteVector(vector value) {return false;};
}

/*!
\}
*/
