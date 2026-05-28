/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph_Connections
\{
*/

class GraphNodeConnectionBase: ScriptAndConfig
{
	/** \name Constants
	 * Populated by the engine.
	 */
	///@{

	/*!
	Node A property index in connection data, it is a string
	*/
	static const int NODE_REF_A;
	/*!
	Node B property index in connection data, it is a string
	*/
	static const int NODE_REF_B;
	/*!
	Port A property index in connection data, it is a string
	*/
	static const int PORT_A;
	/*!
	Port B property index in connection data, it is a string
	*/
	static const int PORT_B;
	///@}

	private void GraphNodeConnectionBase(notnull GraphNodePortBase pPortFrom, GraphNodePortBase pPortTo) {}

	proto external GraphNodePortBase GetPortFrom();
	proto external GraphNodePortBase GetPortTo();
	proto external void SetSelectedColor(notnull Color selectedColor);
	proto external ref Color GetSelectedColor();
	proto external void SetUnselectedColor(notnull Color selectedColor);
	proto external ref Color GetUnselectedColor();

	// callbacks

	event void Render(notnull GraphWidget graphWidget, notnull GraphNodeView view);
}

/*!
\}
*/
