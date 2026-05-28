/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup ScriptValue
\{
*/

/*!
Represents a named, type-flexible data holder for use in script APIs.

ScriptValue encapsulates a uniquely identified variable with a name and an extensible value.
It can be used in various systems to store and access dynamically-typed data associated
with a name and GUID.

For inherited classes, the first data member is used as data holder and its type is used as type of the specific ScriptValue.
*/
class ScriptValue: ScriptAndConfig
{
	proto external void SetName(string name);
}

/*!
\}
*/
