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
	/*!
	Select when the deserialization should happen:
	BEFORE_CONSTRUCTOR: Instance was created but the constructor not yet called. So apply logic can not rely on something the constructor manually initalized.s
	REPLACE_CONSTRUCTOR: The deserialization logic replaces any constructor defined. Allows setting all relevant values without wasting time setting up defaults that will be overridden anyway.
	AFTER_CONSTRUCTOR: Default "normal" behavior as if custom scripts were manipulating the instance after the "new" call.
	*/
	static event EScriptedStateDeserializeEvent GetDeserializeEvent()
	{
	    return EScriptedStateDeserializeEvent.AFTER_CONSTRUCTOR;
	}

	//! Native serialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected ESerializeResult SerializeNative(notnull Managed instance, notnull SaveContext context, ESerializeMode mode = ESerializeMode.BOTH);
	//! Native deserialization logic used by the game. Can be called by scripts before or after custom data or not all.
	proto external protected bool DeserializeNative(notnull Managed instance, notnull LoadContext context, ESerializeMode mode = ESerializeMode.BOTH);

	// callbacks

	/*!
	Custom implementation for writing the script data into into the save context.
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected ESerializeResult Serialize(notnull Managed instance, notnull SaveContext context) { return SerializeNative(instance, context); };
	/*!
	Custom implementation for deserialzing the script data from the load context
	\return True for success. False for data/state errors which will abort the process and use configured fault handling
	*/
	event protected bool Deserialize(notnull Managed instance, notnull LoadContext context) { return DeserializeNative(instance, context); };
}

/*!
\}
*/
