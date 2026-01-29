/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//!-----------------------------------------------------------------------------
//! object which allow to parse upon generic JSON structure and format it back
//!
class JsonApiStruct: Managed
{
	/*!
	\brief Register script variable for automatic processing of JSON stream
	*/
	proto external void RegV(string name);
	/*!
	\brief Unregister script variable from automatic processing of JSON stream
	*/
	proto external void UnregV(string name);
	/*!
	\brief Register all variable present on object for auto processing (it is not recursive!)
	*/
	proto external void RegAll();
	/*!
	\brief Start object at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto external void StartObject(string name);
	/*!
	\brief End object at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto external void EndObject();
	/*!
	\brief Add scripted object to hierarchy (calls through hierarchy)
	*/
	proto external void StoreObject(string name, JsonApiStruct obj);
	/*!
	\brief Add float value to hierarchy
	*/
	proto external void StoreFloat(string name, float value);
	/*!
	\brief Add integer value to hierarchy
	*/
	proto external void StoreInteger(string name, int value);
	/*!
	\brief Add boolean value to hierarchy
	*/
	proto external void StoreBoolean(string name, bool value);
	/*!
	\brief Add string value to hierarchy
	*/
	proto external void StoreString(string name, string value);
	/*!
	\brief Add vector value to hierarchy
	*/
	proto external void StoreVector(string name, vector value);
	/*!
	\brief Start array at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto external void StartArray(string name);
	/*!
	\brief End array at hierarchy - !!! Be cautious and doublecheck results when using this !!!
	*/
	proto external void EndArray();
	/*!
	\brief Add scripted unnamed/ array object
	*/
	proto external void ItemObject(JsonApiStruct obj);
	/*!
	\brief Add unnamed/ array float value
	*/
	proto external void ItemFloat(float value);
	/*!
	\brief Add unnamed/ array integer value
	*/
	proto external void ItemInteger(int value);
	/*!
	\brief Add unnamed/ array boolean value
	*/
	proto external void ItemBoolean(bool value);
	/*!
	\brief Add unnamed/ array string value
	*/
	proto external void ItemString(string value);
	/*!
	\brief Add unnamed/ array vector value
	*/
	proto external void ItemVector(vector value);
	/*!
	\brief Start an array inside an array
	*/
	proto external void ItemArray();
	/*!
	\brief Create JSON data from this object
	*/
	proto external void Pack();
	/*!
	\brief Start object initialization from provided RAW string data
	*/
	proto external void ExpandFromRAW(string data);
	/*!
	\brief Get object as JSON string (If no data are present - empty JSON string object is passed)
	*/
	proto external string AsString();
	/*!
	\brief Return true if stringified JSON are present (readable as string)
	*/
	proto external bool HasData();
	/*!
	\brief Create JSON data and save it to file
	*/
	proto external bool PackToFile(string FileName);
	/*!
	\brief Save to file (only if data are present from previous operation - load for example) as JSON data
	*/
	proto external bool SaveToFile(string FileName);
	/*!
	\brief Load JSON from file and use it to initialize this object
	*/
	proto external bool LoadFromFile(string FileName);

	// callbacks

	/*!
	\brief Event when expand (unpack) process starts
	*/
	event void OnExpand();
	/*!
	\brief Event when pack starts - you will pack your stuff here
	*/
	event void OnPack();
	/*!
	\brief	Event called when operation finished with Success
			errorCode is EJsonApiError
	*/
	event void OnSuccess(int errorCode);
	/*!
	\brief	Event called when operation finished with Error
			errorCode is EJsonApiError
	*/
	event void OnError(int errorCode);
	/*!
	\brief Called when parsing object
	*/
	event void OnObject(string name);
	/*!
	\brief Called when parsing array
	*/
	event void OnStartArray(string name);
	/*!
	\brief Called when array end, returns count of items
	*/
	event void OnEndArray(int itemCount);
	/*!
	\brief Called when parsing object
	*/
	event void OnItemObject(int index, string name);
}

/*!
\}
*/
