/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System_Config_Configurations
\{
*/

sealed class EntityPersistenceConfig: PersistenceConfig
{
	//! Automatically spawn back the entity on load
	bool m_bSelfSpawn;
	//! If disabled the entity can only be stored as child of others
	bool m_bStorageRoot;
	//! If and how to handle the entity being a child of a parent.
	EPersistenceParentHandling m_eParentHandling;

}

/*!
\}
*/
