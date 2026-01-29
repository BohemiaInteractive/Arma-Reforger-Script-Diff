[Obsolete("Use SCR_ParamEnumArray")]
class ParamEnumAddons : array<ref ParamEnum>
{
	//------------------------------------------------------------------------------------------------
	[Obsolete("Use SCR_ParamEnumArray.FromAddons instead")]
	private static ParamEnumArray FromEnum(int titleFormat = 2, int hideCoreModules = 0)
	{
		return SCR_ParamEnumArray.FromAddons(titleFormat, hideCoreModules);
	}
}

// TODO: rename to SCR_AddonTools?
class SCR_AddonTool
{
	protected static const ref array<string> LOADED_ADDON_IDS = {}; // static values get reset every time addons are loaded/unloaded
	protected static const ref array<string> LOADED_ADDON_GUIDS = {};

	protected static const ref array<string> CORE_ADDONS = { "core", "ArmaReforger" };

	//------------------------------------------------------------------------------------------------
	//! Returns an array of addons where given resource is present or modified.
	//! \param[in] prefab Prefab path
	//! \param[in] ignoreCoreAddons if true then ArmaReforger and Core are ignored unless no other addons are found
	//! \return array of addon ID strings
	static array<string> GetResourceAddons(ResourceName prefab, bool ignoreCoreAddons = false)
	{
		array<string> addonNames = {};

		if (prefab.IsEmpty())
			return addonNames;

		Resource prefabResource = BaseContainerTools.LoadContainer(prefab);
		if (!prefabResource)
			return addonNames;

		BaseResourceObject configContainer;
		configContainer = prefabResource.GetResource();
		if (!configContainer)
			return addonNames;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return addonNames;

		configBase.GetSourceAddons(addonNames);

		if (ignoreCoreAddons && !addonNames.IsEmpty())
		{
			foreach (string item : CORE_ADDONS)
			{
				addonNames.RemoveItem(item);
			}
		}

		return addonNames;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns last addon where given resource is present.
	//! \param[in] prefab Prefab path
	//! \return class name
	static string GetResourceLastAddon(ResourceName prefab)
	{
		array<string> addonNames = GetResourceAddons(prefab);
		if (addonNames.IsEmpty())
			return string.Empty;

		return addonNames[addonNames.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Get addon name by providing index of the addon.
	//! Can be used in tandem with ParamEnumAddons.
	//! \param[in] index Index number
	//! \return Addon ID
	static string GetAddonID(int index)
	{
		LoadAddons();

		if (!LOADED_ADDON_IDS.IsIndexValid(index))
			return string.Empty;

		return LOADED_ADDON_IDS[index];
	}

	//------------------------------------------------------------------------------------------------
	// \param[in] ignoreCoreAddons
	//! \return addon IDs (format "core", "ArmaReforger" etc)
	static array<string> GetAllAddonIDs(/* bool ignoreCoreAddons = false */)
	{
		LoadAddons();

		return SCR_ArrayHelperT<string>.GetCopy(LOADED_ADDON_IDS);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] addonIndex
	//! \return addon file system, e.g "$core:", "$ArmaReforger:" etc
	static string GetAddonFileSystem(int addonIndex)
	{
		LoadAddons();

		if (!LOADED_ADDON_IDS.IsIndexValid(addonIndex))
			return string.Empty;

		return ToFileSystem(LOADED_ADDON_IDS[addonIndex]);
	}

	//------------------------------------------------------------------------------------------------
	// \param[in] ignoreCoreAddons
	//! \return addon IDs (format "$core:", "$ArmaReforger:" etc)
	static array<string> GetAllAddonFileSystems(/* bool ignoreCoreAddons = false */)
	{
		LoadAddons();

		array<string> result = {};

		foreach (string addonID : LOADED_ADDON_IDS)
		{
			result.Insert(ToFileSystem(addonID));
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] addonIndex
	//! \return
	static bool IsVanillaAddon(int addonIndex)
	{
		if (!LOADED_ADDON_GUIDS.IsIndexValid(addonIndex))
			return false;

		return GameProject.IsVanillaAddon(LOADED_ADDON_GUIDS[addonIndex]);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] addonID
	//! \return
	static bool IsVanillaAddon(string addonID)
	{
		int addonIndex = LOADED_ADDON_IDS.Find(addonID);
		if (addonIndex < 0)
			return false;

		return GameProject.IsVanillaAddon(LOADED_ADDON_GUIDS[addonIndex]);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the FileSystem prefix-stripped path
	//! e.g from $ArmaReforger:scripts/Game/Global/SCR_AddonTools.c to scripts/Game/Global/SCR_AddonTools.c
	//! $abc:test -> test
	//! $:test -> test
	//! :test -> :test
	//! $abc -> $abc
	//! $abc: -> <empty string>
	//! \param[in] fileSystemPath
	//! \return path without addon prefix, or untouched provided path if the format is wrong (proper format = $addonName:somethingElse)
	static string StripFileSystem(string fileSystemPath)
	{
		int length = fileSystemPath.Length();
		if (fileSystemPath.IsEmpty())
			return fileSystemPath;

		if (!fileSystemPath.StartsWith("$"))
			return fileSystemPath;

		int colonIndex /* ha, ha */ = fileSystemPath.IndexOf(":");
		if (colonIndex < 0)
			return fileSystemPath;

		++colonIndex;
		if (colonIndex == length)
			return string.Empty;

		return fileSystemPath.Substring(colonIndex, length - colonIndex);
	}

	//------------------------------------------------------------------------------------------------
	//!  Convert addon name to file system format.
	//! For instance, "ArmaReforger" will get converted to "$ArmaReforger:".
	//! \param[in] addonID
	//! \return addon name or empty string on wrong input
	static string ToFileSystem(string addonID)
	{
		addonID.TrimInPlace();
		if (!addonID) // !.IsEmpty()
			return string.Empty;

		return string.Format("$%1:", addonID);
	}

	//------------------------------------------------------------------------------------------------
	protected static void LoadAddons()
	{
		if (!LOADED_ADDON_IDS.IsEmpty())
			return;

		array<string> guids = {};
		GameProject.GetLoadedAddons(guids);

		LOADED_ADDON_GUIDS.Copy(guids);
		foreach (string addonGUID : LOADED_ADDON_GUIDS)
		{
			LOADED_ADDON_IDS.Insert(GameProject.GetAddonID(addonGUID));
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Return the absolute path (without a trailing '/')
	//! \param[in] addonId
	//! \param[in] relativeDirPath if a directory resourceName is provided, use resourceName.GetPath()
	//! \param[out] result
	//! \param[in] mustExist
	//! \return true on success, false on failure
	static bool GetAddonAbsolutePath(int addonId, string relativeDirPath, out string result, bool mustExist = true)
	{
		return Workbench.GetAbsolutePath(ToFileSystem(GetAddonID(addonId)) + relativeDirPath, result, mustExist);
	}

	//------------------------------------------------------------------------------------------------
	//! Return the absolute path (without a trailing '/')
	//! \param[in] addonId
	//! \param[in] directory a directory resourceName should be provided\
	//! if a -file- ResourceName is provided, the xxx.yyy part WILL be part of the provided directory so use FilePath.StripFileName if not wanted
	//! \param[out] result
	//! \param[in] mustExist
	//! \return true on success, false on failure
	static bool GetAddonAbsolutePath(int addonId, ResourceName directory, out string result, bool mustExist = true)
	{
		return Workbench.GetAbsolutePath(ToFileSystem(GetAddonID(addonId)) + directory.GetPath(), result, mustExist);
	}
#endif
}
