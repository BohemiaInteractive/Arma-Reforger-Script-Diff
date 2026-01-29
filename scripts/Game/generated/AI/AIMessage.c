/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

//! Base class for all messages related to AI
class AIMessage: ScriptAndConfig
{
	proto external void SetFlag(int fl);
	proto external void ClearFlag(int fl);
	proto external bool GetFlag(int fl);
	proto external void SetText(string pText);
	proto external string GetText();
	proto external vector GetPosition();
	proto external void SetPosition(vector pos);
	//! sender is filled when broadcasting
	proto external AIAgent GetSender();
	proto external void SetReceiver(AIAgent pReceiver);
	proto external AIAgent GetReceiver();
	proto external IEntity GetObject();
	proto external void SetObject(IEntity obj);

	// callbacks

	event void OnBroadcast(AIAgent pOwner);
	event void OnReceived(AIAgent pReceiver);
}

/*!
\}
*/
