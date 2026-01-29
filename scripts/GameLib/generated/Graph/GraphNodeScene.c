/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

class GraphNodeScene: ScriptAndConfig
{
	//Nodes property index in graph data, it is an array of GraphNode
	//static const int NODES;  //< This is commented since it is automatically define by CPP, but you can still use it of course
	//Connections property index in graph data, it is an array of GraphNodeConnectionBase
	//static const int CONNECTIONS; //< This is commented since it is automatically define by CPP, but you can still use it of course
	void GraphNodeScene() {}

	// --- Model
	proto external void SetModel(IGraphModel graphModel);
	proto external IGraphModel GetModel();
	//! Delete every selected items
	proto external void DeleteSelectedItems();
	//! Start drawing temporary connection from pPort, clear define if we should clear other current temporary connection
	proto external void BeginDrawConnection(notnull GraphNodePortBase pPort, bool clear = true);
	//! Returns a position snapped to the grid
	proto external vector SnapToGrid(vector scenePos);
	//! Returns graphics item at scene position
	proto external GraphGraphicsItem ItemAt(vector scenePos);
	//! Returns every selected items
	proto external int GetSelectedItems(notnull array<GraphGraphicsItem> selectedItems);
	//! Select top most item at scenePos, clear is true when we want to clear already selected items. Returns true if an item has been selected
	proto external bool SelectItemAt(vector scenePos, bool clear = true);
	//! Update current drawing connection with scenePos as parameter
	proto external void UpdateDrawConnection(vector scenePos);
	//! End current drawing connection with scenePos as parameter
	proto external void EndDrawConnection(vector scenePos);
	//! Set node position, if finalPosition is true, node position is applied to the model, otherwise node position is only moved in the scene
	proto external void SetNodePosition(notnull GraphNode node, vector newScenePos, bool finalPosition = true);
}

/*!
\}
*/
