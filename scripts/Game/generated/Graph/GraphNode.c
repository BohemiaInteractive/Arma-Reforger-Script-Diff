/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

/*!
Used to represent any kind of node inside of a graph.
Each node is represented by his ID.
Which is assigned either by the constructor or the graph AddNode method.
*/
class GraphNode: Managed
{
	static const int INVALID_NODE_ID = 4294967295;
	void GraphNode(GraphNodeId nodeId = INVALID_NODE_ID) {}

	proto external GraphNodeId GetID();
}

/*!
\}
*/
