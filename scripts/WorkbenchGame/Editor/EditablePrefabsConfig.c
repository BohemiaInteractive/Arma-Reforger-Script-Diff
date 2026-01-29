#ifdef WORKBENCH
[BaseContainerProps(configRoot: true)]
class EditablePrefabsConfig
{
	[Attribute(defvalue: "Prefabs", desc: "Source directory where all prefabs are placed.", params: "unregFolders")]
	protected ResourceName m_SourceDirectory;

	[Attribute(defvalue: "PrefabsEditable", desc: "Target directory where editable sub-prefabs will be crated.\nFolder structure inside will mimic the structure in the source directory.", params: "folders")]
	protected ResourceName m_TargetDirectory;

	[Attribute(defvalue: "Auto", desc: "Auto-generated Prefabs folder name")]
	protected string m_sAutoFolderName;

	[Attribute(defvalue: "E_", desc: "Prefix added before file name of each prefab (i.e., editable entity prefab of 'Car.et' will be 'E_Car.et')")]
	protected string m_sFileNamePrefix;

	[Attribute(defvalue: "", desc: "Ignore file names containing this string. To define mutliple, separate them by a comma (without a space afterwards).")]
	protected string m_sSourceBlacklist;

	[Attribute(desc: "Entities that will be created in temporary world to ensure correct functionality (e.g., AIWorld for AI groups)", params: "et")]
	protected ref array<ResourceName> m_SupportEntities;

	[Attribute()]
	protected ref array<ref EditablePrefabsSetting> m_Settings;

	protected bool m_bIsValid;
	protected string m_sSourcePath;
	protected string m_sTargetPath;
	protected string m_sTargetPathAuto;
	protected ref map<string, string> m_mLinksFromTarget;
	protected ref map<string, string> m_mLinksFromSource;
	protected ref array<string> m_aSourceBlacklist = {};
	protected string m_sCurrentAddon = "$ArmaReforger:";

	protected static const string ADDON_PREFIX = "$profile:";
	protected static const string WORLD_PATH = "worlds/Plugins/EditablePrefabsConfig";
	protected static const string WORLD_NAME = "EditablePrefabsWorld";
	protected static const string META_EXTENSION = ".meta";
	protected static const string COPY_EXTENSION = "_copy";

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Prefab management
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Create or update editable entity prefab
	//! \param[in] api World Editor API
	//! \param[in] prefab Source prefab
	void CreateEditablePrefab(WorldEditorAPI api, ResourceName prefab, bool onlyUpdate = false, map<string, SCR_EEditablePrefabResult> results = null)
	{
		if (prefab.IsEmpty())
			return;

		if (IsBlacklisted(prefab))
		{
			if (results)
				results.Set(string.Format("@\"%1\"", prefab.GetPath()), SCR_EEditablePrefabResult.IGNORED);
			return;
		}

		if (!m_mLinksFromSource)
			GetLinks();

		string addonName = SCR_AddonTool.GetResourceLastAddon(prefab);
		m_sCurrentAddon = SCR_AddonTool.ToFileSystem(addonName);

		bool exists = false;
		string prefabPath = prefab.GetPath();
		if (prefabPath.StartsWith(m_sTargetPath))
		{
			//--- From target
			if (prefabPath.StartsWith(m_sTargetPathAuto))
			{
				//--- Update prefab in target folder
				prefab = GetSourcePrefab(prefab);
				if (prefab.IsEmpty())
					return;
				exists = true;
			}
			else
			{
				///--- Not in 'Auto' folder
				Print(string.Format("Cannot update editable prefab @\"%1\", it's not auto-generated (i.e., not in '%2' folder)!", prefabPath, m_sTargetPathAuto), LogLevel.WARNING);
				return;
			}
		}
		else
		{
			//--- From source
			Resource prefabResource = Resource.Load(prefab);
			if (prefabResource && SCR_EditableEntityComponentClass.GetEditableEntitySource(prefabResource))
			{
				//--- Is already configured as editable entity - proceed, but only update components
				UpdateEditablePrefab(api, prefab);
				return;
			}
			else
			{
				//--- Not an editable entity, find its editable variant
				ResourceName existingPrefab;
				exists = GetLinkFromSource(SCR_ConfigHelper.GetGUID(prefab, true), existingPrefab);

				if (existingPrefab.IsEmpty())
				{
					//--- Create
					if (onlyUpdate)
						return;
					string prefabName = FilePath.StripPath(prefabPath);
					prefabPath.Replace(prefabName, m_sFileNamePrefix + prefabName);
					prefabPath.Replace(m_sSourcePath, m_sTargetPathAuto);
				}
				else
				{
					//--- Update
					prefabPath = existingPrefab.GetPath();
				}
			}
		}

		//--- Create prefab directory
		string directoryPath = FilePath.StripFileName(prefabPath);
		if (!CreateDirectoryFor(directoryPath))
			return;

		IEntitySource entitySource = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entitySource)
		{
			Print(string.Format("Unable to create entity %1!", prefab), LogLevel.ERROR);
			return;
		}

