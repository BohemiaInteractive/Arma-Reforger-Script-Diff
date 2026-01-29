/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

enum TraceFlags
{
	//! Tracing against entities.
	ENTS,
	//! Tracing against terrain.
	WORLD,
	//! Tracing against ocean surface.
	OCEAN,
	//! Trace also against visibility occluders, like particles
	VISIBILITY,
	//! Stop on any contact. The best for visibility testing
	ANY_CONTACT,
	//! Do not stop on any contact, report them all through callback
	ALL_CONTACTS,
	//! Most common flag set.
	DEFAULT,
}

/*!
\}
*/
