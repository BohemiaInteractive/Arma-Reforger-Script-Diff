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
Base serialization context for saving data from game.
It works as an adapter between the game logic and serialized data.
*/
class BaseSerializationSaveContext: BaseSerializationContext
{
	//! Write the value under the name of the variable passed.
	proto bool Write(void value);
	//! Write the value under the specified name.
	proto bool WriteValue(string name, void value);
	//! Write the value only it does not match the provided default value, or the context does not allow seeking (e.g. json=no default, binary=defaults written)
	proto bool WriteDefault(void value, void defaultValue);
	//! s. WriteValue and WriteDefault
	proto bool WriteValueDefault(string name, void value, void defaultValue);
	/*!
	Set the next map key for manual map writing (StartMap).
	NOTE: Only to be used if the value is complex (object, array, nested map). For primitive types use WriteValue("key", "value") for each entry instead.
	*/
	proto external bool WriteMapKey(string key);
}

/*!
\}
*/
