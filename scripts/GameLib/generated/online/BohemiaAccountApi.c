/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

/*!
This API is used for linking user Game Identity with his Bohemia Account.

Bohemia Account can be linked only to one GameIdentity at once.
Attempts to link with additional GameIdentity will result in failure.
*/
sealed class BohemiaAccountApi
{
	private void BohemiaAccountApi();
	private void ~BohemiaAccountApi();

	/*!
	Will request link of Bohemia Account with current GameIdentity of client.
	\param callback callback where you will receive result when request finishes.
	\param email email address of Bohemia Account.
	\param password password of Bohemia Account.
	\param twoFactorToken token for two-factor-authentication. Can be left empty since it is required only if link fails with EApiCode::EACODE_ERROR_USER_MISSING_2FA.

	\note These error ApiCodes are related to linking of Bohemia Accounts and should be properly handled:
				- EApiCode::EACODE_ERROR_USER_LOCKED - account is currently locked and cannot be authenticated for link.
				- EApiCode::EACODE_ERROR_USER_INVALID_CREDENTIALS - invalid email or password.
				- EApiCode::EACODE_ERROR_USER_MISSING_2FA - this account requires two-factor-authentication token.
				- EApiCode::EACODE_ERROR_USER_INVALID_2FA - invalid two-factor-authentication token.
				- EApiCode::EACODE_ERROR_IDENTITY_CONFLICT_IDENTITY - account is already linked to different GameIdentity.
	*/
	static proto void Link(notnull BackendCallback callback, string email, string password, string twoFactorToken);
	//! Will request unlink of Bohemia Account from current Game Identity of client.
	static proto void Unlink(notnull BackendCallback callback);
	/*!
	Returns remaining time in seconds until locked account from last attempt is unlocked.
	Will reset if there is attempt to link with different account or if links returns different error than account locked.
	*/
	static proto int GetSecondsUntilAccountUnlockTime();
	//! Returns true if current Game Identity of client is already linked with Bohemia Account.
	static proto bool IsLinked();
	/*!
	Returns email of linked Bohemia Account.
	\note When called in modded game this method will always return censored email.
	      In Workbench it can return uncensored email even with mods if user gives his consent.
	*/
	static proto string GetEmail();
	//! Returns username of linked Bohemia Account.
	static proto string GetName();
}

/*!
\}
*/
