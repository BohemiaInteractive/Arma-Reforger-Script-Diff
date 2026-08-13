/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Snapshot of a group member's state, captured after the change has been applied (with the exception of MEMBER_LEFT and MEMBER_KICKED events).
sealed class GroupAuditLogMember: Managed
{
	private void GroupAuditLogMember();
	private void ~GroupAuditLogMember();

	proto external GroupMemberRole GetRole();
	proto external string GetNickname();
	proto external UUID GetGameIdentity();
}

/*!
\}
*/
