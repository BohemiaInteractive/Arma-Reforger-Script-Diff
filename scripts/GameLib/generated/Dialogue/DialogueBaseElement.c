/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

//! Base class for any element in Dialogue that can use Variables
sealed class DialogueBaseElement: ScriptAndConfig
{
	/*
	For Script to signal the use of any Variable declared in a Dialogue,
	declare an Attribute of type 'string', add it to the "Variables" category
	and set in parameters what type it expects to use.
	Currently available types: bool, int, float, vector3, string
	This string will be used to match with the name of the Variable in the Dialogue.
	Transition Rules don't have the ability to modify variable values.
	e.g.:
		[Attribute("", UIWIdgets.SearchComboBox, "MyEditorName", category: "Variables", params: "variableType=int")}
		string VARIABLE_NAME;
	*/
	private void DialogueBaseElement();
	private void ~DialogueBaseElement();

	/*
	Retrieves the value of the Dialogue variable which's name matches the provided one.
	Returns false if no such variable exists or if the 'value' output variable's type doesn't match the Dialogue variable's type.
	*/
	proto bool GetVariable(string name, out void value);
}

/*!
\}
*/
