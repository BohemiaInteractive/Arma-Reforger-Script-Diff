/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI_SmartActions
\{
*/

class AISmartActionComponent: ScriptComponent
{
	GenericEntity m_Owner;
	void AISmartActionComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = GenericEntity.Cast(ent);
	}

	proto external AIAgent GetUser();
	proto external bool IsActionAccessible();
	proto external void SetActionAccessible(bool accessible);
	proto external int GetTags(out notnull array<string> outTags);
	proto external void SetActionOffset(vector offset);
	proto external vector GetActionOffset();

	// callbacks

	event override void EOnInit(IEntity owner);
	event void OnActionBegin(IEntity owner);
	event void OnActionEnd(IEntity owner);
	event void OnActionFailed(IEntity owner);
}

/*!
\}
*/
