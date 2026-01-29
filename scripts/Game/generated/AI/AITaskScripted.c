/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AITaskScripted: AITask
{
	event void OnEnter(AIAgent owner);
	event void OnInit(AIAgent owner);
	event void OnAbort(AIAgent owner, Node nodeCausingAbort);
	/*!
	Scripted nodes are suposed to return SUCCESS, FAIL or RUNNING state.
	*/
	event ENodeResult EOnTaskSimulate(AIAgent owner, float dt) { return ENodeResult.SUCCESS; };
	event protected string GetNodeMiddleText();
}

/*!
\}
*/
