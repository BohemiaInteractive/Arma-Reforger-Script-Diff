/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Serializers
\{
*/

sealed class PersistenceSerializerBase: ScriptAndConfig
{
	//! Delcare the type this serializer can handle so it can automatically find matching data instances. MUST be implemented!
	static event typename GetTargetType();
	//! Optinally override this if a failure in deserialization logic requires the entity/scripted state to be deleted as it would not be useable at all.
	static event EDeserializeFailHandling GetDeserializeFailHandling()
	{
	    return EDeserializeFailHandling.IGNORE;
	}
	//! Only constructed through the persistence system
	private void PersistenceSerializerBase();
	private void ~PersistenceSerializerBase();

	//! Get persistence system the serializer is linked to
	proto external PersistenceSystem GetSystem();

	// callbacks

	//! Do any inital configuration of the serializer once it is linked to the system
	event protected void Setup();
}

/*!
\}
*/
