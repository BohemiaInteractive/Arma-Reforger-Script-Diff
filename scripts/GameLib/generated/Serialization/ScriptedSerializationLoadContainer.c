/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Serialization
\{
*/

//! Scripted load container for custom handling of storage
class ScriptedSerializationLoadContainer: BaseSerializationLoadContainer
{
	event protected bool StartObject() {return false;};
	event protected bool EndObject() {return false;};
	event protected bool StartArray(out int count) {return false;};
	event protected bool EndArray() {return false;};
	event protected bool StartMap(out int count) {return false;};
	event protected bool EndMap() {return false;};
	//! Key reader for named properties in objects or map keys.
	event protected bool ReadKey(inout string key) {return false;};
	event protected bool IsNull() {return false;};
	event protected bool ReadBool(out bool value) {return false;};
	event protected bool ReadString(out string value) {return false;};
	event protected bool ReadInt(out int value) {return false;};
	event protected bool ReadFloat(out float value) {return false;};
	event protected bool ReadVector(out vector value) {return false;};

	//! Reset the container to re-use for new data.
	event protected void Reset();
	//! Data extension of the serialized format (e.g. txt, csv, yml) if stored in a filesystem.
	event protected string GetDataExtension() {return ".txt";};
	event protected bool ImportFromString(string data) {return false;};
}

/*!
\}
*/
