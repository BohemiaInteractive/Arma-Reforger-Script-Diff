#ifdef WORKBENCH
//! @defgroup GenerateWindowsPrefabsTool Windows Prefab Generator
//! Workbench plugin for generation of windows, sash & glass prefabs

//! @ingroup GenerateWindowsPrefabsTool

//! Workbench plugin for generation of windows & glass prefabs.
//!
//! Multiple buttons for:
//! - glass, sash and frame generation from XOB
//! - sash and window prefab update
[WorkbenchToolAttribute(
	name: "Windows Prefabs Generator Tool",
	description: "Generate prefabs for destroyable windows.\n\n" +
		"- select sash/window xob/prefab by:\n" +
		"-- using the Resource Browser\n" +
		"-- opening the prefab in Prefab Edit Mode\n" +
		"- select glass xob from the UI",
	awesomeFontCode : 0xF2D2)]
class SCR_GenerateWindowsPrefabsTool : WorldEditorTool
{
	/*
		Category: General
	*/

	[Attribute(defvalue: "1", uiwidget: UIWidgets.ComboBox, desc: "File system where new prefabs will be created", enums: SCR_ParamEnumArray.FromAddons(), category: "General")]
	protected int m_iAddonToUse;

	[Attribute(defvalue: "1", desc: "Remove updated prefab's existing children", category: "General")]
	protected bool m_bRemoveExistingChildren;

	/*
		Category: Glass
	*/

	[Attribute(defvalue: "{86834A0D5920F32F}Prefabs/Structures/Core/DestructibleGlass_Base.et", desc: "Choose a base class of your glass prefabs", params: "et", category: "Glass")]
	protected ResourceName m_sGlassBasePrefab;

	[Attribute(desc: "Path to save the glass prefabs", params: "unregFolders", category: "Glass")]
	protected ResourceName m_sGlassSavePath;

