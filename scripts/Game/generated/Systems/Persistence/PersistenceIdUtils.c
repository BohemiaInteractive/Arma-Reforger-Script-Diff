/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

sealed class PersistenceIdUtils
{
	private void PersistenceIdUtils();
	private void ~PersistenceIdUtils();

	static proto UUID Generate(EPersistenceIdFlags flags = EPersistenceIdFlags.DYNAMIC);
	//! Get type/flags of persistence id
	static proto EPersistenceIdFlags GetFlags(UUID uuid);
	//! Get the approximate time the ID was created (+- 1 second). For DYNAMIC_ENTITY only!
	static proto int GetUnixTime(UUID uuid);
	//! Get the hive the ID was generated on - 0 by default if no hive is setup.
	static proto int GetHiveId(UUID uuid);
	//! Sequence number during the generation. Strictly for debug purposes only, as it is not predictable or reliable! For DYNAMIC_ENTITY only!
	static proto int GetSequence(UUID uuid);
}

/*!
\}
*/
