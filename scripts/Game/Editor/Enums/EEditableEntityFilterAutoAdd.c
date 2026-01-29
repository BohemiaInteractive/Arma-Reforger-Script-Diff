enum EEditableEntityFilterAutoAdd
{
	NEVER, ///< Editable entities are never inherited from parent filter
	ALWAYS, ///< Editable entities are always inherited from parent filter (provided the filter condition is met)
	ON_ACTIVATE ///< Editable entities are inherited from parent only when the filter is activated
};