	[Attribute(desc: "Choose XOB files to process", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "xob", category: "Glass")]
	protected ref array<ResourceName> m_aGlassVariants;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "Number of damage variants - set to -1 to autodetect", params: "-1 100 1", category: "Glass")]
	protected int m_iGlassDmgCount;

	[Attribute(desc: "Model base path and base name to use for the shards; leave empty to generate a name from the save path", category: "Glass")]
	protected string m_sGlassName;

	[Attribute(defvalue: "1", desc: "Use Multi Phase destruction instead of Fractal destruction", category: "Glass")]
	protected bool m_bUseMultiPhaseDestruction;

	/*
		Category: Sash
	*/

	[Attribute(defvalue: "{08C59B2AAE05ACFE}Prefabs/Structures/BuildingParts/Windows/WindowSash_base.et", desc: "Base class for the sash prefabs", params: "et", category: "Sash")]
	protected ResourceName m_sSashBasePrefab;

	[Attribute(defvalue: "{604D1E6E05FB1038}Assets/Structures/BuildingsParts/Windows", desc: "Path to save the sash prefabs", params: "unregFolders", category: "Sash")]
	protected ResourceName m_sSashSavePath;

	/*
		Category: Window
	*/

	[Attribute(defvalue: "{3FF56BA19C8780DE}Prefabs/Structures/BuildingParts/Windows/Window_Base.et", desc: "Base class for window prefabs", params: "et", category: "Window")]
	protected ResourceName m_sWindowBasePrefab;

	[Attribute(defvalue: "{604D1E6E05FB1038}Assets/Structures/BuildingsParts/Windows", desc: "Path to save the window prefabs", params: "unregFolders", category: "Window")]
	protected ResourceName m_sWindowSavePath;

	protected static const string BASE_PREFAB_SUFFIX = "_base"; //< only used to name the Prefab file - case-insensitive
	protected static const string MESHOBJECT_CLASSNAME = "MeshObject";
	protected static const string HIERARCHY_CLASSNAME = "Hierarchy";
	protected static const string GENERICENTITY_CLASSNAME = "GenericEntity";

	// criteria MUST be lowercase as ResourceNames are lowercased when searched
	protected static const ref array<string> GLASS_CRITERIA = { "glass_", "x" };
	protected static const ref array<string> SASH_CRITERIA = { "winsash_", "x" };
	protected static const ref array<string> WINDOW_CRITERIA = { "win_", "x" };

	//! these criteria must target/filter ALL bones (window or sash), so no "glass_" filter in there
	protected static const ref array<string> BONE_CRITERIA = { "x" };

	/*
		Category: Debug
	*/

	// pure IEntitySource mode activated! -not yet-
	// [Attribute(category: "DEBUG - DO NOT USE FOR PRODUCTION")]
	bool m_bUsePureIEntitySource;

	/*

		BUTTON SECTION

	*/

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("IMPORT: Glass .xob")]
	protected void CreateGlassButton()
	{
		CreateGlass();
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Sash .xob")]
	protected void CreateSashesButton()
	{
		CreateAndFillPrefabFromXOBs(SASH_CRITERIA, { GLASS_CRITERIA }, m_sSashBasePrefab, m_sSashSavePath);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Frame .xob")]
	protected void CreateFramesButton()
	{
		CreateAndFillPrefabFromXOBs(WINDOW_CRITERIA, { SASH_CRITERIA, GLASS_CRITERIA }, m_sWindowBasePrefab, m_sWindowSavePath);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("UPDATE: Sash .et")]
	protected void UpdateSashPrefabs()
	{
		RefreshPrefabs(SASH_CRITERIA, { GLASS_CRITERIA });
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Window .et")]
	protected void UpdateWindowPrefabs()
	{
		RefreshPrefabs(WINDOW_CRITERIA, { SASH_CRITERIA, GLASS_CRITERIA });
	}

	/*

		CREATE SECTION

	*/

	//------------------------------------------------------------------------------------------------
	//! Create a Prefab with the found XOBs
	protected void CreateAndFillPrefabFromXOBs(notnull array<string> xobCriteria, notnull array<ref array<string>> fillerCriteria, ResourceName basePrefab, ResourceName saveDirectory)
	{
		// check input
		if (saveDirectory.IsEmpty())
		{
			Print("Save Directory is empty. Please select a valid save location", LogLevel.WARNING);
			return;
		}

		if (basePrefab.IsEmpty())
		{
			Print("Base Class is empty. Please select a valid base prefab", LogLevel.WARNING);
			return;
		}

		// check saveDir
		string absoluteSaveDir;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddonToUse, saveDirectory, absoluteSaveDir))
		{
			Print("Wrong path: " + SCR_AddonTool.GetAddonID(m_iAddonToUse) + saveDirectory.GetPath(), LogLevel.WARNING);
			return;
		}

		// get & check XOBs
		array<ResourceName> foundXOBs = GetListedSelectedOrOpenedResources("xob", xobCriteria);
		if (foundXOBs.IsEmpty())
		{
			Print("No Sash models are listed nor are they selected via the Resource Browser. Do one of the two", LogLevel.WARNING);
			return;
		}

		map<string, ResourceName> prefabMap = PrefabSearchMap(fillerCriteria);

		foreach (ResourceName xobPath : foundXOBs)
		{
			// create entity with selected base class
			m_API.BeginEntityAction("Processing " + xobPath);

			if (!CreateAndSaveBaseAndDefaultPrefabFromXOB(prefabMap, xobPath, basePrefab, absoluteSaveDir))
				Print("Could not process " + xobPath.GetPath(), LogLevel.ERROR);

			m_API.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Save both _base prefab (empty) and a default prefab (inheriting from the base one, with created children)
	//! \return true on success, false on failure
	protected bool CreateAndSaveBaseAndDefaultPrefabFromXOB(map<string, ResourceName> prefabMap, ResourceName xobPath, ResourceName parentPrefab, string absoluteSaveDir)
	{
		IEntitySource entitySource = m_API.CreateEntity(parentPrefab, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entitySource)
		{
			Print("Prefab's entity could not be created", LogLevel.ERROR);
			return false;
		}

		// find MeshObject component - if it doesn't exist, SKIP
		if (!SCR_BaseContainerTools.FindComponentSource(entitySource, MESHOBJECT_CLASSNAME))
		{
			Print(parentPrefab + " does not have the " + MESHOBJECT_CLASSNAME + " component - fix the base prefab " + parentPrefab, LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		// apply XOB path to mesh
		if (!m_API.SetVariableValue(entitySource, { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) }, "Object", xobPath))
		{
			Print("Could not apply XOB model", LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		string fileNameWithoutExtension = FilePath.StripExtension(FilePath.StripPath(xobPath));

		// save as base prefab
		string absoluteFilePath = FilePath.Concat(absoluteSaveDir, fileNameWithoutExtension + BASE_PREFAB_SUFFIX + ".et");
		if (!SaveEntitySourceAsNewTemplate(entitySource, absoluteFilePath))
		{
			Print("Script was unable to create BASE prefab", LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		// prepare the default prefab
		entitySource = m_API.CreateEntity(SCR_WorldEditorToolHelper.GetResourceNameFromFile(absoluteFilePath), "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entitySource)
		{
			Print("Could not create BASE prefab!", LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		// get all bones and create prefab from them - if prefabMap is provided
		if (prefabMap && !prefabMap.IsEmpty())
			CreateEntitySourcesFromBoneNames(prefabMap, entitySource, true);

		// save the default prefab
		absoluteFilePath = FilePath.Concat(absoluteSaveDir, fileNameWithoutExtension + ".et");
		if (!SaveEntitySourceAsNewTemplate(entitySource, absoluteFilePath))
		{
			Print("Could not create DEFAULT prefab", LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! As its name says, take the provided entitySource and save it as a new Prefab
	//! \return true on success, false on failure
	protected bool SaveEntitySourceAsNewTemplate(notnull IEntitySource entitySource, string absoluteFilePath)
	{
		bool fileCreated = m_API.CreateEntityTemplate(entitySource, absoluteFilePath);

		if (fileCreated)
			Print("Created " + absoluteFilePath, LogLevel.VERBOSE);
		else
			Print("Failed to create " + absoluteFilePath, LogLevel.WARNING);

		return fileCreated;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a prefab from the provided XOBs
	protected void CreateGlass()
	{
		// Validate input parameters
		if (m_sGlassSavePath.IsEmpty())
		{
			Print("Path To SaveGlass is empty. Please select a valid save location", LogLevel.WARNING);
			return;
		}

		if (m_sGlassBasePrefab.IsEmpty())
		{
			Print("Base Class Glass is empty. Please select a valid base prefab for your glass", LogLevel.WARNING);
			return;
		}

		if (m_aGlassVariants)
		{
			for (int i = m_aGlassVariants.Count() - 1; i >= 0; i--)
			{
				if (m_aGlassVariants[i].IsEmpty())
					m_aGlassVariants.RemoveOrdered(i);
			}

			WorldEditorTool.UpdatePropertyPanel();
		}

		if (!m_aGlassVariants || m_aGlassVariants.IsEmpty())
		{
			Print("Glass Variants is empty. Please select at least one glass XOB variant", LogLevel.WARNING);
			return;
		}

		string glassName = m_sGlassName.Trim();
		if (glassName.IsEmpty())
			glassName = FilePath.StripPath(m_sGlassSavePath.GetPath());

		// check saveDir
		string absoluteSaveDir;
		if (!SCR_AddonTool.GetAddonAbsolutePath(m_iAddonToUse, m_sGlassSavePath, absoluteSaveDir))
		{
			Print("Wrong path: " + SCR_AddonTool.GetAddonID(m_iAddonToUse) + m_sGlassSavePath.GetPath(), LogLevel.WARNING);
			return;
		}

		// Automatically detect number of glass variants
		int glassVariants = m_iGlassDmgCount;
		if (glassVariants < 0)
			glassVariants = GetGlassCount(m_aGlassVariants[0]);

		// Create new entity
		m_API.BeginEntityAction("Processing " + glassName);
		IEntitySource entitySource = m_API.CreateEntity(m_sGlassBasePrefab, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		array<ref ContainerIdPathEntry> containerPath;

		// Add glass variants to entity with correct parameters
		// Use Multi Phase destruction
		if (m_bUseMultiPhaseDestruction)
		{
			// Modify SCR_DestructionMultiPhaseComponent first m_DamagePhases and replace m_PhaseModel with ResourceName fetched from SCR_DestructionFractalComponent
			array<ref ContainerIdPathEntry> subContainerPath = { ContainerIdPathEntry("SCR_DestructionMultiPhaseComponent"), ContainerIdPathEntry("m_DamagePhases", 0) };
			m_API.SetVariableValue(entitySource, subContainerPath, "m_PhaseModel", GetDestroyedGlassModelPath(m_aGlassVariants[0], 0));
		}
		else
		{
			// Use Fractal destruction
			containerPath = { new ContainerIdPathEntry("SCR_DestructionFractalComponent") };
			foreach (int currentIndex, ResourceName glassVariant : m_aGlassVariants)
			{
				for (int i = glassVariants; i > 0; i--)
				{
					// Modify damage mask
					string damageMask = GetDamageMask(glassVariant);
					if (glassVariants > 1)
					{
						string tempPath = FilePath.StripFileName(damageMask);
						string tempFile = FilePath.StripPath(damageMask);
						tempFile.Replace("Glass_01", "Glass_" + i.ToString(2));
						damageMask = tempPath + tempFile;
					}

					// Apply parameters to sub variant
					m_API.CreateObjectArrayVariableMember(entitySource, containerPath, "m_FractalVariants", "SCR_FractalVariation", currentIndex);
					array<ref ContainerIdPathEntry> subContainerPath = { ContainerIdPathEntry("SCR_DestructionFractalComponent"), ContainerIdPathEntry("m_FractalVariants", currentIndex) };
					m_API.SetVariableValue(entitySource, subContainerPath, "m_ModelNormal", (string)glassVariant);
					m_API.SetVariableValue(entitySource, subContainerPath, "m_ModelDestroyed", GetDestroyedGlassModelPath(glassVariant, i));
					m_API.SetVariableValue(entitySource, subContainerPath, "m_aModelFragments", GetDamageVariants(damageMask));
				}
			}
		}

		// Assign some model to MeshObject
		containerPath = { new ContainerIdPathEntry(MESHOBJECT_CLASSNAME) };
		m_API.SetVariableValue(entitySource, containerPath, "Object", m_aGlassVariants[0]);

		// Save our modified entity to prefab
		bool fileCreated = m_API.CreateEntityTemplate(entitySource, FilePath.Concat(absoluteSaveDir, glassName + ".et"));
		if (fileCreated)
			Print(string.Format("@\"%1\"", FilePath.Concat(absoluteSaveDir, glassName + ".et")), LogLevel.NORMAL);
		else
			Print("Script was unable to create new prefab at the designated location", LogLevel.ERROR);

		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Get path to destroyed glass model
	protected ResourceName GetDestroyedGlassModelPath(ResourceName resourceName, int variant)
	{
		if (resourceName.IsEmpty())
		{
			Print("Provided glass is empty!", LogLevel.WARNING);
			return resourceName;
		}

		string tempString = resourceName.GetPath();
		if (variant > 0)
			tempString.Replace("Glass_01", "Glass_" + variant.ToString(2));

		tempString = FilePath.StripFileName(tempString) + "dst/" + FilePath.StripPath(tempString);
		tempString.Replace(".xob", "_dst.xob");

		string absPathDst;
		if (!Workbench.GetAbsolutePath(tempString, absPathDst, true))
		{
			Print("Unable to find destroyed model for " + tempString + ". Base model will be used instead", LogLevel.WARNING);
			return resourceName;
		}

		return SCR_WorldEditorToolHelper.GetResourceNameFromFile(absPathDst);
	}

	//------------------------------------------------------------------------------------------------
	//! Get damage mask string
	//! \return resourceNameDirPath/Dmg/resourceNameFileName with ".xob" replaced by "_dmg_"
	protected string GetDamageMask(ResourceName resourceName)
	{
		string inputString = resourceName.GetPath();
		inputString = FilePath.StripFileName(inputString) + "Dmg/" + FilePath.StripPath(inputString);
		inputString.Replace(".xob", "_dmg_");
		return inputString;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all variants (resourceName + "00", "01", "02" etc) present in data
	//! \return comma-separated resourceNames
	protected string GetDamageVariants(ResourceName resourceName)
	{
		bool variantExists = true;
		int numberOfVariants = 0;

		array<string> variants = {};
		while (variantExists)
		{
			string absPath;
			string inputStringTemp = resourceName + numberOfVariants.ToString(2) + ".xob";
			variantExists = Workbench.GetAbsolutePath(inputStringTemp, absPath);
			if (variantExists)
			{
				variants.Insert(Workbench.GetResourceName(inputStringTemp));
				numberOfVariants++;
			}
		}

		return SCR_StringHelper.Join(",", variants, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Count how many damage variants exist
	protected int GetGlassCount(ResourceName glassResourceName)
	{
		if (glassResourceName.IsEmpty())
			return 0;

		bool variantExists = true;
		int numberOfVariants = 1;
		string relativeFilePath = glassResourceName.GetPath();
		relativeFilePath.Replace("Glass_01", "Dmg/Glass_01");

		while (variantExists)
		{
			string inputStringTemp = relativeFilePath;
			inputStringTemp.Replace(".xob", "_dmg_00.xob");
			inputStringTemp.Replace("Glass_01", "Glass_" + numberOfVariants.ToString(2));
			Print(inputStringTemp, LogLevel.DEBUG);

			variantExists = Workbench.GetAbsolutePath(inputStringTemp, inputStringTemp, true);
			if (variantExists)
				numberOfVariants++;
		}

		return numberOfVariants;
	}

	/*

		REFRESH SECTION

	*/

	//------------------------------------------------------------------------------------------------
	//! Update prefabs - to use other prefabs deduced from bone names
	protected void RefreshPrefabs(notnull array<string> prefabCriteria, notnull array<ref array<string>> fillerCriteria)
	{
		array<ResourceName> prefabsToUpdate = GetListedSelectedOrOpenedResources("et", prefabCriteria);
		if (prefabsToUpdate.IsEmpty())
		{
			Print("No Prefabs were found to update", LogLevel.WARNING);
			return;
		}

		map<string, ResourceName> prefabMap = PrefabSearchMap(fillerCriteria);
		if (prefabMap.IsEmpty())
		{
			Print("No filler Prefabs were found", LogLevel.WARNING);
			return;
		}

		foreach (ResourceName prefabToUpdate : prefabsToUpdate)
		{
			if (prefabToUpdate.IsEmpty())
				continue;

			m_API.BeginEntityAction("Processing " + prefabToUpdate);

			if (!RefreshPrefab(prefabMap, prefabToUpdate))
				Print("Could not process " + prefabToUpdate, LogLevel.ERROR);

			m_API.EndEntityAction();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true = success, false = failure
	protected bool RefreshPrefab(notnull map<string, ResourceName> boneResourceNames, ResourceName prefabResourceName)
	{
		IEntitySource entitySource = m_API.CreateEntity(prefabResourceName, "", m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
		if (!entitySource)
			return false;

		IEntitySource actualPrefab = IEntitySource.Cast(entitySource.GetAncestor());
		if (!actualPrefab)
		{
			Print("Created entity's source does not have an ancestor", LogLevel.ERROR);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
			return false;
		}

		// remove existing children
		array<IEntitySource> childEntitySources;
		if (m_bRemoveExistingChildren && entitySource.GetNumChildren() > 0)
		{
			childEntitySources = {};
			for (int i, count = entitySource.GetNumChildren(); i < count; i++)
			{
				childEntitySources.Insert(actualPrefab.GetChild(i));
			}

			if (!childEntitySources.IsEmpty()) // crashes otherwise
				m_API.RemovePrefabMembers(childEntitySources);
		}

		array<ref Resource> childrenResources; // needed for nullpointer reasons
		if (m_bUsePureIEntitySource)
		{
			Print("USE PURE IENTITYSOURCE", LogLevel.DEBUG);

			childrenResources = CreateResourcesFromBoneNames(boneResourceNames, entitySource);

			// reused array var
			childEntitySources = {};
			foreach (Resource resource : childrenResources)
			{
				childEntitySources.Insert(resource.GetResource().ToEntitySource());
			}
			if (childEntitySources && !childEntitySources.IsEmpty())
				m_API.AddPrefabMembers(actualPrefab, childEntitySources);
		}
		else
		{
			Print("USE WORLDEDITOR IENTITYSOURCE", LogLevel.DEBUG);

			// reused array var
			childEntitySources = CreateEntitySourcesFromBoneNames(boneResourceNames, entitySource, false);
			if (childEntitySources && !childEntitySources.IsEmpty())
				m_API.MoveEntitiesToPrefab(entitySource, actualPrefab, childEntitySources);	// deletes children IEntities in the process
		}

		// update the prefab's .et
		bool savedSuccessfully = m_API.SaveEntityTemplate(actualPrefab);
		if (savedSuccessfully)
			Print("Saved successfully", LogLevel.VERBOSE);
		else
			Print(prefabResourceName.GetPath() + " failed to save!", LogLevel.ERROR);

		SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
		return true;
	}

	/*

		GLOBAL SECTION

	*/

	//------------------------------------------------------------------------------------------------
	//! Get prefab search result as a map of filename (lowercase)/resourceName
	//! \return filenameLC-prefabResourceName pair map - (e.g "glass_100x42"-"{6B82AB4CBFE63D6E}Prefabs/Structures/BuildingParts/Windows/Glass_100x42/Glass_100x42.et")

	protected map<string, ResourceName> PrefabSearchMap(notnull array<ref array<string>> searchStringGroups)
	{
		map<string, ResourceName> result = new map<string, ResourceName>();
		array<ResourceName> searchResult;
		foreach (array<string> searchStrings : searchStringGroups)
		{
			foreach (ResourceName resourceName : SCR_WorkbenchHelper.SearchWorkbenchResources({ "et" }, searchStrings))
			{
				string toLowerFileNameWithoutExtension = FilePath.StripExtension(FilePath.StripPath(resourceName));
				toLowerFileNameWithoutExtension.ToLower();
				result.Insert(toLowerFileNameWithoutExtension, resourceName);
				Print("Adding " + toLowerFileNameWithoutExtension + " - " + resourceName, LogLevel.VERBOSE);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get defined array resources or selected resources - filtered by keyword
	//! \param[in] resourceNames
	//! \param[in] wantedExtension without the separating period, e.g "xob"
	//! \param[in] keywords (lowercase) mandatory keywords that must exist in resource filename
	//! \return array of results - either selected or array-listed ResourceNames
	protected array<ResourceName> GetListedSelectedOrOpenedResources(string wantedExtension = "", array<string> keywords = null, array<ResourceName> resourceNames = null)
	{
		array<ResourceName> temporaryResults;
		if (resourceNames && !resourceNames.IsEmpty())
			temporaryResults = resourceNames;
		else // find selected assets
			temporaryResults = SCR_WorldEditorToolHelper.GetSelectedOrOpenedResources();

		array<ResourceName> result = {};
		foreach (ResourceName resourceName : temporaryResults)
		{
			string extension;
			string resourceNameLC = FilePath.StripExtension(resourceName, extension);

			if (!wantedExtension.IsEmpty() && extension != wantedExtension)
				continue;

			if (resourceName.EndsWith(BASE_PREFAB_SUFFIX + ".et")) // avoid updating BASE prefabs
				continue;

			resourceNameLC = FilePath.StripPath(resourceNameLC);
			resourceNameLC.ToLower();

			if (SCR_StringHelper.ContainsEvery(resourceNameLC, keywords))
				result.Insert(resourceName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the created entities' entitySources
	protected array<IEntitySource> CreateEntitySourcesFromBoneNames(notnull map<string, ResourceName> prefabMap, notnull IEntitySource entitySource, bool addToParent)
	{
		if (prefabMap.IsEmpty())
			return {};

		IEntity entity = m_API.SourceToEntity(entitySource);
		if (!entity)
		{
			Print("null entity?!", LogLevel.WARNING);
			return {};
		}

		array<string> boneNames = {};
		entity.GetAnimation().GetBoneNames(boneNames);
		boneNames.RemoveItem("Scene_Root");

		IEntitySource parent;
		if (addToParent)
			parent = entitySource;

		array<IEntitySource> newPrefabSources = {};
		foreach (string boneName : boneNames)
		{
			IEntitySource bonePrefabEntitySource = CreateEntitySourceFromBoneName(prefabMap, boneName, parent);
			if (bonePrefabEntitySource)
				newPrefabSources.Insert(bonePrefabEntitySource);
			else
				Print("Bone " + boneName + " could not see a matching prefab created", LogLevel.VERBOSE);
		}

		return newPrefabSources;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] parent can be null
	// - if so, the IEntitySource will be created from Resource, no IEntity will be created ← not a Doxygen comment -yet- for a reason
	//! \return the created entity's entitySource
	protected IEntitySource CreateEntitySourceFromBoneName(notnull map<string, ResourceName> prefabMap, string boneName, IEntitySource parent)
	{
		if (boneName.IsEmpty())
			return null;

		string prefab = FindAdaptedPrefab(prefabMap, boneName);
		if (prefab.IsEmpty())
		{
			Print("No prefab could be found matching bone " + boneName, LogLevel.VERBOSE);
			return null;
		}

		IEntitySource result = m_API.CreateEntity(prefab, "", m_API.GetCurrentEntityLayerId(), parent, vector.Zero, vector.Zero);
		if (!result)
		{
			Print("Entity " + prefab + " could not be created for bone " + boneName, LogLevel.WARNING);
			return null;
		}

		IEntityComponentSource hierarchyComponent = SCR_BaseContainerTools.FindComponentSource(result, HIERARCHY_CLASSNAME);
		if (!hierarchyComponent) // do not create the component, directly throw an error
		{
			Print("No Hierarchy component - fix by adding a " + HIERARCHY_CLASSNAME + " component to " + prefab + " first", LogLevel.WARNING);
			SCR_WorldEditorToolHelper.DeleteEntityFromSource(result);
			return null;
		}

		m_API.SetVariableValue(result, { new ContainerIdPathEntry(HIERARCHY_CLASSNAME) }, "PivotID", boneName);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the created entities' entitySources
	protected array<ref Resource> CreateResourcesFromBoneNames(notnull map<string, ResourceName> prefabMap, notnull IEntitySource entitySource)
	{
		if (prefabMap.IsEmpty())
			return {};

		IEntity entity = m_API.SourceToEntity(entitySource);
		if (!entity)
		{
			Print("null entity?!", LogLevel.WARNING);
			return {};
		}

		array<string> boneNames = GetBoneNames(entity);

		array<ref Resource> result = {};
		Resource bonePrefabResource;
		foreach (string boneName : boneNames)
		{
			bonePrefabResource = CreateResourceFromBoneName(prefabMap, boneName);
			if (bonePrefabResource)
				result.Insert(bonePrefabResource);
			else
				Print("Bone " + boneName + " could not see a matching prefab created", LogLevel.VERBOSE);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the created entity's Resource (for ref reasons)
	protected Resource CreateResourceFromBoneName(notnull map<string, ResourceName> prefabMap, string boneName)
	{
		if (boneName.IsEmpty())
			return null;

		string prefab = FindAdaptedPrefab(prefabMap, boneName);
		if (prefab.IsEmpty())
		{
			Print("No prefab could be found matching bone " + boneName, LogLevel.VERBOSE);
			return null;
		}

		Resource result = BaseContainerTools.CreateContainer(GENERICENTITY_CLASSNAME);
		if (!result || !result.IsValid())
		{
			Print("Could not create a new " + GENERICENTITY_CLASSNAME + " for boneName", LogLevel.WARNING);
			return null;
		}

		IEntitySource resultSource = result.GetResource().ToEntitySource();
		resultSource.SetAncestor(prefab);
		IEntityComponentSource hierarchyComponent = SCR_BaseContainerTools.FindComponentSource(resultSource, HIERARCHY_CLASSNAME);
		if (!hierarchyComponent) // do not create the component, directly throw an error
		{
			Print("No Hierarchy component - fix by adding a " + HIERARCHY_CLASSNAME + " component to " + prefab + " first", LogLevel.WARNING);
			return null;
		}
		hierarchyComponent.Set("PivotID", boneName);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get useful bone names without e.g Scene_Root or COM_
	protected array<string> GetBoneNames(notnull IEntity entity)
	{
		array<string> result = {};
		entity.GetAnimation().GetBoneNames(result);
		for (int i = result.Count() - 1; i >= 0; i--)
		{
			string boneName = result[i];
			// ditch Scene_Root, Center Of Mass bones, non-WxH bones
			if (boneName == "Scene_Root" || boneName.StartsWith("COM_") || !SCR_StringHelper.ContainsEvery(boneName, BONE_CRITERIA))
				result.Remove(i); // order does not matter here
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Find the prefab adapted to the provided bone name
	//! \return the prefab's ResourceName, string.Empty if not found
	protected ResourceName FindAdaptedPrefab(notnull map<string, ResourceName> prefabMap, string boneName)
	{
		if (boneName.IsEmpty())
			return string.Empty;

		boneName.ToLower();
		foreach (string key, ResourceName value : prefabMap)
		{
			if (boneName.Contains(key))
			{
				Print("Found " + key + " for bone " + boneName, LogLevel.DEBUG);
				return value;
			}
		}

		Print("Did not find prefab for bone " + boneName, LogLevel.DEBUG);
		return string.Empty;
	}
}
#endif // WORKBENCH
