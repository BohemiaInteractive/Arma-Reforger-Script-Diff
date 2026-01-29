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
	//Node A property index in connection data, it is a string
	//static const int NODE_REF_A;  //< This is commented since it is automatically define by CPP, but you can still use it of course
	//Node B property index in connection data, it is a string
	//static const int NODE_REF_B;  //< This is commented since it is automatically define by CPP, but you can still use it of course
	//Port A property index in connection data, it is a string
	//static const int PORT_A;  //< This is commented since it is automatically define by CPP, but you can still use it of course
	//Port B property index in connection data, it is a string
	//static const int PORT_B;  //< This is commented since it is automatically define by CPP, but you can still use it of course
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
