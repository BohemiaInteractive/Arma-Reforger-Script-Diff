/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

/*!
Builder for a new server ban. Fill in the target player, scope action, reason and
duration before passing it to ServerBanManagerApi.RequestCreateBan().

Create a blank instance with ServerBanManifest.CreateInstance(), or pre-fill one
from an existing ban entry using the ServerBanManifest.CreateFromBanEntry() factory.
*/
sealed class ServerBanManifest: Managed
{
	/*!
	Returns true if this manifest has all important parameters set with valid values.
	- Scope action is set.
	- BanList is set.
	- IdentityId of target user is set.
	- Duration OR ExpireAt OR Permanent is set to a valid expiration value
	*/
	proto external bool IsValid();
	//! Game identity ID of the user to ban.
	proto external void SetIdentityId(UUID identityId);
	//! Returns the game identity ID of the user to ban.
	proto external UUID GetIdentityId();
	//! Action component of the ban scope. System is always SERVER (implied by the endpoint).
	proto external void SetScopeAction(EBanScopeAction eAction);
	//! Returns the action component of the ban scope.
	proto external EBanScopeAction GetScopeAction();
	//! Sets the reason category for the ban.
	proto external void SetReasonType(EBanReasonType eType);
	//! Returns the reason category of the ban.
	proto external EBanReasonType GetReasonType();
	//! Sets the optional human-readable reason message for the ban.
	proto external void SetReasonMessage(string sMessage);
	//! Returns the human-readable reason message for the ban.
	proto external string GetReasonMessage();
	/*!
	Mark the ban to be permanent (without duration when it will expire).
	If set to true then SetDuration/SetExpireAt will not have any effect.
	*/
	proto external void SetPermanent(bool bPermanent);
	//! Returns true if the ban is permanent (no expiry).
	proto external bool IsPermanent();
	/*!
	Sets duration in seconds relative to creating time which determines time when ban is expected to expire.
	It is ignored when ban is marked as permanent.
	\note By default duration is initialized to -1 which is also considered as invalid value
	      which must be changed or ban must be marked as Permanent.
	\warning Relative duration is mutually exclusive with absolute time.
	         Setting duration will override previously set time via SetExpireAt
	*/
	proto external void SetDuration(int iSeconds);
	/*!
	Returns the ban duration in seconds relative to creation time, or -1 if not set.
	Check IsPermanent() first - duration is ignored when the ban is permanent.
	*/
	proto external int GetDuration();
	/*!
	Sets absolute time when ban will be expected to expire.
	It is ignored when ban is marked as permanent.
	\warning  Absolute time is mutually exclusive with relative duration.
	          Setting time will override previously set duration via SetDuration
	*/
	proto external void SetExpireAt(Timestamp time);
	/*!
	Returns the absolute time when the ban expires.
	Check IsPermanent() first - expire time is ignored when the ban is permanent.
	Returns a default-constructed Timestamp if not set.
	*/
	proto external Timestamp GetExpireAt();
	/*!
	Sets the target server. Required when managing bans from a game client rather than from a dedicated server.
	Pass null to clear a previously set server.
	*/
	proto external void SetServer(ServerInfo pServer);
	//! Returns the ID of the server targeted by this manifest.
	proto external string GetServerId();
	/*!
	Optional subset of writable ban lists to target. When empty, all writable ban lists for
	the server receive the ban. Ban list IDs are extracted from the objects.
	*/
	proto external void SetBanList(notnull BanList banList);
	//! Returns the ID of the ban list targeted by this manifest.
	proto external string GetBanListId();
	/*!
	Factory: creates a new manifest pre-filled from an existing ban entry, intended
	for edit flows (remove the old ban and issue a new one). Returns null when the
	target is not a player. The server must be set via SetServer() before submitting.
	*/
	static proto ref ServerBanManifest CreateFromBanEntry(notnull BanEntry pEntry);
}

/*!
\}
*/
