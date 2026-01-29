/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

class BackendDebugApi
{
	private void BackendDebugApi();
	private void ~BackendDebugApi();

	//debug api is available only in developer mode
	//rate is always a float <0,1>
	//no host set == apply on any request
	//hostname has to contain the https:// prefix

	proto external void SetLag(int iLagMs);
	proto external void SetTimeoutRate(float fProb);
	proto external void SetErrorRate(float fProb);
	proto external void SetErrorCode(int iCode);
	proto external void SetTamperRate(float fProb);
	proto external void Reset();
	proto external void ApplyOnHost(string sHostName);
}

/*!
\}
*/
