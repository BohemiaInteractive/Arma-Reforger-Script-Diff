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

	proto external bool CanRemoveConnection(notnull GraphNodeConnectionBase connection);
	proto external bool CanConnectPorts(notnull GraphNodePortBase portFrom, notnull GraphNodePortBase portTo);
	//! Add node to the model, parameter nodeType should inherit from ScriptedGraphNode
	proto external GraphNode AddNode(typename nodeType, string nodeName, vector nodePosition);
	//! Save node instance data back to the source, by using every attributes available
	proto external bool SaveNode(notnull GraphNode node);

	// callbacks

	event protected bool OverrideCanRemoveConnection(notnull GraphNodeConnectionBase connection) { return true; };
	event protected bool OverrideCanConnectPorts(notnull GraphNodePortBase portFrom, notnull GraphNodePortBase portTo) { return true; };
}

/*!
\}
*/
