/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIControlComponentClass: AIComponentEntityClass
{
}

class AIControlComponent: AIComponentEntity
{
	// AI
	proto external AIAgent GetControlAIAgent();
	// enabling, disabling, switching to player control
	proto external void ActivateAI();
	proto external void DeactivateAI();
	proto external bool IsAIActivated();
}

/*!
\}
*/
