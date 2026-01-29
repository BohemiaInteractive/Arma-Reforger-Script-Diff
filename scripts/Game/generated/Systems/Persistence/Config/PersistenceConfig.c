/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Config
\{
*/

sealed class PersistenceConfig: ScriptAndConfig
{
	//! What collection this will be put in
	PersistenceCollection m_Collection;
	//! Chose on which save types this configuration will activate
	ESaveGameType m_eSaveMask;
	//! Delete the persistent record when the instance is destructed in playmode.
	bool m_bSelfDelete;
	//! Currently only constructed through the persistence system config
	private void PersistenceConfig();

	//! True if this config instance was modified and applied via SetConfig on the PersistenceSystem
	proto external bool IsScripted();
}

/*!
\}
*/
