/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Represents a single entry in the group audit log, describing who performed an action, who was affected, and when.
sealed class GroupAuditLogEvent: Managed
{
	private void GroupAuditLogEvent();
	private void ~GroupAuditLogEvent();

	proto external EGroupAuditLogEventType GetEventType();
	/*!
	Based on the event type this equals to:
	GROUP_CREATED, GROUP_DETAILS_UPDATED - the member who created the group / made changes
	MEMBER_PROMOTED, TOP_ROLE_TRANSFERRED, MEMBER_DEMOTED - the member who performed the promotion / demotion
	MEMBER_JOINED - the member who joined (same as target)
	MEMBER_INVITATION_ACCEPTED - the member who sent the invitation (nickname and role may be null, if they have since left the group)
	MEMBER_APPLICATION_ACCEPTED - the member who approved the application
	MEMBER_LEFT - the member who left (same as target)
	MEMBER_KICKED - the member who performed the kick
	*/
	proto external GroupAuditLogMember GetCaller();
	/*!
	Based on the event type this equals to:
	GROUP_CREATED, GROUP_DETAILS_UPDATED - null
	MEMBER_PROMOTED, TOP_ROLE_TRANSFERRED, MEMBER_DEMOTED - the member who was promoted / demoted
	MEMBER_JOINED - the member who joined (same as caller)
	MEMBER_INVITATION_ACCEPTED - the member who accepted the invitation
	MEMBER_APPLICATION_ACCEPTED - the member whose application was approved
	MEMBER_LEFT - the member who left (same as caller)
	MEMBER_KICKED - the member who was kicked
	*/
	proto external GroupAuditLogMember GetTarget();
}

/*!
\}
*/
