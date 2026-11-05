/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup DLC
\{
*/

/*!
Utility interface to check for resource usage and get warning events.
*/
sealed class DlcManager: EventProvider
{
	ref ScriptInvoker<EDlc> OnLicenseGrantedInvoker = new ScriptInvoker<EDlc>();

	//! Get the validator instance.
	static proto DlcManager Get();
	//! Refresh DLC license ownership information.
	proto external void RefreshLicenses();
	/*!
	Check that all of the resources currently referenced by the entity are covered by DLC ownership.
	\param player Entity of the player for whom to perform the check. Will assume local instance when passed null
	\param entity What entity to validate usage permission for.
	*/
	proto external EDlc GetRequiredDlcsForEntity(const IEntity player, const notnull IEntity entity);
	proto external bool CanUseEntity(const IEntity player, const notnull IEntity entity);
	/*!
	Check that all of the resources currently referenced by the entity are covered by DLC ownership.
	\param player Entity of the player for whom to perform the check. Will assume local instance when passed null
	\param resourceName Prefab/Asset to evaluate.
	\param forceLoad Attempt to load the resource for evaluation if not already cached.
	*/
	proto external EDlc GetRequiredDlcsForResource(const IEntity player, ResourceName resourceName, bool forceLoad = false);
	proto external bool CanUseResource(const IEntity player, ResourceName resourceName, bool forceLoad = false);
	//! Check if the current user owns the provided DLC(s).
	proto external bool HasOwnership(EDlc dlcMask);
	//! Open the current platform store to acquire a missing DLC.
	proto external void OpenStore(EDlc dlc);

	// callbacks

	//! Invoked when access to particular DLC is granted.
	event protected void OnLicenseGranted(EDlc dlc) { OnLicenseGrantedInvoker.Invoke(dlc); };
	/*!
	Called when an entity is used without required DLC ownership.
	A warning will be issued first. On second invoke the players ability to interact with the entity is disrupted.
	*/
	[EventAttribute()]
	event void OnDisallowedUsage(IEntity entity, bool warning);
}

/*!
\}
*/
