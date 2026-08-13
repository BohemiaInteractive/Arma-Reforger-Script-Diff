/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System_Serializers
\{
*/

class ScriptedStateSerializer: PersistenceSerializerBase
{
	//! Native spawn data serialization logic that can be called explictly if script overrides SerializeSpawnData.
	proto external protected ESerializeResult SerializeSpawnDataNative(notnull Managed instance, notnull SaveContext context, SerializerDefaultSpawnData defaultData = null);
	//! Native serialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected ESerializeResult SerializeNative(notnull Managed instance, notnull SaveContext context, ESerializeMode mode = ESerializeMode.BOTH);
	//! Call the native handling of state spawning. Will return an instance of the given target type and attempt to re-use any passed instance.
	proto external protected ref Managed DeserializeSpawnDataNative(Managed instance, SaveContext context, bool callConstructor = true);
	//! Native deserialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected bool DeserializeNative(notnull Managed instance, notnull LoadContext context, ESerializeMode mode = ESerializeMode.BOTH);

	// callbacks

	//! Custom instance spawn data serialization logic if needed. Must match the data that DeserializeSpawnData will attempt to read
	event protected ESerializeResult SerializeSpawnData(notnull Managed instance, notnull SaveContext context, SerializerDefaultSpawnData defaultData) { return SerializeSpawnDataNative(instance, context, defaultData); };
	/*!
	Custom implementation for writing the script data into into the save context.
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context) { return SerializeNative(instance, context); };
	//! Callback turning spawn data (if context is notnull) into a desired target instance to return. May be given an existing instance to try and apply the data onto instead.
	event protected ref Managed DeserializeSpawnData(Managed instance, SaveContext context) { return DeserializeSpawnDataNative(instance, context); };
	/*!
	Custom implementation for deserialzing the script data from the load context
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected bool Deserialize(notnull Managed instance, notnull LoadContext context) { return DeserializeNative(instance, context); };
}

/*!
\}
*/
