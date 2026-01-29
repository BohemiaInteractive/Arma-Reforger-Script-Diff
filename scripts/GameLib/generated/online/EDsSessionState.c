/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Session state
enum EDsSessionState
{
	//! session is not initialized
	EDSESSION_OFFLINE,
	//! session is handling online services, connections and loading initial data
	EDSESSION_LAUNCHING,
	//! session is waiting for script/ game to finish initialization (load world) and such
	EDSESSION_INIT,
	//! session is running - players can connect
	EDSESSION_ACTIVE,
	//! session is being terminated
	EDSESSION_CLOSING,
	//! session is paused (this is state where server was hibernated)
	EDSESSION_PAUSED,
	//! session is restarted (in process)
	EDSESSION_RESTARTING,
}

/*!
\}
*/
