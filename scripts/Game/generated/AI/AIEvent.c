/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

//! Base class for all events related to AI
class AIEvent
{
	proto external vector GetPositionn();
	proto external void SetPosition(vector pos);
	proto external AIAgent GetSender();
	proto external void SetReceiver(AIAgent pReceiver);
	proto external AIAgent GetReceiver();
	proto external IEntity GetObject();
	proto external void SetObject(IEntity obj);
}

/*!
\}
*/
