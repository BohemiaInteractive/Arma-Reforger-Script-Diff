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
Base serialization context for saving data to BaseSerializationSaveContainer based class.
This context passes given data to set container class.
*/
class ContainerSerializationLoadContext: BaseSerializationLoadContext
{
	//! skipEmptyObjects : This feature is for loading data where only objects with some data are present and empty objects are skipped.
	//!		If this feature is enabled, the context won't fail on StartObject method if it's not present in the data. Only
	//!		when caller attempts to read key from this missing object it ends with error.
	void ContainerSerializationLoadContext(bool skipEmptyObjects = true) {}

	//! Returns the assigned container
	proto external BaseSerializationLoadContainer GetContainer();
	/*!
	Set the new assigned container.
	\param container The new container.
	*/
	proto external void SetContainer(BaseSerializationLoadContainer container);
}

/*!
\}
*/
