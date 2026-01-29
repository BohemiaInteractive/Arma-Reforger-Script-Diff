/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

//! Various entity events.
enum EntityEvent
{
	TOUCH,
	INIT,
	VISIBLE,
	FRAME,
	POSTFRAME,
	//! Animation system event
	ANIMEVENT,
	SIMULATE,
	POSTSIMULATE,
	// TODO: too specific. Make a universal physics system event
	JOINTBREAK,
	PHYSICSMOVE,
	CONTACT,
	PHYSICSACTIVE,
	FIXEDFRAME,
	POSTFIXEDFRAME,
	USER3,
	USER4,
	USER5,
	//! General event switch
	DISABLED,
	//! Everything except general switch
	ALL,
}

/*!
\}
*/
