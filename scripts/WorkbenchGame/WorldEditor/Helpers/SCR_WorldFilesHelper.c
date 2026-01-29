#ifdef WORKBENCH
class SCR_WorldFilesHelper
{
	// protected static const string GENERIC_WORLD_ENTITY_CLASS = ((typename)GenericWorldEntity).ToString();
	protected static const string GENERIC_WORLD_ENTITY_CLASS = "GenericWorldEntity";
	protected static const string LAYERS_DIR = "%1_Layers"; // %1 being world name, without extension
	protected static const string DEFAULT_LAYER = "default.layer";

	protected static const ref array<string> COPY_SUB_DIRECTORIES = { ".Rivers", ".Shore", "SurfaceMasks", "Terrain", "%1", LAYERS_DIR };

	protected static const ref array<string> COPY_DIR_FILES =			{ "%1.ent",	"%1.smap",	"%1.smd",	"%1.topo",	"%1_SMD.conf" };
	protected static const ref array<bool> COPY_DIR_FILES_REGISTER =	{ 1,		1,			0,			1,			1 };

	//------------------------------------------------------------------------------------------------
	//! Create a world and registers it through Resource Manager
	//! \param[in] relativeWorldDirectory addon-prefixed path
	//! \param[in] worldName created files will be based on this name
	//! \param[in] defaultLayerEntities entities to be created in the default layer - if no GenericWorldEntity Prefab is provided, a default one will be created
	//! \param[in] overwriteWorldFile if set to true, deletes and rewrites world file if it exists
	//! \param[in] overwriteDefaultLayerFile if set to true, deletes and rewrites default layer file if it exists
	//! \return addon-prefixed world .ent RELATIVE file path on success, empty string on failure
	static string CreateWorld(string relativeWorldDirectory, string worldName, array<ResourceName> defaultLayerEntities = null, bool overwriteWorldFile = false, bool overwriteDefaultLayerFile = false)
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(relativeWorldDirectory))
		{
			Print("Empty world directory provided", LogLevel.WARNING);
			return string.Empty;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(worldName))
		{
			Print("Empty world name provided", LogLevel.WARNING);
			return string.Empty;
		}

		string absoluteWorldDirectory;
		if (!Workbench.GetAbsolutePath(relativeWorldDirectory, absoluteWorldDirectory, false))
		{
			Print("Wrong relative world directory provided: " + relativeWorldDirectory, LogLevel.WARNING);
			return string.Empty;
		}

		string worldFilePath = FilePath.Concat(absoluteWorldDirectory, worldName + ".ent");
		string layersDirectory = FilePath.Concat(absoluteWorldDirectory, worldName + "_Layers");
		string defaultLayerFilePath = FilePath.Concat(layersDirectory, "default.layer");

		if (!FileIO.FileExists(absoluteWorldDirectory))
			FileIO.MakeDirectory(absoluteWorldDirectory);

		// create world and layers directory
		if (!FileIO.FileExists(layersDirectory))
		{
			if (!FileIO.MakeDirectory(layersDirectory))
			{
				Print("Cannot create " + layersDirectory, LogLevel.WARNING);
				return string.Empty;
			}
		}

		// create .ent file
		if (overwriteWorldFile || !FileIO.FileExists(worldFilePath))
		{
			FileHandle fileHandle = FileIO.OpenFile(worldFilePath, FileMode.WRITE);
			if (!fileHandle)
			{
				Print("Cannot write world file " + worldFilePath, LogLevel.WARNING);
				return string.Empty;
			}

			fileHandle.WriteLine("Layer default {");
			fileHandle.WriteLine(" Index 0");
			fileHandle.WriteLine("}");
			fileHandle.Close();
		}
		else
		{
			Print("File already exists, skipping " + worldFilePath, LogLevel.WARNING);
		}

		// create default.layer file
		if (overwriteDefaultLayerFile || !FileIO.FileExists(defaultLayerFilePath))
		{
			FileHandle fileHandle = FileIO.OpenFile(defaultLayerFilePath, FileMode.WRITE);
			if (!fileHandle)
			{
				Print("Cannot write default layer file " + defaultLayerFilePath, LogLevel.WARNING);
				return string.Empty;
			}

			ResourceName foundWorldEntity;
			if (defaultLayerEntities)
			{
				foreach (ResourceName resourceName : defaultLayerEntities)
				{
					if (SCR_BaseContainerTools.GetContainerClassName(resourceName) == GENERIC_WORLD_ENTITY_CLASS)
					{
						foundWorldEntity = resourceName;
						break;
					}
				}
			}

			// done this way to write it first
			if (foundWorldEntity.IsEmpty())
			{
				fileHandle.WriteLine(GENERIC_WORLD_ENTITY_CLASS + " world {");
				fileHandle.WriteLine("}");
			}
			else
			{
				fileHandle.WriteLine(GENERIC_WORLD_ENTITY_CLASS + " world : \"" + foundWorldEntity + "\" {");
				fileHandle.WriteLine("}");
			}

			if (defaultLayerEntities)
			{
				foreach (ResourceName resourceName : defaultLayerEntities)
				{
					if (resourceName == foundWorldEntity)
						continue;

					fileHandle.WriteLine(string.Format("%1 : \"%2\" {", SCR_BaseContainerTools.GetContainerClassName(resourceName), resourceName));
					fileHandle.WriteLine("}");
				}
			}

			fileHandle.Close();
		}
		else
		{
			Print("File already exists, skipping " + defaultLayerFilePath, LogLevel.WARNING);
		}

		return relativeWorldDirectory + "/" + worldName + ".ent";
	}

	//------------------------------------------------------------------------------------------------
	//! Create a copy of the provided world's directory
	//! \param[in] absoluteSourceFilePath absolute source world ent file path
	//! \param[in] absoluteDestinationFilePath absolute destination world ent file path
	//! \return true on success, false otherwise
	static bool DuplicateWorld(string absoluteSourceFilePath, string absoluteDestinationFilePath)
	{
		PrintFormat("[SCR_WorldFilesHelper.DuplicateWorld] STARTED", level: LogLevel.NORMAL);
		PrintFormat("Duplicating %1 to %2", absoluteSourceFilePath, absoluteDestinationFilePath, level: LogLevel.NORMAL);

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteSourceFilePath) || !absoluteSourceFilePath.EndsWith(".ent"))
		{
			Print("Provided source world file is empty", LogLevel.WARNING);
			return false;
		}

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(absoluteDestinationFilePath) || !absoluteDestinationFilePath.EndsWith(".ent"))
		{
			Print("Provided destination world file is empty", LogLevel.WARNING);
			return false;
		}

		if (!FileIO.FileExists(absoluteSourceFilePath))
		{
			Print("Provided source world file does not exist", LogLevel.WARNING);
			return false;
		}

		if (FileIO.FileExists(absoluteDestinationFilePath))
		{
			Print("Provided destination world file exists", LogLevel.WARNING);
			return false;
		}

		string absoluteSourceDirPath = FilePath.StripFileName(absoluteSourceFilePath);
		string absoluteDestinationDirPath = FilePath.StripFileName(absoluteDestinationFilePath);

		if (!FileIO.FileExists(FilePath.Concat(absoluteSourceDirPath, "Terrain")))
		{
			Print("The 'Terrain' subdirectory does not exist in the source world's directory", LogLevel.WARNING);
			return false;
		}

		string sourceWorldName = FilePath.StripExtension(FilePath.StripPath(absoluteSourceFilePath));
		string destinationWorldName = FilePath.StripExtension(FilePath.StripPath(absoluteDestinationFilePath));

		foreach (int fileIndex, string dirFile : COPY_DIR_FILES)
		{
			string sourceDirFile = FilePath.Concat(absoluteSourceDirPath, string.Format(dirFile, sourceWorldName));
			if (!FileIO.FileExists(sourceDirFile))
			{
				Print(sourceDirFile + " not found, skipping", LogLevel.NORMAL);
				continue;
			}

			string destinationDirFile = FilePath.Concat(absoluteDestinationDirPath, string.Format(dirFile, destinationWorldName));
			// if (!FileIO.CopyFile(sourceDirFile, destinationDirFile))
			if (!SCR_FileIOHelper.CopyFile(sourceDirFile, destinationDirFile))
			{
				PrintFormat("Cannot copy %1 to %2", sourceDirFile, destinationDirFile, level: LogLevel.ERROR);
				continue;
			}

			PrintFormat("Copied %1 to %2", string.Format(dirFile, sourceWorldName), string.Format(dirFile, destinationWorldName), level: LogLevel.NORMAL);

			if (COPY_DIR_FILES_REGISTER[fileIndex]) // do I register?
			{
				Print("Re-registering " + destinationDirFile, LogLevel.NORMAL);
				resourceManager.RegisterResourceFile(destinationDirFile, false);
			}
		}

		foreach (string subDirectory : COPY_SUB_DIRECTORIES)
		{
			string sourceSubDir = FilePath.Concat(absoluteSourceDirPath, string.Format(subDirectory, sourceWorldName));
			if (!FileIO.FileExists(sourceSubDir))
			{
				Print(sourceSubDir + " not found, skipping", LogLevel.NORMAL);
				continue;
			}

			string destinationSubDir = FilePath.Concat(absoluteDestinationDirPath, string.Format(subDirectory, destinationWorldName));
			if (!SCR_FileIOHelper.CopyDirectory(sourceSubDir, destinationSubDir))
			{
				PrintFormat("Cannot copy %1 to %2" + subDirectory, sourceWorldName, LogLevel.ERROR);
				continue;
			}

			PrintFormat("Copied %1 to %2", string.Format(subDirectory, sourceWorldName), string.Format(subDirectory, destinationWorldName), level: LogLevel.NORMAL);

			ReRegisterMetaFiles(resourceManager, destinationSubDir);
		}


		PrintFormat("[SCR_WorldFilesHelper.DuplicateWorld] FINISHED", level: LogLevel.NORMAL);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void ReRegisterMetaFiles(notnull ResourceManager resourceManager, string dir)
	{
		array<string> filesToRegister = {};
		array<ref SCR_FileInfo> metaFiles = SCR_FileIOHelper.GetDirectoryContent(dir, "meta");
		foreach (SCR_FileInfo fileInfo : metaFiles)
		{
			if ((fileInfo.m_eAttributes | FileAttribute.DIRECTORY) == FileAttribute.DIRECTORY)
				continue;

			Print("Deleting " + fileInfo.m_sFilePath, LogLevel.NORMAL);
			if (!FileIO.DeleteFile(fileInfo.m_sFilePath))
			{
				Print("Cannot delete " + fileInfo.m_sFilePath, LogLevel.ERROR);
				continue;
			}

			string fileFromMeta = FilePath.StripExtension(fileInfo.m_sFilePath);
			if (FileIO.FileExists(fileFromMeta))
				filesToRegister.Insert(fileFromMeta);
		}

		foreach (string fileToRegister : filesToRegister)
		{
			Print("Re-registering " + fileToRegister, LogLevel.NORMAL);
			resourceManager.RegisterResourceFile(fileToRegister, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Replaces the current WorldEntity (GenericWorldEntity or Prefab) by the provided Prefab
	//! \param[in] relativeWorldFilePath addon-prefixed .ent file path in which to replace the GenericWorldEntity
	//! \param[in] newWorldEntityPrefab the new WorldEntity Prefab; if empty, default GenericWorldEntity is be used
	//! \return true on success, false otherwise
	static bool ReplaceWorldEntity(string relativeWorldFilePath, ResourceName newWorldEntityPrefab)
	{
		if (!relativeWorldFilePath)
		{
			Print("Provided world file is empty", LogLevel.WARNING);
			return false;
		}

		if (!newWorldEntityPrefab)
			newWorldEntityPrefab = GENERIC_WORLD_ENTITY_CLASS;

		string absoluteWorldFilePath;
		if (!Workbench.GetAbsolutePath(relativeWorldFilePath, absoluteWorldFilePath, true))
		{
			Print("Provided world file does not exist: " + relativeWorldFilePath, LogLevel.WARNING);
			return false;
		}

		string defaultLayerAbsFilePath = GetDefaultLayerAbsoluteFilePath(relativeWorldFilePath);
		if (!defaultLayerAbsFilePath || !FileIO.FileExists(defaultLayerAbsFilePath))
		{
			Print("Cannot find default layer's location: \"" + defaultLayerAbsFilePath + "\"", LogLevel.WARNING);
			return false;
		}

		Print("Reading " + defaultLayerAbsFilePath, LogLevel.NORMAL);
		array<string> lines = SCR_FileIOHelper.ReadFileContent(defaultLayerAbsFilePath);

		bool found;
		for (int i, count = lines.Count(); i < count; i++)
		{
			if (lines[i].EndsWith(" {") && lines[i].StartsWith(GENERIC_WORLD_ENTITY_CLASS + " "))
			{
				lines[i] = GENERIC_WORLD_ENTITY_CLASS + " world : \"" + newWorldEntityPrefab + "\" {"; // }
				found = true;
				break;
			}
		}

		if (!found)
		{
			PrintFormat("No %1 class can be found in %2", GENERIC_WORLD_ENTITY_CLASS, defaultLayerAbsFilePath, level: LogLevel.WARNING);
			return false;
		}

		bool result = SCR_FileIOHelper.WriteFileContent(defaultLayerAbsFilePath, lines);
		if (result)
			Print("File written successfully: " + defaultLayerAbsFilePath, LogLevel.NORMAL);
		else
			Print("File cannot be written: " + defaultLayerAbsFilePath, LogLevel.WARNING);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the absolute path to the default layer (worldDir/worldName_Layers/default.layer)
	//! \param[in] relativeWorldFilePath addon-prefixed .ent file path in which to replace the GenericWorldEntity
	//! \return path to the default layer or empty string on error
	protected static string GetDefaultLayerAbsoluteFilePath(string relativeWorldFilePath)
	{
		string defaultLayerAbsFilePath;
		if (!Workbench.GetAbsolutePath(relativeWorldFilePath, defaultLayerAbsFilePath))
		{
			Print("Cannot get absolute path for " + relativeWorldFilePath, LogLevel.WARNING);
			return string.Empty;
		}

		string layerAbsDir = FilePath.StripFileName(defaultLayerAbsFilePath);
		if (!layerAbsDir.EndsWith("/"))
			layerAbsDir += "/";

		string worldName = FilePath.StripExtension(FilePath.StripPath(relativeWorldFilePath));

		return layerAbsDir + string.Format(LAYERS_DIR, worldName) + "/" + DEFAULT_LAYER;
	}
}
#endif // WORKBENCH
