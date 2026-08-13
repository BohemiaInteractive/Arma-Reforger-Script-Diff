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
	[Obsolete()]
	void RequestShutdown() {}
	[Obsolete()]
	void RequestActive() {}
	[Obsolete()]
	void RequestFinish() {}
	[Obsolete()]
	void RequestRestart() {}
	[Obsolete()]
	float GetUpTime() { return 0; }
	[Obsolete()]
	string Name() { return ""; }
	[Obsolete()]
	string RoomID() { return ""; }
	[Obsolete()]
	string ScenarioID() { return ""; }

	/*!
	\brief Status of the hosted instance (EDsSessionState)
	*/
	[Obsolete()]
	proto external int Status();
	/*!
	\brief Active Status of the hosted instance (Temporary for AR compatibility)
	*/
	[Obsolete()]
	proto external bool StatusActive();
	/*!
	\brief Player limit on the instance
	*/
	[Obsolete()]
	proto external int PlayerLimit();
	/*!
	\brief Player count on the instance
	*/
	[Obsolete()]
	proto external int PlayerCount();
}

/*!
\}
*/
