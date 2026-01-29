/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! DS server Session
class DSSession
{
	private void DSSession();
	private void ~DSSession();

	/*!
	\brief Name of the the session
	*/
	proto external string Name();
	/*!
	\brief Status of the hosted instance (EDsSessionState)
	*/
	proto external int Status();
	/*!
	\brief Player limit on the instance
	*/
	proto external int PlayerLimit();
	/*!
	\brief Player count on the instance
	*/
	proto external int PlayerCount();
	/*!
	\brief Request termination of server
	*/
	proto external void RequestShutdown();
	/*!
	\brief Request server to enter active state
	*/
	proto external void RequestActive();
	/*!
	\brief Request finishing of hosted session (game won, draw, canceled)
	*/
	proto external void RequestFinish();
	/*!
	\brief Request restart of hosted session (with or without new parameters)
	*/
	proto external void RequestRestart();
	/*!
	\brief Get current session uptime in seconds
	*/
	proto external float GetUpTime();
	/*!
	\brief ID of Room created on server
	*/
	proto external string RoomID();
	/*!
	\brief ID of Scenario hosted on server
	*/
	proto external string ScenarioID();
}

/*!
\}
*/
