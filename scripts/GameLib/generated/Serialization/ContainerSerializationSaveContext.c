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
class ContainerSerializationSaveContext: BaseSerializationSaveContext
{
	//! skipEmptyObjects : This feature is for writing only sub-objects to container which have some data and not creating hierarchy of empty sub-objects.
	void ContainerSerializationSaveContext(bool skipEmptyObjects = true) {}

	//! Returns the assigned container
	proto external BaseSerializationSaveContainer GetContainer();
	/*!
	Set the new assigned container.
	\param container The new container.
	*/
	proto external void SetContainer(BaseSerializationSaveContainer container);
}

/*!
\}
*/
