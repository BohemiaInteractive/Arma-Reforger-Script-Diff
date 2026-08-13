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
	// returns whether AI is activated
	proto external bool ActivateAI(bool forced = true);
	proto external void DeactivateAI();
	proto external bool IsAIActivated();
}

/*!
\}
*/
