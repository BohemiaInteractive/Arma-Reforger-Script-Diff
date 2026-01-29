#ifdef WORKBENCH
// temporary entity/Source solutions as container creation might come soon™ (BaseContainerTools.CreatePrefabEntityContainer)
class SCR_PrefabHelper
{
	static const string PREFAB_BASE_SUFFIX = "_base"; //<! used by SCR_PrefabManagementTool to strip it when creating a child

	protected static const string MESHOBJECT_CLASSNAME = "MeshObject";
	protected static const string DEFAULT_PARENT_PREFAB = "GenericEntity";
	protected static const string PREFAB_DOTTED_EXTENSION = ".et";

	protected static const ref array<ResourceName> PREFAB_SEARCH_DIRECTORIES = {
				"Prefabs/",				// 0
				"PrefabLibrary/",		// 1
				"PrefabsEditable/",		// 2
	};

	//------------------------------------------------------------------------------------------------
	//! Create a clone of the provided prefab at the provided destination, overriding file if any
	//! Destination directory is created if it does not exist
	//! \param[in] absoluteFilePath the directory is automatically created if needed and a trailing '.et' is automatically added if missing
	//! \return created clone's ResourceName or empty string on error
	static ResourceName ClonePrefab(ResourceName sourcePrefab, string absoluteFilePath)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(sourcePrefab))
		{
			Print("Source Prefab is empty", LogLevel.WARNING);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteFilePath))
		{
			Print("Absolute File Path is empty", LogLevel.WARNING);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteFilePath.EndsWith(PREFAB_DOTTED_EXTENSION))
			absoluteFilePath += PREFAB_DOTTED_EXTENSION;

		string absoluteDirPath = FilePath.StripFileName(absoluteFilePath);
		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!FileIO.MakeDirectory(absoluteDirPath))
			{
				Print("Cannot create the destination directory (" + absoluteDirPath + ")", LogLevel.ERROR);
				return string.Empty;
			}
		}

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(sourcePrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source cannot be created", LogLevel.ERROR);
			return string.Empty;
		}

		IEntitySource actualPrefab = IEntitySource.Cast(entitySource.GetAncestor());
		if (!actualPrefab)
		{
			Print("Created entity's source does not have an ancestor", LogLevel.ERROR);
			return string.Empty;
		}

		if (!worldEditorAPI.CreateEntityTemplate(actualPrefab, absoluteFilePath))
		{
			Print("Cannot save Prefab", LogLevel.ERROR);
			return string.Empty;
		}

		MetaFile metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file cannot be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Create a child of the provided prefab at the provided destination, overriding file if any
	//! Destination directory is created if it does not exist
	//! \param[in] absoluteFilePath the directory is automatically created if needed and a trailing '.et' is automatically added if missing
	//! \return created child's ResourceName
	static ResourceName CreateChildPrefab(ResourceName sourcePrefab, string absoluteFilePath)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(sourcePrefab))
		{
			Print("Source Prefab is empty", LogLevel.WARNING);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteFilePath))
		{
			Print("Absolute File Path is empty", LogLevel.WARNING);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteFilePath.EndsWith(PREFAB_DOTTED_EXTENSION))
			absoluteFilePath += PREFAB_DOTTED_EXTENSION;

		string absoluteDirPath = FilePath.StripFileName(absoluteFilePath);
		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!FileIO.MakeDirectory(absoluteDirPath))
			{
				Print("Cannot create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
				return string.Empty;
			}
		}

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(sourcePrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source cannot be created", LogLevel.ERROR);
			return string.Empty;
		}

		if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteFilePath))
		{
			Print("Cannot create Prefab", LogLevel.ERROR);
			return string.Empty;
		}

		MetaFile metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file cannot be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Create Prefabs from the provided XOBs - only setting MeshObject to these Prefabs.
	//! Destination directory is created if it does not exist
	//! The difference with CreatePrefabFromXOB is that prefabs here cannot be named, their name will be based on XOB filename
	//! \param[in] xobs list of XOBs to import
	//! \param[in] absoluteDirPath a trailing '/' is automatically added if missing
	//! \param[in] parentPrefab if not provided, GenericEntity is used
	//! \param[in] createBasePrefab if a prefab_base.et should be created (from which prefab.et inherits)
	//! \return created prefabs, null on error
	static array<ResourceName> CreatePrefabsFromXOBs(notnull array<ResourceName> xobs, string absoluteDirPath, ResourceName parentPrefab = string.Empty, bool createBasePrefab = false)
	{
		if (xobs.IsEmpty())
			return {};

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		if (!FileIO.FileExists(absoluteDirPath))
		{
			if (!FileIO.MakeDirectory(absoluteDirPath))
			{
				Print("Cannot create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
				return null;
			}
		}

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySourceWithoutEntity(parentPrefab);
		if (!entitySource)
		{
			Print("Prefab's entity source cannot be created", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// if the MeshObject component doesn't exist, create it
		if (!CreateEntitySourceComponentIfNeeded(entitySource, MESHOBJECT_CLASSNAME))
		{
			Print("Cannot add MeshObject component", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		array<ResourceName> result = {};
		array<ref ContainerIdPathEntry> meshObjectPath = { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) };

		foreach (ResourceName xob : xobs)
		{
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
			{
				Print("Provided XOB is empty - skipping", LogLevel.WARNING);
				continue;
			}

			string prefabFileName = FilePath.StripExtension(FilePath.StripPath(xob.GetPath())) + PREFAB_DOTTED_EXTENSION;

			// apply XOB path to mesh
			if (!worldEditorAPI.SetVariableValue(entitySource, meshObjectPath, "Object", xob))
			{
				Print("Cannot apply XOB model to " + prefabFileName, LogLevel.ERROR);
				continue;
			}

			ResourceName resourceName = SaveEntitySourceAsPrefab(entitySource, absoluteDirPath, prefabFileName, createBasePrefab);
			if (resourceName.IsEmpty())
			{
				Print("Cannot save Prefab " + prefabFileName, LogLevel.WARNING);
				continue;
			}

			result.Insert(resourceName);
		}

		EndEntityAction(manageEditAction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create Prefab from the provided XOB - only setting MeshObject to the Prefab.
	//! Destination directory is created if it does not exist
	//! The difference with CreatePrefabsFromXOBs is that providing the .et path extracts the final file name
	//! the base prefab (if requested) will be named accordingly (name without extension + '_base.et')
	//! \param[in] xob path of the XOB to import
	//! \param[in] absoluteDestinationPath can be either a directory (a trailing '/' is automatically added if missing) or a file path (.et), the directory is automatically created if needed
	//! \param[in] parentPrefab if not provided, GenericEntity is used
	//! \param[in] createBasePrefab if a prefab_base.et should be created (from which prefab.et inherits)
	//! \return created prefabs, null on error
	static ResourceName CreatePrefabFromXOB(ResourceName xob, string absoluteDestinationPath, ResourceName parentPrefab = string.Empty, bool createBasePrefab = false)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
		{
			Print("Provided XOB is empty", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteDestinationPath.EndsWith(PREFAB_DOTTED_EXTENSION)) // use the already existing method
		{
			array<ResourceName> result = CreatePrefabsFromXOBs({ xob }, absoluteDestinationPath, parentPrefab, createBasePrefab);
			if (result.IsEmpty())
				return string.Empty;
			else
				return result[0];
		}

		string absoluteDirPath = FilePath.StripFileName(absoluteDestinationPath);
		if (absoluteDirPath.EndsWith("/"))
			absoluteDirPath = absoluteDirPath.Substring(0, absoluteDirPath.Length() - 1);

		if (!FileIO.MakeDirectory(absoluteDirPath))
		{
			Print("Cannot create directory \"" + absoluteDirPath + "\"", LogLevel.ERROR);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySourceFromXOB(xob, parentPrefab);
		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);

		EndEntityAction(manageEditAction);

		if (!entitySource)
		{
			Print("Prefab's entity source cannot be created", LogLevel.ERROR);
			return string.Empty;
		}

		string prefabFileName = FilePath.StripPath(absoluteDestinationPath);
		return SaveEntitySourceAsPrefab(entitySource, absoluteDirPath, prefabFileName, createBasePrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Create the structure from the provided blueprints
	//! \param[in] structure
	//! \param[in] addonPathPrefix e.g "$ArmaReforger:"
	//! \return true on success, false on failure
	static bool CreatePrefabStructure(notnull SCR_PrefabHelper_Structure structure, string addonPathPrefix)
	{
		structure.m_sName.TrimInPlace();
		if (!structure.m_sName)
		{
			Print("Name is empty", LogLevel.ERROR);
			return false;
		}

		if (!SCR_FileIOHelper.IsValidFileName(structure.m_sName))
		{
			Print("Name is invalid - " + structure.m_sName, LogLevel.ERROR);
			return false;
		}

		if (!structure.m_Directory)
		{
			Print("Main directory is null", LogLevel.ERROR);
			return false;
		}

		string absolutePath;
		if (!Workbench.GetAbsolutePath(addonPathPrefix, absolutePath, false))
		{
			Print("Cannot get absolute path for " + addonPathPrefix + structure.m_Directory.m_sRelativePath, LogLevel.ERROR);
			return false;
		}

		return CreatePrefabStructureDirectory(structure.m_sName, structure.m_Directory, absolutePath);
	}

	//------------------------------------------------------------------------------------------------
	//! Create the structure from the provided blueprints
	//! \param[in] structureName
	//! \param[in] directory
	//! \param[in] addon addon prefix, e.g "ArmaReforger" for "$ArmaReforger:Prefabs"
	//! \return true on success, false on failure
	protected static bool CreatePrefabStructureDirectory(string structureName, notnull SCR_PrefabHelper_StructureDirectory directory, string absoluteParentPath)
	{
		string absoluteDirectoryPath = string.Format(FilePath.Concat(absoluteParentPath, directory.m_sRelativePath), structureName);
		if (!FileIO.MakeDirectory(absoluteDirectoryPath))
		{
			Print("Cannot create directory " + absoluteDirectoryPath, LogLevel.ERROR);
			return false;
		}

		foreach (SCR_PrefabHelper_StructureFile file : directory.m_aFiles)
		{
			string absoluteFilePath = string.Format(FilePath.Concat(absoluteDirectoryPath, file.m_sFileName), structureName);
			ResourceName createdResource = CreateChildPrefab(file.m_sParentResource, absoluteFilePath);
			if (!createdResource)
			{
				Print("Cannot create file " + absoluteFilePath, LogLevel.WARNING);
				continue;
			}
		}

		foreach (SCR_PrefabHelper_StructureDirectory subDirectory : directory.m_aSubDirectories)
		{
			string absoluteSubDirectoryPath = string.Format(FilePath.Concat(absoluteDirectoryPath, subDirectory.m_sRelativePath), structureName);
			if (!FileIO.MakeDirectory(absoluteSubDirectoryPath))
			{
				Print("Cannot create directory " + absoluteSubDirectoryPath, LogLevel.WARNING);
				continue;
			}

			CreatePrefabStructureDirectory(structureName, subDirectory, absoluteSubDirectoryPath); // did you mean recursion?
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create an entity and return an IEntitySource
	//! Keeps the entity in the world, does NOT delete the entity itself
	//! \param[in] parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource or null on error
	static IEntitySource CreateEntitySource(ResourceName parentPrefab = string.Empty)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(parentPrefab))
			parentPrefab = DEFAULT_PARENT_PREFAB;

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = worldEditorAPI.CreateEntity(parentPrefab, "", worldEditorAPI.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entitySource)
		{
			Print("Entity cannot be created (parent = " + parentPrefab + ")", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! A wrapper method for CreateEntitySource + DeleteEntityFromSource and World Editor action management
	//! \param[in] parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource or null on error
	protected static IEntitySource CreateEntitySourceWithoutEntity(ResourceName parentPrefab = string.Empty)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySource(parentPrefab);
		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! Create an entity from an XOB and return an IEntitySource
	//! Keeps the entity in the world, does NOT delete the entity itself
	//  protected for now
	//! \param[in] xob path of the XOB to import
	//! \param[in] parentPrefab if not provided, GenericEntity is used
	//! \return created entity's IEntitySource
	protected static IEntitySource CreateEntitySourceFromXOB(ResourceName xob, ResourceName parentPrefab = string.Empty)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(xob))
		{
			Print("Provided XOB is empty", LogLevel.ERROR);
			return null;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return null;
		}

		string xobName = FilePath.StripPath(xob.GetPath());

		bool manageEditAction = BeginEntityAction();

		IEntitySource entitySource = CreateEntitySource(parentPrefab); // do NOT switch to CreateEntitySourceWithoutEntity!
		if (!entitySource)
		{
			Print("Prefab's entity source cannot be created for xob " + xobName, LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// if the MeshObject component doesn't exist, create it
		if (!CreateEntitySourceComponentIfNeeded(entitySource, MESHOBJECT_CLASSNAME))
		{
			Print("Cannot add MeshObject component", LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		// apply XOB path to mesh
		if (!worldEditorAPI.SetVariableValue(entitySource, { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) }, "Object", xob))
		{
			Print("Cannot apply XOB model to IEntitySource for xob " + xobName, LogLevel.ERROR);
			EndEntityAction(manageEditAction);
			return null;
		}

		EndEntityAction(manageEditAction);

		return entitySource;
	}

	//------------------------------------------------------------------------------------------------
	//! Create and add a Component to the provided entitySource if not present
	//! \param[in] entitySource
	//! \param[in] componentClassname
	//! \return found or created component, null on error
	static IEntityComponentSource CreateEntitySourceComponentIfNeeded(notnull IEntitySource entitySource, string componentClassname)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(componentClassname))
		{
			Print("Provided componentClassname is empty", LogLevel.WARNING);
			return null;
		}

		IEntityComponentSource result = SCR_BaseContainerTools.FindComponentSource(entitySource, componentClassname);
		if (result)
			return result;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		bool manageEditAction = BeginEntityAction();

		result = worldEditorAPI.CreateComponent(entitySource, componentClassname);
		if (!result)
			Print("Cannot add the " + MESHOBJECT_CLASSNAME + " component to the Prefab", LogLevel.ERROR);

		EndEntityAction(manageEditAction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] addonIndex the addon index (e.g 0 = core, 1 = ArmaReforger, etc) -1 to get from all loaded addons
	//! \param[in] getMode
	//! - -1 to get all .et files (Prefabs, PrefabLibrary, PrefabsEditable)
	//! - 0 for Prefabs
	//! - 1 for PrefabLibrary
	//! - 2 for PrefabsEditable
	//! any other number reverts to -1 (get all .et files)
	//! \return a map of addonIndex -> ResourceNames
	static map<int, ref array<ResourceName>> GetPrefabs(int addonIndex = -1, int getMode = -1)
	{
		array<string> addonIDs;
		if (addonIndex < 0)
		{
			addonIDs = SCR_AddonTool.GetAllAddonIDs();
		}
		else
		{
			string addonID = SCR_AddonTool.GetAddonID(addonIndex);
			if (!addonID) // IsEmpty()
			{
				addonIndex = -1;
				addonIDs = SCR_AddonTool.GetAllAddonIDs();
			}
			else
			{
				addonIDs = { addonID };
			}
		}

		map<int, ref array<ResourceName>> result = new map<int, ref array<ResourceName>>();

		SearchResourcesFilter searchFilter = new SearchResourcesFilter();
		searchFilter.fileExtensions = { "et" };

		array<ResourceName> resourceNames;
		foreach (int index, string addonID : addonIDs)
		{
			if (addonIndex > -1)
				index = addonIndex; // little one-loop cheat

			if (PREFAB_SEARCH_DIRECTORIES.IsIndexValid(getMode))
				searchFilter.rootPath = SCR_AddonTool.GetAddonFileSystem(index) + PREFAB_SEARCH_DIRECTORIES[getMode];
			else
				searchFilter.rootPath = SCR_AddonTool.GetAddonFileSystem(index);

			resourceNames = {};
			ResourceDatabase.SearchResources(searchFilter, resourceNames.Insert);
			result.Insert(index, resourceNames);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the absolute file path of the provided resourceName
	//! \param[in] resourceName the resourceName from which to obtain the absolute file path
	//! \param[in] mustExist if true, the file MUST exist to return a valid value
	//! \return the provided resourceName's absolute file path or string.Empty on error / file not existing with mustExist true
	static string GetResourceNameAbsolutePath(ResourceName resourceName, bool mustExist = true)
	{
		string result;
		if (!Workbench.GetAbsolutePath(resourceName.GetPath(), result, mustExist))
			return string.Empty;

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Gives the target's directory the source directory's hierarchy/path
	//! Relative paths are expected, so separator must be slash (/)
	//! \code
	//! string relativeSourceDirectory = "Assets/Rocks/Granite";
	//! string relativeTargetDirectory = "Prefabs/Miniatures";
	//! string result = GetRelativeParentDirectory(relativeSourceDirectory, relativeTargetDirectory);
	//! Print(result); // Prefabs/Miniatures/Rocks/Granite
	//! \endcode
	//! See SCR_PrefabHelper.FormatRelativePath
	//! \param[in] relativeSourceDirectory the relative source directory, e.g Assets/Rocks/Granite
	//! \param[in] relativeTargetDirectory the relative target directory, e.g Prefabs/Miniatures
	//! \return target relative directory without leading or trailing slash, e.g Prefabs/Miniatures/Rocks/Granite
	static string GetRelativeParentDirectory(string relativeSourceDirectory, string relativeTargetDirectory)
	{
		if (relativeSourceDirectory.IsEmpty())
			return FormatRelativePath(relativeTargetDirectory);

		if (relativeTargetDirectory.IsEmpty())
			return FormatRelativePath(relativeSourceDirectory);

		relativeSourceDirectory = FormatRelativePath(relativeSourceDirectory);
		relativeTargetDirectory = FormatRelativePath(relativeTargetDirectory);

		// both are root directories
		if (!relativeSourceDirectory.Contains("/") && !relativeTargetDirectory.Contains("/"))
			return relativeTargetDirectory;

		bool slashFound;
		string path;
		for (int i, length = relativeSourceDirectory.Length(); i < length; i++)
		{
			string character = relativeSourceDirectory[i];
			if (!slashFound)
			{
				if (character == "/")
					slashFound = true;
			}
			else
			{
				path += character;
			}
		}

		if (!slashFound)
			path = relativeSourceDirectory;

		return relativeTargetDirectory + "/" + path;
	}

	//------------------------------------------------------------------------------------------------
	//! Format provided relative path, removing leading and trailing slashes, replacing antislashes by slashes, etc.
	//! \param[in] relativePath the relative path
	//! \return formatted relativePath
	protected static string FormatRelativePath(string relativePath)
	{
		if (relativePath.IsEmpty())
			return string.Empty;

		relativePath.Replace(SCR_StringHelper.ANTISLASH, SCR_StringHelper.SLASH);
		relativePath.Replace(SCR_StringHelper.DOUBLE_SLASH, SCR_StringHelper.SLASH);

		if (relativePath.StartsWith("/"))
			relativePath = relativePath.Substring(1, relativePath.Length() - 1);

		if (relativePath.EndsWith("/"))
			relativePath = relativePath.Substring(0, relativePath.Length() - 1);

		return relativePath;
	}

	//------------------------------------------------------------------------------------------------
	//! Update a Prefab from the provided BaseContainer (that must obviously originate from a Prefab)
	//! \param[in] actualPrefab a spawned Prefab IEntitySource's ancestor (BaseContainer)
	//! \return true in case of success, otherwise false
	static bool UpdatePrefabFromEntitySourceAncestor(notnull BaseContainer actualPrefab)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		bool manageEditAction = BeginEntityAction();
		bool result = worldEditorAPI.SaveEntityTemplate(actualPrefab); // no IEntitySource casting required
		EndEntityAction(manageEditAction);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Save a Prefab from the provided IEntitySource, with a _base Prefab or not
	//! Destination directory is created if it does not exist
	//! Keeps the entity in the world, does NOT delete the entity itself
	//! \param[in] absoluteDirPath a trailing '/' is automatically added if missing
	//! \param[in] prefabFileName a trailing '.et' is automatically added if missing
	//! \return ResourceName, empty on error
	static ResourceName SaveEntitySourceAsPrefab(notnull IEntitySource entitySource, string absoluteDirPath, string prefabFileName, bool createBasePrefab = false)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDirPath))
		{
			Print("Provided absoluteDirPath is empty", LogLevel.ERROR);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(prefabFileName))
		{
			Print("Provided prefabFileName is empty", LogLevel.ERROR);
			return string.Empty;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return string.Empty;
		}

		if (!absoluteDirPath.EndsWith("/"))
			absoluteDirPath += "/";

		if (!FileIO.FileExists(absoluteDirPath) && !FileIO.MakeDirectory(absoluteDirPath))
		{
			Print("Cannot create the destination directory (" + absoluteDirPath + ")", LogLevel.ERROR);
			return string.Empty;
		}

		if (!prefabFileName.EndsWith(PREFAB_DOTTED_EXTENSION))
			prefabFileName += PREFAB_DOTTED_EXTENSION;

		string absoluteFilePath = absoluteDirPath + prefabFileName;
		MetaFile metaFile;

		if (createBasePrefab)
		{
			string basePrefabFileName = FilePath.StripExtension(prefabFileName) + PREFAB_BASE_SUFFIX + PREFAB_DOTTED_EXTENSION;
			string absoluteBaseFilePath = absoluteDirPath + basePrefabFileName;

			// save base as Prefab
			if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteBaseFilePath))
			{
				Print("Cannot save base prefab " + basePrefabFileName + " at " + absoluteBaseFilePath, LogLevel.WARNING);
				return string.Empty;
			}

			metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteBaseFilePath);
			if (!metaFile)
			{
				Print("Created base Prefab's meta file cannot be found", LogLevel.WARNING);
				return string.Empty;
			}

			// base prefab has been created, now let's create the inheriting child prefab

			entitySource = CreateEntitySourceWithoutEntity(metaFile.GetResourceID());
			if (!entitySource)
			{
				Print("Prefab's child entity source cannot be created", LogLevel.ERROR);
				return string.Empty;
			}
		}

		// save as Prefab
		if (!worldEditorAPI.CreateEntityTemplate(entitySource, absoluteFilePath))
		{
			Print("Cannot save prefab " + prefabFileName + " at " + absoluteFilePath, LogLevel.WARNING);
			return string.Empty;
		}

		metaFile = SCR_WorldEditorToolHelper.GetResourceManager().GetMetaFile(absoluteFilePath);
		if (!metaFile)
		{
			Print("Created Prefab's meta file cannot be found", LogLevel.WARNING);
			return string.Empty;
		}

		return metaFile.GetResourceID();
	}

	//------------------------------------------------------------------------------------------------
	//! Update a Prefab
	//! \param[in] actualPrefab the spawned Prefab's IEntitySource ancestor
	//! \return true on success, false on failure
	static bool UpdatePrefab(IEntitySource actualPrefab)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("WorldEditorAPI is not available", LogLevel.ERROR);
			return false;
		}

		return worldEditorAPI.SaveEntityTemplate(actualPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Begin an Entity Action in World Editor API if required
	//! \return true if a BeginEntityAction has been called and EndEntityAction has to be called by EndEntityAction(), false otherwise
	protected static bool BeginEntityAction()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (worldEditorAPI.IsDoingEditAction())
		{
			return false;
		}
		else
		{
			worldEditorAPI.BeginEntityAction();
			return true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! End an Entity Action in World Editor API if required
	//! \param[in] manageEditAction if World Editor Entity Action should be terminated, result of an earlier BeginEntityAction call
	protected static void EndEntityAction(bool manageEditAction)
	{
		if (manageEditAction)
			SCR_WorldEditorToolHelper.GetWorldEditorAPI().EndEntityAction();
	}
}

[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleField("m_sName")]
class SCR_PrefabHelper_Structure
{
	[Attribute(defvalue: "Structure name", desc: "Config type's friendly display name")]
	string m_sName;

	[Attribute(desc: "Root directory")]
	ref SCR_PrefabHelper_StructureDirectory m_Directory;
}

[BaseContainerProps(), SCR_BaseContainerResourceTitleField("m_sRelativePath")]
class SCR_PrefabHelper_StructureDirectory
{
	[Attribute(defvalue: "Prefabs/Category/SubCategory/%1", desc: "Directory path - can be empty (root), a simple name (one directory) or multiple (sub)directories separated by a slash\n%1 = project name (e.g S105)")]
	string m_sRelativePath;

	[Attribute(desc: "Subdirectories")]
	ref array<ref SCR_PrefabHelper_StructureDirectory> m_aSubDirectories;

	[Attribute(desc: "Files")]
	ref array<ref SCR_PrefabHelper_StructureFile> m_aFiles;
}

[BaseContainerProps(), SCR_BaseContainerResourceTitleField("m_sFileName")]
class SCR_PrefabHelper_StructureFile
{
	[Attribute(defvalue: "%1_base.et", desc: "Relative file path in which this Prefab is created - extension is optional and will be defined from parent Prefab\n%1 = project name (e.g S105)")]
	string m_sFileName;

	[Attribute(defvalue: "", desc: "The parent resource from which this one inherits", uiwidget: UIWidgets.ResourceNamePicker, params: "et conf")]
	ResourceName m_sParentResource;
}
#endif // WORKBENCH
