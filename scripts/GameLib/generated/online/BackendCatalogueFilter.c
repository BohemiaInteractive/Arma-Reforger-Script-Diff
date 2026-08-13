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
Base class for catalogue filters that support dynamic value filtering.
Stores an array of BackendValueFilter objects and provides common operations.
Not constructible from script directly — use derived classes instead.
*/
class BackendCatalogueFilter: Managed
{
	protected void BackendCatalogueFilter();

	//! Returns expected type for the specified path of value filter.
	proto external EBackendValueFilterType GetValueFilterType(int path);
	//! Sets a boolean value filter. Replaces any existing filters for this path. Always uses EQUAL operator.
	proto external BackendValueFilter SetBoolValueFilter(int path, bool value);
	/*!
	Sets an integer value filter. Replaces any existing filters for this path.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_LESS_THAN). Composite masks are rejected.
	*/
	proto external BackendValueFilter SetIntValueFilter(int path, EBackendValueFilterOperator op, int value);
	/*!
	Sets a float value filter. Replaces any existing filters for this path.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_LESS_THAN). Composite masks are rejected.
	*/
	proto external BackendValueFilter SetFloatValueFilter(int path, EBackendValueFilterOperator op, float value);
	/*!
	Sets a string value filter. Replaces any existing filters for this path.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_NOT_EQUAL). Composite masks are rejected.
	*/
	proto external BackendValueFilter SetStringValueFilter(int path, EBackendValueFilterOperator op, string value);
	/*!
	Sets a static-enum value filter. Replaces any existing filters for this path.
	The JSON string is resolved from the path info's enum string table (m_pEnumStrings),
	so callers only pass the integer enum value.
	\param op Single operator bit (e.g. BVFO_EQUAL). Composite masks are rejected.
	*/
	proto external BackendValueFilter SetEnumStaticValueFilter(int path, EBackendValueFilterOperator op, int value);
	/*!
	Adds an integer value filter. Replaces existing filter with same path+operator, otherwise appends.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_LESS_THAN). Composite masks are rejected.
	*/
	proto external BackendValueFilter AddIntValueFilter(int path, EBackendValueFilterOperator op, int value);
	/*!
	Adds a float value filter. Replaces existing filter with same path+operator, otherwise appends.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_LESS_THAN). Composite masks are rejected.
	*/
	proto external BackendValueFilter AddFloatValueFilter(int path, EBackendValueFilterOperator op, float value);
	/*!
	Adds a string value filter. Replaces existing filter with same path+operator, otherwise appends.
	\param op Single operator bit (e.g. BVFO_EQUAL, BVFO_NOT_EQUAL). Composite masks are rejected.
	*/
	proto external BackendValueFilter AddStringValueFilter(int path, EBackendValueFilterOperator op, string value);
	//! Removes specific value filter by its pointer. Returns true if it was present and removed.
	proto external bool RemoveValueFilter(BackendValueFilter valueFilter);
	//! Removes all value filters for the given path. Returns the number of filters removed.
	proto external int RemoveValueFiltersByPath(int path);
	//! Removes all value filters for the given path+operator. Returns the number of filters removed.
	proto external int RemoveValueFiltersByPathAndOperator(int path, EBackendValueFilterOperator op);
	//! Removes all value filters regardless of path.
	proto external void ClearValueFilters();
	//! Returns number of currently set value filters.
	proto external int GetValueFilterCount();
	//! Returns true if any value filter exists for the given path.
	proto external bool HasValueFilter(int path);
	/*!
	Returns the single value filter matching \p path and \p op, or nullptr if not present.
	At most one filter per path+operator is stored (enforced by SetXxxValueFilter / AddXxxValueFilter).
	*/
	proto external BackendValueFilter GetValueFilter(int path, EBackendValueFilterOperator op);
	//! Returns all value filters.
	proto external void GetValueFilters(out array<BackendValueFilter> outFilters);
	//! Returns all value filters for the given path.
	proto external void GetValueFiltersByPath(out array<BackendValueFilter> outFilters, int path);
}

/*!
\}
*/
