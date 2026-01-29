/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class Revision: Managed
{
	private void Revision();
	void ~Revision();

	static const int INVALID_MAJOR_VERSION = -1;

	//!Returns true if any local files are corrupted or missing. WorkshopItem::VerifyIntegrity() needs to be called first.
	proto external bool IsCorrupted();
	//! Returns name of Backend Environment of Workshop from which this revision originates.
	proto external string GetBackendEnv();
	//! Returns true if this specific revision is already downloaded.
	proto external bool IsDownloaded();
	//! Returns version of asset as string.
	proto external string GetVersion();
	//! Return progress of current processing (delta patching) in range of <0.0. ... 1.0>.
	proto external float GetProcessingProgress();
	//! Returns true if addon is currently processing delta patches.
	proto external bool IsProcessing();
	/*!
	Returns version of asset as array.
	\param[out] version - output for array of 3 Integers which represents numbers between dots
	*/
	proto external void GetVersionArray(out notnull array<int> version);
	//! Returns game version on which asset was uploaded as string.
	proto external string GetGameVersion();
	/*!
	Returns game version on which asset was uploaded as array.
	\param[out] version - output for array of 4 Integers which represents numbers between dots
	*/
	proto external void GetGameVersionArray(out notnull array<int> version);
	/*!
	Request list of all dependencies for this Revision.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void LoadDependencies(BackendCallback pCallback);
	/*!
	Will insert all instances of Dependency for this revision into outputDependencies.
	Dependencies are available only if ELS_DEPENDENCIES data are loaded. See GetLoadFlags() for more.
	\param[out] dependencies - output for array of dependencies
	*/
	proto external int GetDependencies(out notnull array<WorkshopItem> dependencies);
	/*!
	Request list of all scenarios for this Revision.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void LoadScenarios(BackendCallback pCallback);
	/*!
	Will insert all instances of MissionWorkshopItem for this revision into scenarios.
	Scenarios are available only if ELS_SCENARIOS data are loaded. See GetLoadFlags() for more.
	\param[out] scenarios - output for array of scenarios
	*/
	proto external int GetScenarios(out notnull array<MissionWorkshopItem> scenarios);
	/*!
	Request changelog for this Revision
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void LoadChangelog(BackendCallback pCallback);
	/*!
	Returns changelog of this Revision.
	Changelog is available only if ELS_CHANGELOG data are loaded. See GetLoadFlags() for more.
	*/
	proto external string ChangeLog();
	/*!
	Returns total size of this revision.
	Size is available only if ELS_FILES data are loaded. See GetLoadFlags() for more.
	*/
	proto external float GetTotalSize();
	// To be removed
	[Obsolete("Use GetTotalSize() instead!")]
	proto external float GetSizeBytes();
	/*!
	Will request download of manifests for this Revision to compute patch sizes.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void ComputePatchSize(BackendCallback pCallback);
	/*!
	Get size of update patch from currently downloaded revision to this revision in bytes.
	If no other Revision of same asset is downloaded then size will be total size of Revision.
	If this Revision is already downloaded then size will be 0.
	\param[out] size - size of patch in bytes
	\returns true if size was successfully calculated. false if we could not get size and result should not be used
	*/
	proto bool GetPatchSize(out float size);
	//! Get array containing names of all files in this Revision.
	proto external int GetFiles(out notnull array<string> aFiles);
	//! Compare this Revision to another one.
	proto external int CompareTo(Revision rev);
	//! Returns true if both revisions are equal.
	static proto bool AreEqual(Revision a, Revision b);
	/*!
	Describes which data are not loaded yet.
	\returns int - bitmask which should be compared with enum EPendingLoadState().
	*/
	proto external int GetLoadFlags();
	//! Returns availability status of this Revision as enum ERevisionAvailability().
	proto external ERevisionAvailability GetAvailability();
}

/*!
\}
*/
