/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

class GraphNode: ScriptAndConfig
{
	/** \name Constants
	 * Populated by the engine.
	 */
	///@{

	/*!
	Name property index in node data, it is a string
	*/
	static const int NAME;
	///@}

	//Used to describe ports of a nodes that are not static but dynamic
	static void GetPortsInfo(BaseContainer graphSrc, BaseContainer nodeSrc, notnull array<ref GraphPortInfo> inputPorts, notnull array<ref GraphPortInfo> outputPorts);
	private void GraphNode() {}

	proto external string GetName();
	//! Access node port by 'id'.
	proto external GraphNodePortBase GetPort(int id, EGraphNodePortDirection direction);
	proto external GraphNodePortBase FindPort(string portName, EGraphNodePortDirection direction);
	//! Return number of port connections.
	proto external int GetPortConnectionsCount(int id, EGraphNodePortDirection direction);
	//! Return total connections count.
	proto external int GetTotalConnectionsCount();
	//! Get local position, can be equal to scene position if there is no parent
	proto external vector GetLocalPosition();
	//! Set local position, can be equal to scene position if there is no parent
	proto external void SetLocalPosition(vector localPos);
	//! Get position considering parent position as well
	proto external vector GetPosition();
	//! Get size of the node
	proto external vector GetSize();
	proto external void SetHeaderColor(notnull Color headerColor);
	proto external ref Color GetHeaderColor();
	proto external void SetBackgroundColor(notnull Color backgroundColor);
	proto external ref Color GetBackgroundColor();
	proto external void SetSelectedColor(notnull Color selectedColor);
	proto external ref Color GetSelectedColor();
	proto external void SetUnselectedColor(notnull Color selectedColor);
	proto external ref Color GetUnselectedColor();

	// callbacks

	//! Get which port type to instantiate when creating a connection from this node, returning null will instantiate GraphNodePortCircle port type
	event typename GetPortType(int portId, EGraphNodePortDirection portDirection, owned string name);
	event void Render(notnull GraphWidget graphWidget, notnull GraphNodeView view);
}

/*!
\}
*/
