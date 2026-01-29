/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class ServerConfigApi
{
	private void ServerConfigApi();
	private void ~ServerConfigApi();

	proto external void SearchOnlineConfigs(BackendCallback callback);
	proto external int GetOnlineConfigs(out notnull array<ServerConfigMeta> configs);
	proto external void UploadConfig(string configName, BackendCallback callback);
	proto external ServerConfigMeta Download(string sId, BackendCallback callback);
}

/*!
\}
*/
