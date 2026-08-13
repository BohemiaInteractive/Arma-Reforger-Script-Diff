/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

enum EGroupAuditLogEventType
{
	//! A new group was created
	GROUP_CREATED,
	//! Group details were updated
	GROUP_DETAILS_UPDATED,
	//! A member was promoted to a higher role (target reflects the new role)
	MEMBER_PROMOTED,
	//! A member was demoted to a lower role (target reflects the new role)
	MEMBER_DEMOTED,
	//! The top role was transferred (snapshots reflect the roles after the swap)
	TOP_ROLE_TRANSFERRED,
	//! A member joined the group on their own initiative (target reflects the initial role)
	MEMBER_JOINED,
	//! A member joined the group by accepting an invitation (target reflects the initial role)
	MEMBER_INVITATION_ACCEPTED,
	//! A member joined the group after their application was approved (target reflects the initial role)
	MEMBER_APPLICATION_ACCEPTED,
	//! A member left the group (snapshot reflects the state before leaving)
	MEMBER_LEFT,
	//! A member was removed from the group by another member (target reflects the state at the time of the kick)
	MEMBER_KICKED,
}

/*!
\}
*/
