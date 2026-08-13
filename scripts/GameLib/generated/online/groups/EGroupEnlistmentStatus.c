/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

enum EGroupEnlistmentStatus
{
	//! Group applications don't need any approval, the user joins immediately.
	IMMEDIATE,
	//! Group member with permissions must approve the application for the user to join the group.
	APPROVAL,
	//! Any application to join the group is automatically rejected.
	FREEZE,
	//! Only users with an invitation from a group member can join.
	INVITATION_ONLY,
}

/*!
\}
*/
