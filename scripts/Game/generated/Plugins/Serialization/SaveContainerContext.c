/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Serialization
\{
*/

/*!
Base serialization context for saving data to SaveContainer based class.
This context passes given data to set container class.
*/
class SaveContainerContext: SaveContext
{
	//! skipEmptyObjects : This feature is for writing only sub-objects to container which have some data and not creating hierarchy of empty sub-objects.
	void SaveContainerContext(bool skipEmptyObjects = true) {}

	//! Returns the assigned container
	proto external SaveContainer GetContainer();
	/*!
	Set the new assigned container.
	\param container The new container.
	*/
	proto external void SetContainer(SaveContainer container);
}

/*!
\}
*/