		//--- Remove coordinates which were added when the entity was spawned
		entitySource.ClearVariable("coords");

		//--- Reset placement type, e.g., "slopelandcintact". Would clash with editor positioning on clients
		api.SetVariableValue(entitySource, null, "placement", "none");

		//--- Update source composition
		bool needSave;
		bool editableChildren = true;
		IEntityComponentSource compositionComponentSource = SCR_BaseContainerTools.FindComponentSource(entitySource, SCR_SlotCompositionComponent);
		if (compositionComponentSource)
		{
			compositionComponentSource.Get("m_bEditableChildren", editableChildren);
			SCR_SlotCompositionComponent compositionComponent = SCR_SlotCompositionComponent.Cast(api.SourceToEntity(entitySource).FindComponent(SCR_SlotCompositionComponent));
#ifdef WORKBENCH
			if (compositionComponent)
				needSave = compositionComponent._WB_ConfigureComposition(api, entitySource.GetAncestor());
#endif // WORKBENCH
		}

		//--- Get existing target prefab
		IEntitySource currentEntitySource;
		Resource currentResource = Resource.Load(prefabPath);
		if (currentResource && currentResource.IsValid())
			currentEntitySource = currentResource.GetResource().ToEntitySource();

		//--- Update children
		array<ref SCR_EditorLinkEntry> links = {};
		if (editableChildren && !UpdateChildPrefabs(api, entitySource, links, results: results))
		{
			if (results)
				results.Set(string.Format("@\"%1\"", prefab.GetPath()), SCR_EEditablePrefabResult.FAILED);

			//Print(string.Format("Editable entity creation FAILED: from @\"%1\"", prefab.GetPath()), LogLevel.WARNING);
			api.DeleteEntity(entitySource);
			entitySource.Release(true);
			return;
		}

		//--- Add link component
		if (links.Count() != 0)
		{
			int hasVariants;
			IEntityComponentSource linkComponent = api.CreateComponent(entitySource, "SCR_EditorLinkComponent");
			foreach (int i, SCR_EditorLinkEntry entry : links)
			{				
				if (api.CreateObjectArrayVariableMember(entitySource, {ContainerIdPathEntry("SCR_EditorLinkComponent")}, "m_aEntries", "SCR_EditorLinkEntry", i))
				{
					array<ref ContainerIdPathEntry> entryPath = {ContainerIdPathEntry("SCR_EditorLinkComponent"), ContainerIdPathEntry("m_aEntries", i)};
					api.SetVariableValue(entitySource, entryPath, "m_Prefab", entry.m_Prefab);
					api.SetVariableValue(entitySource, entryPath, "m_vPosition", entry.m_vPosition.ToString(false));
					api.SetVariableValue(entitySource, entryPath, "m_vAngles", entry.m_vAngles.ToString(false));
					api.SetVariableValue(entitySource, entryPath, "m_fScale", entry.m_fScale.ToString());
					
					//~ Check if entry has random variant
					if (hasVariants == 0)
						hasVariants = SCR_EditableEntityComponentClass.HasVariants(entry.m_Prefab);
				}
			}

			api.SetVariableValue(entitySource, {ContainerIdPathEntry("SCR_EditorLinkComponent")}, "m_bRandomizeVariants", hasVariants.ToString());
			
			//api.SetVariableValue(entitySource, {ContainerIdPathEntry("SCR_EditorLinkComponent"), ContainerIdPathEntry("m_aEntries")}, "Name", string.Format(m_sNameFormat, prefabName));
		}

