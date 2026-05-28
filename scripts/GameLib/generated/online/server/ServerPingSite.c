/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

class ServerPingSite: Managed
{
	private void ServerPingSite();
	private void ~ServerPingSite();

	//! Returns unique ID of this site.
	proto external string GetId();
	/*!
	Returns prettified name of this site.
	\note This can possibly return localization key which
	      could be used to show name in UI and proper language
	      preference.
	*/
	proto external string GetName();
	//! Returns true if this site was already measured and has usable metrics available.
	proto external bool IsReady();
	/*!
	Returns min measured Round-Trip Time of ping in ms.
	\note Returns -1 when measurement is not ready yet.
	*/
	proto external int GetMinRTT();
	/*!
	Returns average measured Round-Trip Time of ping in ms.
	\note Returns -1 when measurement is not ready yet.
	*/
	proto external int GetAvgRTT();
	/*!
	Returns max measured Round-Trip Time of ping in ms.
	\note Returns -1 when measurement is not ready yet.
	*/
	proto external int GetMaxRTT();
}

/*!
\}
*/
