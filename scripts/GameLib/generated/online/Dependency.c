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
Dependency represent minimal structure of workshop item that other things depends on.

- WorkshopItem  - addon can depend on other addons
- WorldSaveItem - world save can depend on addons that were loaded at the time of its creation
- Room          - modded servers depends on addons that are loaded on them

It is provided with specific Revision which cannot be changed.
With Revision you can access dependencies of this dependency.

You can request load of entire WorkshopItem that is full representation of this item.

It provided by:
- Revision to reference Dependencies of WorkshopItem on specific version.
- Room as list of addons necessary for specific server.
*/
sealed class Dependency: Managed
{
	private void Dependency();
	void ~Dependency();

	//! Returns ID of this item.
	proto external string GetID();
	//! Returns Name of this item
	proto external string GetName();
	//! Returns total size of files in Bytes
	proto external float TotalFileSize();
	//! Returns Version from Revision of this item
	proto external string GetVersion();
	/*!
	Returns Revision of this item.
	*/
	proto external Revision GetRevision();
	/*!
	Returns true if it is already locally downloaded.
	Revision is also checked so if it is already downloaded with different Revision then it will return false.
	*/
	proto external bool IsOffline();
	/*!
	Will request to load WorkshopItem of this item from backend into cache.
	\param callback Is script callback where you will receive result/error when request finishes
	*/
	proto external void LoadItem(BackendCallback callback);
	/*!
	Returns cached WorkshopItem of this item or null if it was not yet cached.
	Item can be cached by calling LoadItem(BackendCallback callback).
	*/
	proto external WorkshopItem GetCachedItem();
}

/*!
\}
*/
