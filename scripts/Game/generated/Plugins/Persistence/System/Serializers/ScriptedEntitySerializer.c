/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_System_Serializers
\{
*/

class ScriptedEntitySerializer: PersistenceSerializerBase
{
	/*
	Select when the deserialization should happen. Can be applied early to make the data available before default creation logic in OnPostInit or EOnInit
	Called once and return value is cached for all serializer instances of this type!
	*/
	static event EEntityDeserializeEvent GetDeserializeEvent()
	{
	    return EEntityDeserializeEvent.AFTER_FINALIZE;
	}
	//! In case the spawn data needs to be read early from the context. Uusally called from spawn operations on the system.
	event protected bool DeserializeSpawnData(out ResourceName prefab, notnull out EntitySpawnParams params, notnull LoadContext context) { return DeserializeSpawnDataNative(prefab, params, context); }

	//! Native spawn data serialization logic that can be called explictly if script overrides SerializeSpawnData.
	proto external protected ESerializeResult SerializeSpawnDataNative(notnull IEntity entity, notnull SaveContext context, SerializerDefaultSpawnData defaultData);
	//! Native serialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected ESerializeResult SerializeNative(notnull IEntity entity, notnull SaveContext context, ESerializeMode mode = ESerializeMode.NATIVE);
	/*!
	Native spawn data deserilization logic
	\param prefab Resource name to get filled out
	\param params Spawn params that NEED to be provided and will get changed by native data if neccessary. Usually the instance from DeserializeSpawnData should simply be passed through.
	\param context Deserialization data context to read the spawn parameters from.
	*/
	proto protected bool DeserializeSpawnDataNative(out ResourceName prefab, notnull out EntitySpawnParams params, notnull LoadContext context);
	//! Native deserialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected bool DeserializeNative(notnull IEntity entity, notnull LoadContext context, ESerializeMode mode = ESerializeMode.NATIVE);

	// callbacks

	//! Custom entity spawn data serialization logic if needed. Must match the data that DeserializeSpawnData will attempt to read
	event protected ESerializeResult SerializeSpawnData(notnull IEntity entity, notnull SaveContext context, SerializerDefaultSpawnData defaultData) { return SerializeSpawnDataNative(entity, context, defaultData); };
	/*!
	Custom implementation for writing the entity data into into the save context.
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected ESerializeResult Serialize(notnull IEntity entity, notnull SaveContext context) { return SerializeNative(entity, context); };
	/*!
	Custom implementation for deserialzing the entity data from the load context
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected bool Deserialize(notnull IEntity entity, notnull LoadContext context) { return DeserializeNative(entity, context); };
}

/*!
\}
*/
