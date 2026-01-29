/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Serializers
\{
*/

class ScriptedComponentSerializer: PersistenceSerializerBase
{
	/*
	Select when the deserialization should happen. Can be applied early to make the data available before default creation logic in OnPostInit or EOnInit
	Called once and return value is cached for all serializer instances of this type!
	*/
	static event EComponentDeserializeEvent GetDeserializeEvent()
	{
	    return EComponentDeserializeEvent.AFTER_ENTITY_FINALIZE;
	}

	//! Native serialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected ESerializeResult SerializeNative(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context, ENativeSerializerMode mode = ENativeSerializerMode.NATIVE);
	//! Native deserialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected bool DeserializeNative(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context, ENativeSerializerMode mode = ENativeSerializerMode.NATIVE);

	// callbacks

	/*!
	Custom implementation for writing the component data into into the save context.
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context) { return SerializeNative(owner, component, context); };
	/*!
	Custom implementation for deserialzing the component data from the load context
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context) { return DeserializeNative(owner, component, context); };
}

/*!
\}
*/
