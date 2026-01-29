/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph_Ports
\{
*/

class GraphNodePortBase: Managed
{
	private void GraphNodePortBase(int portId, EGraphNodePortDirection portDirection, owned string name) {}

	proto external owned string GetName();
	//! Set label of the port, used as display name mainly
	proto external void SetLabel(string label);
	proto external owned string GetLabel();
	proto external int GetPortID();
	proto external EGraphNodePortDirection GetPortDirection();
	proto external GraphNode GetNode();
	proto external bool SetHighlighted(bool highlighted);
	proto external bool IsHighlighted();
	proto external bool SetConnectable(bool connectable);
	proto external bool IsConnectable();
	//! Returns true if this port is connected to the other port
	proto external bool IsConnectedTo(notnull GraphNodePortBase pPort);
	proto external bool HasConnection(notnull GraphNodeConnectionBase pConnection);
	//! Limit for maximum connection count. Value of -1 means "no limit" (default)
	proto external void SetMaxConnections(int count);
	proto external int GetMaxConnections();
	//! Checks max connection limit. Returns true if we can still add new connection.
	proto external bool CheckMaxConnectionsLimit();
	//! Additional, custom, per-port logic. Returns true if this port is okay to be connected with the other port.
	proto external bool CanConnectToPort(notnull GraphNodePortBase otherPort);
	//! Get local position, can be equal to scene position if there is no parent
	proto external vector GetLocalPosition();
	//! Set local position, can be equal to scene position if there is no parent
	proto external void SetLocalPosition(vector localPos);
	//! Get position considering parent position as well
	proto external vector GetPosition();
	//! Get size of the node
	proto external vector GetSize();
	proto external void SetHighlightColor(notnull Color color);
	proto external ref Color GetHighlightColor();
	proto external void SetInputDisconnectedColor(notnull Color color);
	proto external ref Color GetInputDisconnectedColor();
	proto external void SetOutputDisconnectedColor(notnull Color color);
	proto external ref Color GetOutputDisconnectedColor();
	proto external void SetInputConnectedColor(notnull Color color);
	proto external ref Color GetInputConnectedColor();
	proto external void SetOutputConnectedColor(notnull Color color);
	proto external ref Color GetOutputConnectedColor();

	// callbacks

	//! Get which connection type to instantiate when creating a connection from this port, returning null will instantiate curve connection type
	event typename GetConnectionType();
	//! Additional, custom, per-port logic. Returns true if this port is okay to be connected with the other port.
	event bool OverrideCanConnectToPort(notnull GraphNodePortBase otherPort) { return true; };
	event void Render(notnull GraphWidget graphWidget, notnull GraphNodeView view);
}

/*!
\}
*/
