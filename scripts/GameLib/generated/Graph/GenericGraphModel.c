/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

class GenericGraphModel: IGraphModel
{
	void GenericGraphModel(notnull GraphNodeScene scene, notnull BaseContainer sceneSource) {}

	//! Add node to the model, parameter nodeType should inherit from ScriptedGraphNode
	proto external GraphNode AddNode(typename nodeType, string nodeName, vector nodePosition);
}

/*!
\}
*/
