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
Operator for value filter comparison. Defined as bit flags so path info can OR them
into an allowed-operators mask. A single filter instance stores exactly one bit in
its m_eOperator field, and callers must pass exactly one bit to any
SetXxxValueFilter / AddXxxValueFilter — composite masks are rejected.
*/
sealed enum EBackendValueFilterOperator
{
	EQUAL,
	NOT_EQUAL,
	GREATER_THAN,
	LESS_THAN,
	GREATER_THAN_OR_EQUAL,
	LESS_THAN_OR_EQUAL,
}

/*!
\}
*/
