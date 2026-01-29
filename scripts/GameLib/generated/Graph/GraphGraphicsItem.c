/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

sealed class GraphGraphicsItem: pointer
{
	//! Get position of this item, not taking into account parent positions
	proto external vector GetLocalPosition();
	//! Get scene position of this item, taking into account parent positions
	proto external vector GetPosition();
	//! Set scene position of this item, not taking into account parent positions
	proto external void SetLocalPosition(vector scenePosition);
	//! Cast GraphGraphicsItem to ScriptedGraphNode, returns null in case it is not a ScriptedGraphNode
	proto external GraphNode ToGraphNode();
	//! Cast GraphGraphicsItem to GraphNodeConnectionBase, returns null in case it is not a GraphNodeConnectionBase
	proto external GraphNodeConnectionBase ToConnection();
	//! Cast GraphGraphicsItem to GraphNodeConnectionBase, returns null in case it is not a GraphNodeConnectionBase
	proto external GraphNodePortBase ToPort();
}

/*!
\}
*/
