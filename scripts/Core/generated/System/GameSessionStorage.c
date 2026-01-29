/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

//! GameSessionStorage is used to store data for whole lifetime of game executable run. Here can be stored data which must survive scripts/addons reloading.
sealed class GameSessionStorage
{
	private void GameSessionStorage();
	private void ~GameSessionStorage();

	static ref map<string, string> s_Data = new map<string, string>();

}

/*!
\}
*/
