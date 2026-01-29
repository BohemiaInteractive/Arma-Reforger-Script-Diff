/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

//! Event which gets broadcasted from danger-causing places to AI
class AIDangerEvent: AIMessage
{
	proto external IEntity GetVictim();
	proto external EAIDangerEventType GetDangerType();
	proto external void SetVictim(IEntity victim);
	proto external void SetDangerType(EAIDangerEventType type);
}

/*!
\}
*/
