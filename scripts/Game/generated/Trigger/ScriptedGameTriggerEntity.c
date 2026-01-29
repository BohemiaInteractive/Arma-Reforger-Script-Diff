/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Trigger
\{
*/

class ScriptedGameTriggerEntityClass: BaseGameTriggerEntityClass
{
}

class ScriptedGameTriggerEntity: BaseGameTriggerEntity
{
	event protected event void OnInit(IEntity owner);
	event protected event void OnFrame(IEntity owner, float timeSlice);
	//!  Override this method in inherited class to define a new filter. This base filter will not be used unless overridden!
	event bool ScriptedEntityFilterForQuery(IEntity ent) { return true; };
}

/*!
\}
*/
