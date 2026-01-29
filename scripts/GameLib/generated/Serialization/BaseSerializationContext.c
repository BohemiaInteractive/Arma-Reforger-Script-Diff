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
Base class for serialization contexts
*/
class BaseSerializationContext: ScriptAndConfig
{
	proto external bool CanSeekMembers();
	proto external bool IsValid();
	proto external string GetDataExtension();
	/*!
	Type discriminator is used to add polymorph object instance support to the serializer.
	Type is written inside the object when saving. When the load happens, it creates the instance based on this type.
	*/
	proto external void ConfigureTypeDiscriminator(string fieldName = "$type");
	/*!
	Enable or disable, can be used to pause it for a specific variable and resume after it was written.
	\return Previous value
	*/
	proto external bool EnableTypeDiscriminator(bool enabled);
	//! Is the type discrimiation currently enabled
	proto external bool IsTypeDiscriminatorEnabled();
	//! Reset buffers to re-use for new data.
	proto external void Reset();
	//! Start new map. Empty name will result in an anonymous map. Count must be known ahead of time
	proto bool StartMap(string name, out int count);
	proto external bool EndMap();
	//! Start new array. Empty name will result in an anonymous array. Count must be known ahead of time
	proto bool StartArray(string name, out int count);
	proto external bool EndArray();
	//! Start new subobject. Empty name will result in an anonymous object.
	proto external bool StartObject(string name = string.Empty);
	proto external bool EndObject();
}

/*!
\}
*/
