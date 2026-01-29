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
It is used to represent any kind of graph.
Internally this graph is represented as an adjacency list.
*/
class Graph: Managed
{
	//! Check if the node exist in the graph
	proto external bool HasNode(GraphNodeId nodeId);
	//! Returns the node if the node id is valid
	proto external GraphNode GetNode(GraphNodeId nodeId);
	//! Add a node. Returns INVALID_NODE_ID if the node is already inside the graph.
	proto external GraphNodeId AddNode(GraphNode graphNode);
	/*!
	Returns true if the node has been removed
	*IMPORTANT* Use carefully, because we need to go over the entire graph to remove the node from the adjacency lists.
	*/
	proto external bool RemoveNode(GraphNodeId nodeId);
	/*!
	Returns true if all nodes has been removed
	*IMPORTANT* Use carefully, because we need to go over the entire graph to remove the node from the adjacency lists.
	This is still faster than calling RemoveNode individually. Because we don't search in the adjacency lists of nodes that will be removed.
	*/
	proto external bool RemoveNodes(notnull array<GraphNodeId> nodeIds);
	//! Insert an edge from node1 to node2
	proto external bool AddEdge(GraphNodeId node1, GraphEdge edge);
	//! Get the edge from node1 to node2
	proto external GraphEdge GetEdge(GraphNodeId node1, GraphNodeId node2);
	//! Get the edges from node
	proto external int GetEdges(GraphNodeId node, out notnull array<ref GraphEdge> edges);
	//! Remove the edge from node1 to node2
	proto external bool RemoveEdge(GraphNodeId node1, GraphNodeId node2);
	//! Check if this edge exist in the graph
	proto external bool HasEdge(GraphNodeId node1, GraphNodeId node2);
	//! Returns the number of node added to the array
	proto external int GetAdjacentNodes(GraphNodeId node, out notnull array<GraphNodeId> adjacentNodes);
	//! Go over the graph in a BFS way and calling the callback
	proto void BFS(GraphNodeId startNode, func callback);
	//! Go over the graph in a DFS way and calling the callback
	proto void DFS(GraphNodeId startNode, func callback);

	// callbacks

	event bool CanAddNode(GraphNode graphNode) { return true; };
	event void OnNodeAdded(GraphNode graphNode);
	event void OnNodeRemoved(GraphNode graphNode);
	event bool CanAddEdge(GraphNode node1, GraphEdge edge) { return true; };
	event void OnEdgeAdded(GraphNode node1, GraphEdge edge);
	event void OnEdgeRemoved(GraphNode node1, GraphEdge edge);
}

/*!
\}
*/
