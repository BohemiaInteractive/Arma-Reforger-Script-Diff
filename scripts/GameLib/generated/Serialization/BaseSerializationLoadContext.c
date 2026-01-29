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
Base serialization context for loading data back to the game.
It works as an adapter between the game logic and serialized data.
*/
class BaseSerializationLoadContext: BaseSerializationContext
{
	//! Read a given value. Name of property is automatically derived from the input variable name.
	proto bool Read(out void value);
	//! Read an explictly named value.
	proto bool ReadValue(string name, out void value);
	//! s. Read, if the property was not found but the context type allows to seek members (e.g. json) it will return the default value provided.
	proto bool ReadDefault(out void value, void defaultValue);
	//! s. ReadValue andReadDefault
	proto bool ReadValueDefault(string name, out void value, void defaultValue);
	//! Manual map key reading, must be read in order (idx=0..N)
	proto bool ReadMapKey(int idx, out string key);
	/*!
	Check if the currently open object has any member with the given key name. This includes child objects.
	NOTE: Support depends on specific context implementation. Binary container will always return true!
	*/
	proto external bool DoesKeyExist(string name);
	/*!
	Check if the context has a child object of that name.
	NOTE: Support depends on specific context implementation. Binary container will always return true unless object seeking is enabled!
	*/
	proto external bool DoesObjectExist(string name);
}

/*!
\}
*/
