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
Container for saving data in json format
*IMPORTANT* This should never be instantiated, you should use any of the child classes!!!
*/
class BaseJsonSerializationSaveContainer: BaseSerializationSaveContainer
{
	proto string ExportToString();
	proto external bool SaveToFile(string fileName);
	/*!
	Sets the maximum number of decimal places for float output.
	This setting truncates the output with specified number of decimal places.

	Example:
	@code
	JsonSaveContainer container();
	container.SetMaxDecimalPlaces(3);
	container.StartObject("aa");
	container.WriteValue("value", 0.12345); // "0.123"
	container.EndObject();
	@endcode
	*/
	proto external void SetMaxDecimalPlaces(int maxDecimalPlaces);
	proto external int GetMaxDecimalPlaces();
}

/*!
\}
*/
