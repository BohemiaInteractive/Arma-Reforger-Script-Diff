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
This API is used for accessing current state of Authenticator for backend services from client game.
Authentication is handled in background during entire lifetime of application if it is enabled.

\note This API is only intended for client game and will always be disabled on Dedicated Server which.
      Server use different authentication which uses different API (TBD).
      Listen Server which is running on client would use both this and server auth APIs.
*/
sealed class BackendAuthenticatorApi
{
	private void BackendAuthenticatorApi();
	private void ~BackendAuthenticatorApi();

	/*!
	Return true if authentication with backend is enabled. False if disabled.
	\note Disabled state can be interpreted as signed out state when we are not attempting to reconnect or sign in.
	      Online services which require authentication will not be accessible in this state.
	      State of authentication is set by platform service which determines if user is signed in or not.
	*/
	static proto bool IsEnabled();
	/*!
	Returns true if we are currently attempting to newly authenticate or refresh authentication of client.
	\note This can be true even when already authenticated because we can currently be refreshing authentication.
	*/
	static proto bool IsAuthInProgress();
	//! Returns true if client is currently authenticated with backend and online services can be accessed.
	static proto bool IsAuthenticated();
	/*!
	Returns Identity Id of locally authenticated user. User has to be at least once authenticated to receive ID.
	\note On Dedicated servers this value will be always Null.
	*/
	static proto UUID GetIdentityId();
}

/*!
\}
*/
