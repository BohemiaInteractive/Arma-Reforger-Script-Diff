/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class Node: ScriptAndConfig
{
	protected static ref TStringArray m_aEmptyVars = {};

	proto external int GetNodeID();
	// Script interface
	proto external typename GetVariableType(bool inputPort, string name);
	proto bool GetVariableIn(string name, out void val);
	proto void SetVariableOut(string name, void val);
	proto void ClearVariable(string name);
	proto void GetCallstackStr(out string val);
	proto void GetParentTreeName(out string val);

	// callbacks

	event protected TStringArray GetVariablesIn() { return m_aEmptyVars; };
	event protected TStringArray GetVariablesOut() { return m_aEmptyVars; };
	static event protected bool VisibleInPalette();
	static event protected string GetOnHoverDescription() { return "Node: Base behavior node";	};
	static event protected bool CanReturnRunning();
}

/*!
\}
*/
