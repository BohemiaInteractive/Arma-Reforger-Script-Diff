/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Represents a single scheduled event belonging to a Group.
sealed class GroupEvent: Managed
{
	private void GroupEvent();
	private void ~GroupEvent();

	proto external int GetGroupId();
	proto external string GetTitle();
	proto external string GetDescription();
	// Returns absolute moment the event starts (UTC instant).
	proto external Timestamp GetStartInstant();
	//! Returns absolute moment the event ends (UTC instant).
	proto external Timestamp GetEndInstant();
	proto external int GetMaxParticipants();
	//! Returns time when the event was created.
	proto external Timestamp GetCreatedAt();
	//! Adds the calling player as a participant of the event. Caller must be a member of the group.
	proto external void Join(notnull BackendCallback pCallback);
	//! Removes the calling player from the event's participants. Caller must be a member of the group.
	proto external void Leave(notnull BackendCallback pCallback);
	//! Deletes this event. Caller must have permission to edit the group.
	proto external void Delete(notnull BackendCallback pCallback);
}

/*!
\}
*/
