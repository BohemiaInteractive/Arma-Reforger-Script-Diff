/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

/*!
Immutable value filter object used for dynamic catalogue filtering.
Stores a path, operator, type, and typed value.
Serializes to JSON as: { "path": "...", "operator": "...", "value": ... }
*/
sealed class BackendValueFilter
{
	private void BackendValueFilter();
	private void ~BackendValueFilter();

	//! Returns the path as integer for comparison with path enums.
	proto external int GetPath();
	//! Returns the filter operator.
	proto external EBackendValueFilterOperator GetOperator();
	//! Returns the type of value stored in this filter.
	proto external EBackendValueFilterType GetType();
	//! Returns bool value. Only valid when GetType() == BOOL.
	proto external bool GetBool();
	//! Returns int value. Only valid when GetType() == INT or ENUM.
	proto external int GetInt();
	//! Returns float value. Only valid when GetType() == FLOAT.
	proto external float GetFloat();
	//! Returns string value. Only valid when GetType() == STRING or ENUM.
	proto external string GetString();
}

/*!
\}
*/
