/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Workshop Api instance
sealed class WorkshopApi: WorkshopCatalogue
{
	private void WorkshopApi();
	private void ~WorkshopApi();

	//! ApiCode to string
	string GetCode(int code)
	{
		return typename.EnumToString(EApiCode, code);
	}

	/*!
	\brief Stops all currently running downloads and verification processes
	*/
	proto external void StopAllRunningProcesses();
	/*!
	\brief Destroy items.
	*/
	proto external void Cleanup();
	/*!
	\brief Get current world save
	*/
	proto external WorldSaveItem GetCurrentSave();
	/*!
	\brief Set current world save
	*/
	proto external void SetCurrentSave(WorldSaveItem item);
	/*!
	*/
	proto external int GetBannedItems(out notnull array<string> items);
	/*!
	*/
	proto external int GetDownloads(out notnull array<WorkshopItem> items);
	/*!
	\brief True if local *saves* data were not loaded yet -> You can load them by ScanOfflineSaves()
	*/
	proto external bool NeedSavesScan();
	/*!
	\brief True if local *addons* data were not loaded yet -> You can load them by ScanOfflineAddons()
	*/
	proto external bool NeedAddonsScan();
	/*!
	\brief Get all known tags
	*/
	proto external int GetTags(out notnull array<WorkshopTag> tags);
	/*!
	\brief Get scenarios of enabled WorkshopItems
	*/
	proto external int GetPageScenarios(out array<MissionWorkshopItem> items, int page, int pageSize);
	/*!
	\brief Find in-game MissionWorkshopItem by its MissionHeader config
	*/
	proto external MissionWorkshopItem GetInGameScenario(ResourceName sResource);
	/*!
	\brief Register new tag
	*/
	proto external WorkshopTag RegisterTag(string sName);
	/*!
	\brief Scans for both local addons and world saves
	*/
	proto external void ScanOfflineItems();
	/*!
	\brief Load local addons data
	*/
	proto external void ScanOfflineAddons();
	/*!
	\brief Load local world saves data
	*/
	proto external void ScanOfflineSaves();
	/*!
	\brief Get downloaded WorkshopItems
	*/
	static proto MissionWorkshopItem GetCurrentMission();
	/*!
	\brief Get downloaded WorkshopItems + WorldSaveItems
	*/
	proto external int GetOfflineItems(out notnull array<WorkshopItem> items);
	/*!
	\brief Find a WorkshopItem by ID in the local cache
	*/
	proto external WorkshopItem FindItem(string id);
	/*!
	\brief Triggers OnSuccess when check for banned/up-to-date downloaded addons has finished
	*/
	proto external void OnItemsChecked(BackendCallback callback);
	/*!
	\brief Scan for default scenarios
	*/
	proto external void ReadDefaultScenarios(array<string> resources);
	/*!
	\brief Get item by name
	*/
	proto external WorkshopItem GetByName(string name);
}

/*!
\}
*/
