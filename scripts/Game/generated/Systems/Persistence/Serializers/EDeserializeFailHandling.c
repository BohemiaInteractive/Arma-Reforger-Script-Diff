/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Serializers
\{
*/

sealed enum EDeserializeFailHandling
{
	//! Logging but issue will be ignored
	IGNORE,
	//! Further load on the instance is aborted and is deleted immediately
	DELETE,
	//! Same as DELETE, but global error count for OnAfterLoad is affected.
	ERROR,
}

/*!
\}
*/