		//--- Add components
		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings)
			settings.AddComponents(this, api, prefab, prefabPath, entitySource, currentEntitySource, entitySource);

		//--- Create prefab file
		string absolutePath;
		Workbench.GetAbsolutePath(m_sCurrentAddon + prefabPath, absolutePath, false);

		//--- Back up prefab and meta files
		string metaPath = prefabPath + META_EXTENSION;
		string metaPathCopy = metaPath + COPY_EXTENSION;
		array<string> backup = {};
		if (exists)
		{
			BackupFile(prefabPath, backup);
			FileIO.CopyFile(metaPath, metaPathCopy);
		}

		//--- Create new prefab
		api.CreateEntityTemplate(entitySource, absolutePath);

		//--- Restore meta file (prefab creation overwrites it) and supress 'dummy' changes of component prefabs GUIDs
		if (exists)
		{
			CompareBackup(prefabPath, backup);
			FileIO.CopyFile(metaPathCopy, metaPath);
			FileIO.DeleteFile(metaPathCopy);
		}

		//--- Save changes in the original entity
		if (needSave)
			Workbench.GetModule(WorldEditor).Save();

		api.DeleteEntity(entitySource);
		entitySource.Release(true);

		if (exists)
		{
			if (results)
				results.Set(string.Format("@\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), SCR_EEditablePrefabResult.UPDATED);
			//Print(string.Format("Editable entity UPDATED: @\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), LogLevel.DEBUG);
		}
		else
		{
			if (results)
				results.Set(string.Format("@\"%1\" from @\"%2\"", prefabPath, prefab.GetPath()), SCR_EEditablePrefabResult.CREATED);
			//Print(string.Format("Editable entity ADDED: @\"%1\" from @\"%1\"", prefabPath, prefab.GetPath()), LogLevel.DEBUG);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] prefab
	void UpdateEditablePrefab(WorldEditorAPI api, ResourceName prefab)
	{
		IEntitySource entitySource = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		IEntitySource prefabSource = entitySource.GetAncestor();
		if (!prefabSource)
			return;

		string prefabPath = prefab.GetPath();
		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings)
			settings.AddComponents(this, api, prefab, prefabPath, prefabSource, prefabSource, entitySource);

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		worldEditor.Save();

		api.DeleteEntity(entitySource);
		entitySource.Release(true);

		Print(string.Format("Editable entity UPDATED: @\"%1\"", prefabPath), LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] prefab
	//! \param[in] onlyFileChanges
	//! \return
	string VerifyEditablePrefab(WorldEditorAPI api, ResourceName prefab, bool onlyFileChanges = false)
	{
		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource || !prefabResource.IsValid())
			return string.Empty;

		BaseResourceObject prefabBase = prefabResource.GetResource();
		if (!prefabBase)
			return string.Empty;

		IEntitySource prefabEntity = prefabBase.ToEntitySource();
		if (!prefabEntity)
			return string.Empty;

		string prefabPath = prefab.GetPath();
		IEntitySource ancestorEntity = prefabEntity.GetAncestor();
		if (ancestorEntity)
		{
			string correctPath = ancestorEntity.GetResourceName().GetPath();
			correctPath.Replace(m_sSourcePath, m_sTargetPathAuto);

			string correctName = FilePath.StripPath(correctPath);
			correctPath.Replace(correctName, m_sFileNamePrefix + correctName);

			//--- Source name changed, rename the prefab as well
			if (prefabPath != correctPath)
				prefab = MoveEditablePrefab(api, prefabPath, correctPath);

			//--- Update prefab from the source
			if (!onlyFileChanges)
				CreateEditablePrefab(api, prefab);

			return correctPath;
		}
		else
		{
			//--- Source deleted, delete the prefab as well
			DeleteEditablePrefab(api, prefabPath);
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Move existing editable prefab to a new path (includes also renaming the prefab)
	//! \param[in] prevPath Current path
	//! \param[in] newPath New path
	//! \return New path with GUID
	protected ResourceName MoveEditablePrefab(WorldEditorAPI api, string prevPath, string newPath)
	{
		if (!MoveFile(prevPath, newPath))
			return ResourceName.Empty;

		EditablePrefabsSetting settings = GetSettings(newPath);
		if (settings)
			settings.Move(this, api, prevPath, newPath);

		GetLinks(true);

		string absPath;
		Workbench.GetAbsolutePath(newPath, absPath);
		if (absPath.IsEmpty())
		{
			Print(string.Format("Cannot MOVE '%1', unable to find absolute path for '%2'!", prevPath, newPath), LogLevel.ERROR);
			return ResourceName.Empty;
		}

		Print(string.Format("Editable entity MOVED:\n      Old: '%1'\n      New: '%2'", prevPath, newPath), LogLevel.WARNING);

		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		resourceManager.RegisterResourceFile(absPath, false);
		return Workbench.GetResourceName(absPath);
	}

	//------------------------------------------------------------------------------------------------
	//! Delete existing editable prefab
	protected void DeleteEditablePrefab(WorldEditorAPI api, string prefabPath)
	{
		FileIO.DeleteFile(prefabPath);
		FileIO.DeleteFile(prefabPath + META_EXTENSION);

		EditablePrefabsSetting settings = GetSettings(prefabPath);
		if (settings)
			settings.Delete(this, api, prefabPath);

		Print(string.Format("Editable entity DELETED '%1' (its source no longer exists)!", prefabPath), LogLevel.WARNING);
	}

//	//------------------------------------------------------------------------------------------------
//	//! Change all child prefabs to their editable variants.
//	//! \param[in] prefab editable entity prefab
//	//! \param[in] toEditable true to convert sourc entities to editable, false to do it the other way around
//	void UpdateEditableChildPrefabs(ResourceName prefab, bool toEditable = true)
//	{
//		string prefabPath = prefab.GetPath();
//		if (!FileIO.FileExist(prefabPath))
//			return;
//
//		Print(string.Format("--- Processing prefab '%1'", prefab), LogLevel.DEBUG);
//
//		//--- Replace prefab paths directly in the file (there is no API to do it 'cleanly')
//		FileHandle file = FileIO.OpenFile(prefabPath, FileMode.READ);
//
//		string text, line, substring, replacement;
//		int n = 0;
//		int index, length;
//		bool canReplace;
//		while (file.ReadLine(line) > 0)
//		{
//			if (line.Contains(".et\" {"))
//			{
//				index = line.IndexOf("{") + 1;
//				length = line.LastIndexOf("}") - index;
//				substring = line.Substring(index, length);
//
//				if (toEditable)
//					canReplace = GetLinkFromSource(substring, replacement);
//				else
//					canReplace = GetLinkFromTarget(substring, replacement);
//
//				if (canReplace)
//				{
//					index = line.IndexOf("\"") + 1;
//					length = line.LastIndexOf("\"") - index;
//					substring = line.Substring(index, length);
//					line.Replace(substring, replacement);
//					Print(string.Format("'%1' converted to '%2'", substring, replacement), LogLevel.DEBUG);
//				}
//				else
//				{
//					if (toEditable)
//						canReplace = GetLinkFromTarget(substring, replacement);
//					else
//						canReplace = GetLinkFromSource(substring, replacement);
//
//					if (!canReplace && n != 0)
//					{
//						index = line.IndexOf("\"") + 1;
//						length = line.LastIndexOf("\"") - index;
//						substring = line.Substring(index, length);
//						Print(string.Format("Cannot convert '%1', matching prefab not found!", substring), LogLevel.WARNING);
//					}
//				}
//			}
//
//			if (n != 0)
//				text += "\n";
//			text += line;
//			n++;
//		}
//		file.Close();
//
//		file = FileIO.OpenFile(prefabPath, FileMode.WRITE);
//		FPrint(file, text);
//		file.Close();
//	}

	//------------------------------------------------------------------------------------------------
	//! Move a file and its meta file from one path to another
	//! \param[in] prevPath current path (without GUIDs)
	//! \param[in] newPath new path (without GUIDs)
	//! \return
	bool MoveFile(string prevPath, string newPath)
	{
		if (prevPath == newPath)
		{
			Print(string.Format("Cannot MOVE '%1', target path is the same as current path!", prevPath), LogLevel.ERROR);
			return false;
		}
		if (newPath.IsEmpty())
		{
			Print(string.Format("Cannot MOVE '%1', target path is empty!", prevPath), LogLevel.ERROR);
			return false;
		}

		string newDirectory = FilePath.StripFileName(newPath);
		if (!CreateDirectoryFor(newDirectory))
			return false;

		//--- Copy prefab to new directory and delete the old one
		FileIO.CopyFile(prevPath, newPath);
		FileIO.DeleteFile(prevPath);

		//--- Copy meta file (use intermediary file to prevent warning about duplicate GUID)
		string prevMetaPath = prevPath + META_EXTENSION;
		string newMetaPath = newPath + META_EXTENSION;
		if (FileIO.FileExists(prevMetaPath))
		{
			FileIO.CopyFile(prevMetaPath, newMetaPath + COPY_EXTENSION);
			FileIO.DeleteFile(prevMetaPath);
			FileIO.CopyFile(newMetaPath + COPY_EXTENSION, newMetaPath);
			FileIO.DeleteFile(newMetaPath + COPY_EXTENSION);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Print auto-configuration results to log.
	//! \param[in] results map of confired prefabs with their results
	void LogResults(notnull map<string, SCR_EEditablePrefabResult> results)
	{
		map<SCR_EEditablePrefabResult, LogLevel> logLevels = new map<SCR_EEditablePrefabResult, LogLevel>();
		logLevels.Insert(SCR_EEditablePrefabResult.FAILED, LogLevel.ERROR);
		logLevels.Insert(SCR_EEditablePrefabResult.CREATED, LogLevel.DEBUG);
		logLevels.Insert(SCR_EEditablePrefabResult.UPDATED, LogLevel.DEBUG);
		logLevels.Insert(SCR_EEditablePrefabResult.MOVED, LogLevel.DEBUG);
		logLevels.Insert(SCR_EEditablePrefabResult.DELETED, LogLevel.WARNING);
		logLevels.Insert(SCR_EEditablePrefabResult.NON_EDITABLE, LogLevel.VERBOSE);
		logLevels.Insert(SCR_EEditablePrefabResult.IGNORED, LogLevel.VERBOSE);

		typename resultTypeName = SCR_EEditablePrefabResult;
		SCR_EEditablePrefabResult resultQueried;
		string resultName;
		LogLevel logLevel;
		for (int i = 0, count = resultTypeName.GetVariableCount(); i < count; i++)
		{
			resultTypeName.GetVariableValue(null, i, resultQueried);
			resultName = resultTypeName.GetVariableName(i);
			logLevel = logLevels.Get(resultQueried);

			foreach (string resourceName, SCR_EEditablePrefabResult result : results)
			{
				if (result == resultQueried)
					Print(string.Format("Editable Entity %1: %2", resultName, resourceName), logLevel);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Support functions
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected bool UpdateChildPrefabs(WorldEditorAPI api, IEntitySource entitySource, array<ref SCR_EditorLinkEntry> links, bool forceDisable = false, map<string, SCR_EEditablePrefabResult> results = null, int depth = 0)
	{
		ResourceName ancestor, prefabEditable;
		IEntitySource child;
		BaseContainer ancestorSource;
		bool prefabFound, sourcePrefabFound;
		bool isValid = true;
		for (int e = 0, childrenCount = entitySource.GetNumChildren(); e < childrenCount; e++)
		{
			child = entitySource.GetChild(e);
			prefabFound = false;
			sourcePrefabFound = false;
			bool forceDisableChildren = forceDisable;

			if (forceDisable)
			{
				//--- Disable, one of the parents is a link
				api.SetVariableValue(child, null, "Flags", EntityFlags.EDITOR_ONLY.ToString());
			}
			else
			{
				//--- Find editable prefab variant
				ancestorSource = child.GetAncestor();
				while (ancestorSource)
				{
					ancestor = ancestorSource.GetResourceName();
					if (ancestor.Contains("/"))
					{
						ancestor = Workbench.GetResourceName(ancestor);

						if (SCR_BaseContainerTools.FindComponentSource(child, SCR_EditableEntityComponent))
						{
							prefabFound = true;
							prefabEditable = ancestor;
						}
						else
						{
							prefabFound = GetLinkFromSource(SCR_ConfigHelper.GetGUID(ancestor, true), prefabEditable);
							sourcePrefabFound = GetLinkFromTarget(SCR_ConfigHelper.GetGUID(ancestor, true));
						}
						break;
					}
					ancestorSource = ancestorSource.GetAncestor();
				}

				if (prefabFound)
				{
					//--- Editable prefab - add a source link
					vector position;
					child.Get("coords", position);

					vector angles;
					child.Get("angles", angles);

					float scale;
					child.Get("scale", scale);

					//--- Add to the list of links
					links.Insert(new SCR_EditorLinkEntry(prefabEditable, position, angles, scale));

					//--- Mark the original entity as not-editable (override existing flags)
					api.SetVariableValue(child, null, "Flags", EntityFlags.EDITOR_ONLY.ToString());
					forceDisableChildren = true; //--- Disable all children
				}
				else if (sourcePrefabFound)
				{
					//--- ERROR: Is editable entity
					Print(string.Format("Child entity of type '%1' is already auto-generated! Source compositions must not contain any editable prefabs from '%2/%3'!", GetResourceNameLink(child), m_TargetDirectory, m_sAutoFolderName), LogLevel.WARNING);
					isValid = false;
				}
				/*else if (SCR_BaseContainerTools.FindComponentSource(child, RplComponent))
				{
					//--- ERROR: Contains RplComponent
					Print(string.Format("Child entity '%1' contains RplComponent, but does not have editable variant! Children must not be replicated!", GetResourceNameLink(child)), LogLevel.WARNING);
					isValid = false;
				}*/
				else
				{
					if (results)
						results.Set(GetResourceNameLink(child), SCR_EEditablePrefabResult.NON_EDITABLE);

					if (!SCR_BaseContainerTools.FindComponentSource(child, "Hierarchy"))
					{
						//--- Non-editable prefab or no prefab at all - add to hierarchy
						api.CreateComponent(child, "Hierarchy");
					}
				}
			}

			//--- We need to go deeper
			isValid &= UpdateChildPrefabs(api, child, links, forceDisableChildren, results, depth + 1);
		}
		return isValid;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableComponent(WorldEditorAPI api, IEntitySource entitySource, BaseContainer componentSource, array<ref ContainerIdPathEntry> componentsPath)
	{
		if (!componentSource)
			return;

		array<ref ContainerIdPathEntry> componentsPathNew = {};
		foreach (ContainerIdPathEntry entry : componentsPath)
		{
			componentsPathNew.Insert(entry);
		}
		componentsPathNew.Insert(new ContainerIdPathEntry(componentSource.GetClassName()));

		if (componentSource.GetClassName() != "Hierarchy") //--- Don't disable hierarchy, dummy entities should still be tied to their parent
			api.SetVariableValue(entitySource, componentsPathNew, "Enabled", "0");

		BaseContainerList components = componentSource.GetObjectArray("components");
		if (components)
		{
			int componentsCount = components.Count();
			for (int i = 0; i < componentsCount; i++)
			{
				DisableComponent(api, entitySource, components.Get(i), componentsPathNew);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void BackupFile(string filePath, out notnull array<string> backup)
	{
		FileHandle file = FileIO.OpenFile(filePath, FileMode.READ);
		string line;
		while (file.ReadLine(line) > 0)
		{
			backup.Insert(line);
		}
		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void CompareBackup(string filePath, notnull array<string> backup)
	{
		FileHandle file = FileIO.OpenFile(filePath, FileMode.READ);
		string line, text;
		int i = 0;
		int backupCount = backup.Count();
		bool apply = false;
		while (file.ReadLine(line) > 0)
		{
			if (i >= backupCount)
			{
				apply = false;
				break;
			}

			if (line != backup[i])
			{
				if (line.Contains(".ct"))
				{
					//--- Component GUID updated, restore the original to prevent false-flag SVN diffs
					line = backup[i];
					apply = true;
				}
				else
				{
					//--- Major file change, don't restore the backup
					apply = false;
					break;
				}
			}
			if (i != 0)
				text += "\n";
			text += line;
			i++;
		}
		file.Close();

		if (apply)
		{
			file = FileIO.OpenFile(filePath, FileMode.WRITE);
			file.Write(text);
			file.Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetResourceNameLink(BaseContainer container)
	{
		string resourceName = container.GetClassName();
		while (container)
		{
			if (container.GetResourceName().Contains("/"))
			{
				resourceName = string.Format("@\"%1\"", container.GetResourceName().GetPath());
				break;
			}
			container = container.GetAncestor();
		}
		return resourceName;
	}

	//------------------------------------------------------------------------------------------------
	protected EditablePrefabsSetting GetSettings(string prefabPath)
	{
		EditablePrefabsSetting settings = null;
		int pathLengthMax = -1;
		foreach (EditablePrefabsSetting settingsTemp : m_Settings)
		{
			string targetPath = settingsTemp.GetTarget();
			int pathLength = targetPath.Length();
			if (prefabPath.StartsWith(targetPath) && pathLength > pathLengthMax)
			{
				settings = settingsTemp;
				pathLengthMax = pathLength;
			}
		}
		return settings;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsBlacklisted(string path)
	{
		foreach (string entry : m_aSourceBlacklist)
		{
			if (path.Contains(entry))
				return true;
		}
		return false;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Attribute getters
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Get source path
	//! \return path without GUID
	string GetSourcePath()
	{
		return m_sSourcePath;
	}

	//------------------------------------------------------------------------------------------------
	//! Get raw target path
	//! \return path without GUID
	string GetTargetPath()
	{
		return m_sTargetPath;
	}

	//------------------------------------------------------------------------------------------------
	//! Get path of auto folder in target path
	//! \return path without GUID
	string GetTargetPathAuto()
	{
		return m_sTargetPathAuto;
	}

	//------------------------------------------------------------------------------------------------
	//! Get prefix added to editable entity prefabs
	//! \return prefix
	string GetPrefix()
	{
		return m_sFileNamePrefix;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	//--- General getters
	////////////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! Get config from a prefab
	//! \param[in] configPath prefab
	//! \return config
	static EditablePrefabsConfig GetConfig(ResourceName configPath)
	{
		Resource configResource = Resource.Load(configPath);
		if (!configResource.IsValid())
		{
			Print(string.Format("Cannot load config '%1'!", configPath), LogLevel.ERROR);
			return null;
		}

		BaseResourceObject configContainer = configResource.GetResource();
		if (!configContainer)
			return null;

		BaseContainer configBase = configContainer.ToBaseContainer();
		if (!configBase)
			return null;

		if (configBase.GetClassName() != "EditablePrefabsConfig")
		{
			Print(string.Format("Config '%1' is of type '%2', must be 'EditablePrefabsConfig'!", configPath, configBase.GetClassName()), LogLevel.ERROR);
			return null;
		}

		return EditablePrefabsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configBase));
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the config is configured correctly
	//! \return true if valid
	bool IsValid()
	{
		return m_bIsValid;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a directory for editable entity prefab on given path
	//! \param[in] filePath prefab path
	//! \param[in] addon
	//! \return true if created
	bool CreateDirectoryFor(out string filePath, string addon = "")
	{
		array<string> directories = {};
		filePath.Split("/", directories, true);

		int directoriesCount = directories.Count();
		if (directoriesCount == 0)
			return false;

		if (addon.IsEmpty())
			addon = m_sCurrentAddon;

		filePath = addon;
		for (int i = 0; i < directoriesCount; i++)
		{
			filePath = FilePath.Concat(filePath, directories[i]);

			if (!FileIO.MakeDirectory(filePath))
			{
				Print(string.Format("Unable to create directory '%1'!", filePath), LogLevel.ERROR);
				return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create empty world which can be safely edited.
	//! \return
	bool CreateWorld()
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return false;

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
			return false;

		//--- World already opened, skip
		string currentWorld;
		api.GetWorldPath(currentWorld);
		if (currentWorld == ADDON_PREFIX + WORLD_PATH + "/" + WORLD_NAME + ".ent")
			return true;

		string worldPath = SCR_WorldFilesHelper.CreateWorld(ADDON_PREFIX + WORLD_PATH, WORLD_NAME, m_SupportEntities, false, true);
		if (worldPath.IsEmpty())
		{
			Print("Cannot create Editable Prefabs world files", LogLevel.WARNING);
			return false;
		}

		Sleep(1); // required to open the world right after creation
		if (!worldEditor.SetOpenedResource(worldPath))
		{
			Print("Cannot open Editable Prefabs world - if the world was just created, try restarting Workbench - " + worldPath, LogLevel.NORMAL);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Initialise links to and from editable prefabs
	//! \param[in] forced
	void GetLinks(bool forced = false)
	{
		//--- Already defined
		if (!forced && m_mLinksFromTarget && m_mLinksFromSource)
			return;

		m_mLinksFromTarget = new map<string, string>();
		m_mLinksFromSource = new map<string, string>();

		ref array<ResourceName> resources = {};
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = { "et" };
		ResourceDatabase.SearchResources(filter, resources.Insert);

		string prefabPath, prefabGUID, ancestorGUID;
		ResourceName ancestor;
		Resource prefabResource;
		IEntitySource entitySource, ancestorSource;
		foreach (ResourceName prefab : resources)
		{
			prefabPath = prefab.GetPath();
			if (!prefabPath.StartsWith(m_sTargetPathAuto))
				continue;

			prefabResource = Resource.Load(prefab);
			if (!prefabResource || !prefabResource.IsValid())
				continue;

			entitySource = SCR_BaseContainerTools.FindEntitySource(prefabResource);
			if (!entitySource)
				continue;

			ancestorSource = entitySource.GetAncestor();
			if (!ancestorSource)
				continue;

			ancestor = ancestorSource.GetResourceName();

			prefabGUID = SCR_ConfigHelper.GetGUID(prefab, true);
			ancestorGUID = SCR_ConfigHelper.GetGUID(ancestor, true);

			m_mLinksFromTarget.Insert(prefabGUID, ancestor);
			m_mLinksFromSource.Insert(ancestorGUID, prefab);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get a link from a prefab to its editable prefab
	//! \param[in] guid prefab GUID
	//! \param[out] linkedPrefab found prefab
	//! \return true if found
	bool GetLinkFromSource(string guid, out ResourceName linkedPrefab = ResourceName.Empty)
	{
		return m_mLinksFromSource.Find(guid, linkedPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a link from editable prefab to its source prefab
	//! \param[in] guid prefab GUID
	//! \param[out] linkedPrefab found prefab
	//! \return true if found
	//!
	bool GetLinkFromTarget(string guid, out ResourceName linkedPrefab = ResourceName.Empty)
	{
		return m_mLinksFromTarget.Find(guid, linkedPrefab);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the prefab is an editable entity
	//! \return True if it's an editable entity
	bool IsEditableEntity(ResourceName prefab)
	{
		return prefab.GetPath().StartsWith(m_sTargetPath);
	}

	//------------------------------------------------------------------------------------------------
	//! Get source prefab of editable entity prefab
	//! \param[in] prefab Queried prefab
	//! \return Source prefab
	ResourceName GetSourcePrefab(ResourceName prefab)
	{
		if (!IsEditableEntity(prefab))
			return ResourceName.Empty;

		Resource prefabResource = Resource.Load(prefab);
		if (!prefabResource.IsValid())
			return ResourceName.Empty;

		IEntitySource prefabEntity = SCR_BaseContainerTools.FindEntitySource(prefabResource);
		if (!prefabEntity)
			return ResourceName.Empty;

		IEntitySource ancestorEntity = prefabEntity.GetAncestor();
		if (!ancestorEntity)
			return ResourceName.Empty;

		return ancestorEntity.GetResourceName();
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void EditablePrefabsConfig()
	{
		if (m_SourceDirectory.IsEmpty())
			Print("Cannot use config EditablePrefabsConfig, m_SourceDirectory attribute is empty!", LogLevel.ERROR);

		if (m_TargetDirectory.IsEmpty())
			Print("Cannot use config EditablePrefabsConfig, m_TargetDirectory attribute is empty!", LogLevel.ERROR);

		m_bIsValid = true;
		m_sSourcePath = m_SourceDirectory.GetPath();
		m_sTargetPath = m_TargetDirectory.GetPath();
		m_sTargetPathAuto = m_sTargetPath + "/" + m_sAutoFolderName;
		m_sSourceBlacklist.Split(",", m_aSourceBlacklist, true);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Target")]
class EditablePrefabsSetting
{
	[Attribute(params: "unregFolders et")]
	protected ResourceName m_Target;

	[Attribute()]
	protected ref array<ref EditablePrefabsComponent_Base> m_Components;

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetTarget()
	{
		return m_Target.GetPath();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] prefab
	//! \param[in] targetPath
	//! \param[in] entitySource
	//! \param[in] currentEntitySource
	//! \param[in] instanceEntitySource
	void AddComponents(EditablePrefabsConfig config, WorldEditorAPI api, ResourceName prefab, string targetPath, IEntitySource entitySource, IEntitySource currentEntitySource, IEntitySource instanceEntitySource)
	{
		//--- Get existing components on source file (to prevent duplicates)
		BaseContainer componentContainer, componentCurrent;
		map<string, BaseContainer> sourceComponentClasses = new map<string, BaseContainer>;
		int componentsCount = entitySource.GetComponentCount();
		for (int i = 0; i < componentsCount; i++)
		{
			componentContainer = entitySource.GetComponent(i);
			sourceComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
		}

		//--- Get existing components on target file (to avoid overriding certain values)
		map<string, BaseContainer> currentComponentClasses = new map<string, BaseContainer>;
		if (currentEntitySource)
		{
			componentsCount = currentEntitySource.GetComponentCount();
			for (int i = 0; i < componentsCount; i++)
			{
				componentContainer = currentEntitySource.GetComponent(i);
				currentComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
			}
		}

		//--- Add components from prefabs
		Resource componentResource;
		BaseResourceObject componentBase;
		foreach (EditablePrefabsComponent_Base component : m_Components)
		{
			//--- Get component on current target prefab
			if (!sourceComponentClasses.Find(component.GetClassName(), componentContainer))
				componentContainer = component.AddComponent(config, api, prefab, targetPath, entitySource);

			if (componentContainer)
			{
				//--- If the component exists in source prefab, but is disabled, make sure to enable it here
				bool enabled;
				componentContainer.Get("Enabled", enabled);
				if (!enabled)
					api.SetVariableValue(entitySource, { new ContainerIdPathEntry(componentContainer.GetClassName()) }, "Enabled", "1");

				currentComponentClasses.Find(component.GetClassName(), componentCurrent);
				component.EOnCreate(config, api, prefab, targetPath, entitySource, instanceEntitySource, componentContainer, componentCurrent);
				sourceComponentClasses.Insert(componentContainer.GetClassName(), componentContainer);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] prefabPath
	void Delete(EditablePrefabsConfig config, WorldEditorAPI api, string prefabPath)
	{
		foreach (EditablePrefabsComponent_Base component : m_Components)
		{
			component.EOnDelete(config, api, prefabPath);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] config
	//! \param[in] api
	//! \param[in] currentPath
	//! \param[in] newPath
	void Move(EditablePrefabsConfig config, WorldEditorAPI api, string currentPath, string newPath)
	{
		foreach (EditablePrefabsComponent_Base component : m_Components)
		{
			component.EOnMove(config, api, currentPath, newPath);
		}
	}
}

enum SCR_EEditablePrefabResult
{
	FAILED,
	CREATED,
	UPDATED,
	MOVED,
	DELETED,
	NON_EDITABLE,
	IGNORED
}
#endif // WORKBENCH
