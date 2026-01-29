/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Containers
\{
*/

sealed class BaseContainer: BaseResourceObject
{
	proto static BaseContainer Cast(BaseContainer from);

	//! This function is for internal script usage
	static proto bool SafeCastType(out typename type, out BaseContainer to, BaseContainer from);
	proto external string GetClassName();
	proto external string GetName();
	proto external void SetName(string name);
	proto external BaseContainer GetAncestor();
	proto external void SetAncestor(ResourceName ancestor);
	proto external void ClearVariable(string varName);
	proto external void ClearVariables();
	proto external bool IsVariableSet(string varName);
	proto external bool IsVariableSetDirectly(string varName);
	proto external bool IsType(string varName, typename type);
	proto external int GetVarIndex(string varName);
	proto external string GetVarName(int varIndex);
	proto external int GetNumVars();
	proto external DataVarType GetDataVarType(int varIndex);
	//! Return empty string or value from UIWidgets values
	proto external string GetUIWidget(int varIndex);
	//! Return limits from property params, return value indicates how many values was read (0 - none, 1 - min, 2 - min,max, 3 - min,max,step)
	proto external int GetLimits(int varIndex, out float min, out float max, out float step);
	proto external void GetEnumValues(int varIndex, out array<string> names, out array<int> values);
	proto external bool Get(string varName, out void val);
	/*!
	Return object property base class name. Can be used for both DataVarType.OBJECT and DataVarType.OBJECT_ARRAY properties.
	\note Property class name specify just base class accepted by a property not actual class of assigned object in the property
	*/
	proto external string GetObjectBaseClass(int varIndex);
	proto external bool GetDefaultAsString(string varName, out string val);
	proto external BaseContainer GetObject(string varName);
	proto external bool SetObject(string varName, BaseContainer val);
	/*!
	Get wrapper for array of objects (the list is read only).
	\code
		BaseContainerList points = src.GetObjectArray("Points");

		for (int i = 0; i < points.Count(); i++)
		{
			...
		}
	\endcode
	*/
	proto external ref BaseContainerList GetObjectArray(string varName);
	/*!
	Set array of objects. Return wrapper for object array where objects can be added/removed.
	\code
		BaseContainerList points = src.SetObjectArray("Points");
		points.Insert(...);
		points.Insert(...);
		points.Insert(...);
	\endcode
	*/
	proto external ref BaseContainerList SetObjectArray(string varName);
	proto external bool Set(string varName, void val);
	//! Get list of addons where resource is defined or modified
	proto external int GetSourceAddons(out notnull array<string> addonNames);
	proto external BaseContainer GetChild(int n);
	proto external int GetNumChildren();
	proto external BaseContainer GetParent();
}

/*!
\}
*/
