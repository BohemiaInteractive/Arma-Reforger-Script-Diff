/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System
\{
*/

sealed class PersistenceIdUtils
{
	private void PersistenceIdUtils();
	private void ~PersistenceIdUtils();

	/*!
	Set the hive id used for ID seperation on shared databases. Valid inclusive value range 0...16383.
	Note: Set by default on PersistenceSystem init based on CLI argument/server config.
	*/
	static proto bool ConfigureHive(int hiveid);
	//! Get the current hive id
	static proto int GetCurrentHiveId();
	/*!
	Generate a new dynamic pseudo-random RFC 9562 compliant UUID v8.

	Data layout (big-endian):
	Bit 000..047	48-bit Unix millisecond timestamp as seen in UUID v7 for lexicographic sorting.
	Bit 048..051	UUID Version 8 (b1000).
	Bit 052..063	Random data against predicition.
	Bit 064..065	UUID Variant fixed (b10).
	Bit 066..069	Bohemia Subvariant 1 (b0001).
	Bit 070..083	14-bit id of the current server in a hive, avoiding collisions on parallel generators.
	Bit 084..111	28-bit rolling sequence to guarantee >268M ids per millisecond without collision.
	Bit 112..128	More random data.
	*/
	static proto UUID Generate();
	//! Derive a deterministic UUID from a named/loaded entity
	static proto UUID FromEntity(IEntity entity, bool ignoreParents = false);
	/*!
	Derive a deterministic RFC 9562 compliant UUID v8 from an input string.

	Data layout (big-endian):
	Bit 000..007	Leading zero bits to ensure this id type sorts smaller than timestamped ids.
	Bit 008..047	FNV-1a hash from input string data
	Bit 048..051	UUID Version 8 (b1000).
	Bit 052..063	Hash continuation.
	Bit 064..065	UUID Variant fixed (b10).
	Bit 066..069	Bohemia Subvariant 2 (b0010).
	Bit 070..083	14-bit id of the current server in a hive, avoiding collisions on parallel generators.
	Bit 084..128	Hash continuation.
	*/
	static proto UUID FromString(string str);
	//! Get BI subvariant of persistence id.
	static proto int GetSubvariant(UUID uuid);
	/*!
	Get the approximate time the ID was created (+- 1 second).
	Note: Only relevant on generated ids. Deterministic IDs do not contain timestamp data.
	*/
	static proto int GetUnixTime(UUID uuid);
	//! Get the hive the ID was generated on - 0 by default if no hive is setup.
	static proto int GetHiveId(UUID uuid);
	/*!
	Sequence number during the generation.
	Note: Only relevant on generated ids. Deterministic IDs do not contain sequence data.
	Strictly for debug purposes only, as it is not predictable!
	*/
	static proto int GetSequence(UUID uuid);
}

/*!
\}
*/
