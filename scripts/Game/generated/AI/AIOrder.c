/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

//! Specialized message to send orders between AI agents
class AIOrder: AIMessage
{
	proto external bool IsFinished();
	proto external string GetOrderTypeString();
	proto external EAIOrderType GetOrderType();
	proto external void SetOrderType(EAIOrderType type);
}

/*!
\}
*/
