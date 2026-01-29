/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

//! Role of replicated node (and all items in it) within the replication system.
enum RplRole
{
	//! The state will be replicated to proxies.
	Authority,
	//! Can't replicate state to others, only receives state from authority.
	Proxy,
}

/*!
\}
*/
