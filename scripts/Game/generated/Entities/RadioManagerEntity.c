/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class RadioManagerEntityClass: GenericEntityClass
{
}

class RadioManagerEntity: GenericEntity
{
	/*!
	Gets all transceivers in given range that are in power ON state.
	\param[in] range Range in meters
	\param[out] tsvList Fresh list of BaseTransceivers in range
	\return Number of BaseTransceivers returned
	*/
	proto external int GetTransceiversInRange(vector position, float range, out notnull array<BaseTransceiver> tsvList);
}

/*!
\}
*/
