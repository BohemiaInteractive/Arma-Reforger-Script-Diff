/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Replication
\{
*/

//! Conditional replication rule. Fine grained selection of receivers.
enum RplCondition
{
	//! Sent to all relevant by role only.
	None,
	//! Sent to owner only.
	OwnerOnly,
	//! Sent to everybody but owner.
	NoOwner,
	//! Sent to everybody who fulfills the custom condition.
	Custom,
}

/*!
\}
*/
