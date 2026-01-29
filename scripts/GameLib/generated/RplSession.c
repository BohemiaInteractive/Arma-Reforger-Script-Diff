/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class RplSession
{
	private void RplSession();
	private void ~RplSession();

	/*!
	Try to obtain IP address to listen at when running in RplMode::Listen mode (as server).
	\return     IP as string on success or empty string when IP address could not be determined.
	*/
	static proto owned string TryGetListenAddress();
	//! Registers callbacks for current session.
	static proto void RegisterCallbacks(RplSessionCallbacks callbacks);
	//! Current mode of the replication.
	static proto RplMode Mode();
}
