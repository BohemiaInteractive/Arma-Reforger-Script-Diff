/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AICommunicationComponentClass: AIComponentClass
{
}

class AICommunicationComponent: AIComponent
{
	// Obsolete, don't use
	[Obsolete()]
	proto AIMessage CreateMessage(typename pClass);
	proto external bool RequestBroadcast(AIMessage pMessage, AIAgent receiver = null);
	// API for orders. Orders are added to special queue of orders. Non-order messages are in a different queue.
	proto external AIOrder GetOrder(int id);
	proto external void RemoveOrder(int id);
	proto external int GetOrderCount();
	proto external void ClearOrders();
	// API for messages
	proto external AIMessage GetMessage(int id);
	proto external void RemoveMessage(int id);
	proto external int GetMessageCount();
	proto external void ClearMessages();

	// callbacks

	//! Called when message is received and added to receive queue
	event void OnReceived(AIMessage pMessage);
	event void OnFrame(IEntity owner, float timeSlice);
	event void OnInit(IEntity owner);
}

/*!
\}
*/
