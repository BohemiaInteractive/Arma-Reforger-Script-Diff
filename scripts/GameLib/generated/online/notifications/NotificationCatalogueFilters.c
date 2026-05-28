/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_notifications
\{
*/

class NotificationCatalogueFilters: Managed
{
	/*!
	Sets markedAsRead filter
	Only notifications with markedAsRead matching given value will be returned
	*/
	proto external void SetMarkedAsRead(EBackendCatalogueBoolFlag eMarkedAsRead);
	//! Returns current markedAsRead filter
	proto external EBackendCatalogueBoolFlag GetMarkedAsRead();
	/*!
	Sets createdAt greater than specific date filter
	Only notifications created later than given value will be returned
	*/
	proto external void SetCreatedAtGreaterThan(int iCreatedAt);
	//! Return current createdAt greater than specific date filter
	proto external int GetCreatedAtGreaterThan();
	/*!
	Sets ID greater than a specific value filter
	Only notifications with ID greater than given value will be returned
	*/
	proto external void SetIdGreaterThan(int iId);
	//! Returns current ID greated than a specific value filter
	proto external int GtIdGreaterThan();
	/*!
	Sets categories filter
	Only notifications of set categories are returned
	*/
	proto external void SetCategories(notnull array<string> aCategories);
	//! Returns current categories filter
	proto external int GetCategories(out notnull array<string> outCategories);
}

/*!
\}
*/
