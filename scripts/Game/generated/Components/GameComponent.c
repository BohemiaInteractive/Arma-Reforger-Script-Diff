/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class GameComponent: GenericComponent
{
	/*!
	Called when Entity is being to be destroyed (deleted) or component to be deleted.
	\param owner Entity which owns the component
	*/
	event protected void OnDelete(IEntity owner);
	event bool OnTicksOnRemoteProxy() { return false; };
}

/*!
\}
*/
