// #define DEBUG_TEVR_DUPLICATION

#ifdef WORKBENCH
//! CLI usage:
//! - -tevrWorld	the absolute .ent file of the original world to convert
//! example: -tevrWorld="C:\Reforger\Data\worlds\Arland\Arland.ent"
//! - -tevrVariant	(case-insensitive) the wanted variant - can be one of SCR_ETerrainEntityVariant values
//! example: -tevrVariant=autumn
//! - -tevrConfig	the conversion config ResourceName (preferred) or filePath, SCR_TerrainEntityVariantConfig config class
//! example: -tevrConfig="{E10A32BEF1F9E157}Configs/Workbench/WorldEditor/TerrainEntityVariantReplacementTool/Autumn.conf"
//! \code
//! ArmaReforgerWorkbench.exe -gproj="C:\Reforger\Data\ArmaReforger.gproj" -exitAfterInit -wbModule=WorldEditor -plugin=SCR_TerrainEntityVariantReplacementPlugin -tevrWorld="C:\A3.9R\ARData\worlds\Arland\Arland.ent" -tevrVariant=autumn -tevrConfig="{E10A32BEF1F9E157}Configs/Workbench/WorldEditor/TerrainEntityVariantReplacementTool/Autumn.conf"
//! \endcode
[WorkbenchPluginAttribute(
	name: "Terrain Entity Variant Replacement",
	description: "Convert a terrain to a variant with conversion config",
	wbModules: { "WorldEditor" },
	category: "World Setup",
	awesomeFontCode: 0xF06C)]
class SCR_TerrainEntityVariantReplacementPlugin : WorkbenchPlugin
{
	/*
		Debug
	*/

	[Attribute(category: "Debug", defvalue: "0", desc: "Read Only - all calculations are done but no edits happen")]
	protected bool m_bReadOnly;

	/*
		Replacement
	*/

	[Attribute(category: "Replacement", defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Only replace entities in the current layer", enums: SCR_ParamEnumArray.FromString("All entities;Current layer;Selected entities"))]
	protected int m_iReplacedEntities;

	[Attribute(category: "Replacement", defvalue: SET_ANCESTOR_FORCE_RELOAD.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Entity replacement mode - various modes, same (visual) results", enums: SCR_ParamEnumArray.FromString("Delete/create entity (SLOW - 2x);SetAncestor + visual refresh (slow - 1x);SetAncestor (fast - 0.01x - almost instant but requires world reload);SetAncestor + Save/Load world;SetAncestor + save and force reload"))]
	protected int m_iReplacementMode;

	[Attribute(category: "Replacement", defvalue: SCR_ETerrainEntityVariant.AUTUMN.ToString(), desc: "Expected terrain season (DEFAULT = reset XOB only)", uiwidget: UIWidgets.ComboBox, enumType: SCR_ETerrainEntityVariant)]
	protected SCR_ETerrainEntityVariant m_eWantedVariant;

	[Attribute(category: "Replacement", desc: "Config object - takes precedence over Config File if defined")]
	protected ref SCR_TerrainEntityVariantConfig m_Config;

	[Attribute(category: "Replacement", defvalue: "{E10A32BEF1F9E157}Configs/Workbench/WorldEditor/TerrainEntityVariantReplacementTool/Autumn.conf", desc: "Config file - if Config above is defined, this field is ignored", params: "conf class=SCR_TerrainEntityVariantConfig")]
	protected ResourceName m_sConfigFile;

#ifdef DEBUG_TEVR_DUPLICATION
	[Attribute(category: "Duplication Debug", params: "ent")]
	protected ResourceName m_sTerrainToDuplicate;

	[Attribute(category: "Duplication Debug", defvalue: SCR_ETerrainEntityVariant.AUTUMN.ToString(), uiwidget: UIWidgets.ComboBox, enumType: SCR_ETerrainEntityVariant)]
	protected SCR_ETerrainEntityVariant m_eDuplicationVariant;

	[Attribute(category: "Duplication Debug", defvalue: "1")]
	protected bool m_bDuplicationDebug;
#else // let's not lose Workbench-stored user values
	[Attribute(uiwidget: UIWidgets.None)]
	protected ResourceName m_sTerrainToDuplicate;

	[Attribute(uiwidget: UIWidgets.None)]
	protected SCR_ETerrainEntityVariant m_eDuplicationVariant;

	[Attribute(uiwidget: UIWidgets.None)]
	protected bool m_bDuplicationDebug;
#endif

	protected ref map<ResourceName, ResourceName> m_mReplacementMap = new map<ResourceName, ResourceName>();

	protected bool m_bIsUI;

	protected static const int ALL_ENTITIES = 0;
	protected static const int CURRENT_LAYER_MODE = 1;
	protected static const int SELECTED_ENTITIES_MODE = 2;

	protected static const int CREATE_AND_DELETE = 0;
	protected static const int SET_ANCESTOR_REFRESH = 1;
	protected static const int SET_ANCESTOR = 2;
	protected static const int SET_ANCESTOR_SAVELOAD = 3;
	protected static const int SET_ANCESTOR_FORCE_RELOAD = 4;

	protected static const string PARAM_WORLD = "tevrWorld";
	protected static const string PARAM_VARIANT = "tevrVariant";
	protected static const string PARAM_CONFIG = "tevrConfig";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		m_bIsUI = true;

		if (Workbench.ScriptDialog("Terrain Entity Variant Replacement", "", this) == 0)
			return;

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();
		if (!worldEditorAPI)
			return;

		if (worldEditorAPI.IsGameMode())
		{
			Print("In Game mode", LogLevel.ERROR);
			return;
		}

		if (worldEditorAPI.IsPrefabEditMode())
		{
			Print("In Prefab edit mode", LogLevel.ERROR);
			return;
		}

		if (!m_Config || !LoadConfig(m_Config, m_eWantedVariant))
		{
			if (!LoadConfig(m_sConfigFile, m_eWantedVariant))
				return;
		}

#ifdef DEBUG_TEVR_DUPLICATION
		if (m_sTerrainToDuplicate)
		{
			DebugDuplication();
			return;
		}
#endif

		if (!ConvertCurrentTerrain(m_iReplacedEntities, m_iReplacementMode))
			return;

		if (m_iReplacementMode == SET_ANCESTOR_REFRESH || m_iReplacementMode == SET_ANCESTOR)
			return;

		if (m_iReplacementMode != SET_ANCESTOR_FORCE_RELOAD && Workbench.ScriptDialog("Save/Load operation", "SetAncestor requires a terrain save/load. Proceed?", new SCR_OKCancelWorkbenchDialog()) == 0)
			return;

		if (!worldEditor.Save())
		{
			Print("World Editor cannot save the current world - please proceed manually", LogLevel.ERROR);
			Workbench.Dialog("Error saving world", "World Editor cannot save the current world - please proceed manually");
			return;
		}

		string worldPath;
		worldEditorAPI.GetWorldPath(worldPath);

		if (!worldPath || !worldEditor.SetOpenedResource(worldPath))
		{
			Print("World Editor cannot load the world - please proceed manually", LogLevel.ERROR);
			Workbench.Dialog("Error loading world", "World Editor cannot load the saved world - please proceed manually");
			return;
		}

		// success
	}

#ifdef DEBUG_TEVR_DUPLICATION
	//------------------------------------------------------------------------------------------------
	protected void DebugDuplication()
	{
		string terrainFilePath;
		if (!Workbench.GetAbsolutePath(m_sTerrainToDuplicate.GetPath(), terrainFilePath, true))
		{
			Print("Cannot get world path for " + m_sTerrainToDuplicate, LogLevel.ERROR);
			return;
		}

		const string newTerrainFilePath = GetNewWorldFilePath(terrainFilePath, m_eDuplicationVariant);

		if (m_bDuplicationDebug)
		{
			Print("target      = " + terrainFilePath, LogLevel.NORMAL);
			Print("destination = " + newTerrainFilePath, LogLevel.NORMAL);
			return;
		}

		if (!SCR_WorldFilesHelper.DuplicateWorld(terrainFilePath, newTerrainFilePath))
		{
			Print("Duplication error", LogLevel.ERROR);
			return;
		}

		Print("Duplication success", LogLevel.NORMAL);
	}
#endif

	//------------------------------------------------------------------------------------------------
	protected override void RunCommandline()
	{
		m_bIsUI = false;

		Print("[SCR_TerrainEntityVariantReplacementPlugin.RunCommandline] STARTED", level: LogLevel.NORMAL);

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);

		string terrainFilePath;
		SCR_ETerrainEntityVariant variant;
		ResourceName configResourceName;
		if (!GetCLIParameterValues(worldEditor, terrainFilePath, variant, configResourceName))
			return;

		PrintFormat("Converting %1 terrain to %2 variant using config %3", terrainFilePath, variant, configResourceName, level: LogLevel.NORMAL);

		if (!LoadConfig(configResourceName, variant))
			return;

		Print("Conversion config loaded", LogLevel.NORMAL);

		string newTerrainFilePath = GetNewWorldFilePath(terrainFilePath, variant);

		Print("Terrain duplication destination: " + newTerrainFilePath, LogLevel.NORMAL);

		if (!SCR_WorldFilesHelper.DuplicateWorld(terrainFilePath, newTerrainFilePath))
		{
			PrintFormat("Cannot duplicate world from %1 to %2", terrainFilePath, newTerrainFilePath, level: LogLevel.ERROR);
			return;
		}

		Print("Terrain duplication OK", LogLevel.NORMAL);

		if (!worldEditor.SetOpenedResource(newTerrainFilePath))
		{
			PrintFormat("Cannot open duplicated terrain in World Editor (%1)", newTerrainFilePath, level: LogLevel.ERROR);
			return;
		}

		Print("Duplicated terrain loading OK", LogLevel.NORMAL);

		if (!ConvertCurrentTerrain())
		{
			Print("Error during terrain conversion", LogLevel.ERROR);
			return;
		}

		if (!worldEditor.Save())
		{
			PrintFormat("Error saving duplicated terrain (%1)", newTerrainFilePath, level: LogLevel.ERROR);
			return;
		}

		Print("Converted terrain successfully saved - " + newTerrainFilePath, LogLevel.NORMAL);
		Print("[SCR_TerrainEntityVariantReplacementPlugin.RunCommandline] FINISHED", level: LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	// Show all errors at once to save time
	protected bool GetCLIParameterValues(notnull WorldEditor worldEditor, out string worldPath, out SCR_ETerrainEntityVariant variant, out ResourceName config)
	{
		bool result = true;

		string error;

		worldPath = GetCLIWorldPath(worldEditor, error);
		if (error)
		{
			Print(error, LogLevel.ERROR);
			error = string.Empty;
			result = false;
		}

		variant = GetCLIVariant(worldEditor, error);
		if (error)
		{
			Print(error, LogLevel.ERROR);
			error = string.Empty;
			result = false;
		}

		config = GetCLIConfig(worldEditor, config);
		if (error)
		{
			Print(error, LogLevel.ERROR);
			error = string.Empty;
			result = false;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected string GetCLIWorldPath(notnull WorldEditor worldEditor, out string error)
	{
		string result;

		if (!worldEditor.GetCmdLine("-" + PARAM_WORLD, result))
		{
			error = string.Format("-%1 not provided", PARAM_WORLD);
			return string.Empty;
		}

		if (!result.EndsWith(".ent") || !FileIO.FileExists(result))
		{
			error = string.Format("-%1 value is not a valid file (%2)", PARAM_WORLD, result);
			return string.Empty;
		}

		error = string.Empty;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ETerrainEntityVariant GetCLIVariant(notnull WorldEditor worldEditor, out string error)
	{
		SCR_ETerrainEntityVariant result;

		string variantStr;
		if (!worldEditor.GetCmdLine("-" + PARAM_VARIANT, variantStr))
		{
			error = string.Format("-%1 not provided - see SCR_ETerrainEntityVariant for possible values", PARAM_VARIANT);
			return -1;
		}

		variantStr.ToUpper(); // let's be kind
		result = typename.StringToEnum(SCR_ETerrainEntityVariant, variantStr);

		if (result < 0)
		{
			error = string.Format("-%1 value is invalid - see SCR_ETerrainEntityVariant for possible values (provided %2)", PARAM_VARIANT, variantStr);
			return -1;
		}

		error = string.Empty;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName GetCLIConfig(notnull WorldEditor worldEditor, out string error)
	{
		string config;
		if (!worldEditor.GetCmdLine("-" + PARAM_CONFIG, config))
		{
			error = string.Format("-%1 not provided - config must be of type SCR_TerrainEntityVariantConfig", PARAM_CONFIG);
			return ResourceName.Empty;
		}

		if (!config) // .IsEmpty()
		{
			error = string.Format("-%1 is empty - config must be of type SCR_TerrainEntityVariantConfig", PARAM_CONFIG);
			return ResourceName.Empty;
		}

		ResourceName result;
		if (config.StartsWith("{")) // }
		{
			result = config;
		}
		else
		{
			if (!Workbench.GetAbsolutePath(config, config, true) || !FileIO.FileExists(config))
			{
				error = string.Format("-%1 provided file does not exist (%2)", PARAM_CONFIG, config);
				return ResourceName.Empty;
			}

			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			if (!resourceManager)
			{
				error = "Cannot obtain Resource Manager";
				return ResourceName.Empty;
			}

			MetaFile metaFile = resourceManager.GetMetaFile(config);
			if (!metaFile)
			{
				error = string.Format("-%1 provided file is not registered (%2)", PARAM_CONFIG, config);
				return ResourceName.Empty;
			}

			result = metaFile.GetResourceID();
		}

		if (!Resource.Load(result).IsValid())
		{
			error = string.Format("-%1 is not a valid/registered config (%2)", PARAM_CONFIG, config);
			return ResourceName.Empty;
		}

		error = string.Empty;
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadConfig(ResourceName configResourceName, SCR_ETerrainEntityVariant variant)
	{
		if (!configResourceName) // .IsEmpty()
		{
			Print("Provided config ResourceName is empty", LogLevel.ERROR);
			return false;
		}

		Managed managedInstance = SCR_BaseContainerTools.CreateInstanceFromPrefab(configResourceName, true);
		if (!managedInstance)
		{
			Print("Provided config cannot be loaded - " + configResourceName, LogLevel.ERROR);
			return false;
		}

		SCR_TerrainEntityVariantConfig config = SCR_TerrainEntityVariantConfig.Cast(managedInstance);
		if (!config)
		{
			PrintFormat("Config is not of type %1 - %2", SCR_TerrainEntityVariantConfig, configResourceName, level: LogLevel.ERROR);
			return false;
		}

		return LoadConfig(config, variant);
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadConfig(notnull SCR_TerrainEntityVariantConfig config, SCR_ETerrainEntityVariant variant)
	{
		if (config.m_aEntries.IsEmpty())
		{
			Print("Empty config provided", LogLevel.ERROR);
			return false;
		}

		m_mReplacementMap.Clear();

		foreach (SCR_TerrainEntityVariantConfigEntry configEntry : config.m_aEntries)
		{
			if (m_mReplacementMap.Contains(configEntry.m_sResourceName))
			{
				Print("Config definition duplicate of " + configEntry.m_sResourceName, LogLevel.WARNING);
				continue;
			}

			foreach (SCR_TerrainEntityVariantConfigValue variantValue : configEntry.m_aVariants)
			{
				if (variantValue.m_eVariant == variant) // we have a winner
				{
					m_mReplacementMap.Insert(configEntry.m_sResourceName, variantValue.m_sResourceName);
					break;
				}
			}
		}

		if (m_mReplacementMap.IsEmpty())
		{
			Print("Conversion config is empty", LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ConvertCurrentTerrain(int replacedEntities = ALL_ENTITIES, int replacementMode = SET_ANCESTOR)
	{
		Debug.BeginTimeMeasure();
		bool result = ConvertCurrentTerrainMeasured(replacedEntities, replacementMode);
		Debug.EndTimeMeasure("Total conversion time");

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true on success, false otherwise - currently always returns true (imagine the code quality©!)
	protected bool ConvertCurrentTerrainMeasured(int replacedEntities, int replacementMode)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		Debug.BeginTimeMeasure();
		array<IEntitySource> entitiesToProcess = GetentitiesToProcess(replacedEntities);
		Debug.EndTimeMeasure("Get entities to process");

		int entitiesToProcessCount = entitiesToProcess.Count();
		if (entitiesToProcessCount < 1)
		{
			Print("No terrain entities were found to be replaced", LogLevel.NORMAL);
			if (m_bIsUI)
				Workbench.Dialog("No entities found to replace", "No terrain entities were found to be replaced");

			return true;
		}

		if (replacementMode == CREATE_AND_DELETE)
		{
			Debug.BeginTimeMeasure();
			array<ref SCR_TEVR_Data> creationDataList = GetCreationData(entitiesToProcess);
			Debug.EndTimeMeasure(entitiesToProcessCount.ToString() + " entities replacement data creation");

			if (m_bReadOnly)
			{
				Print("Read Only mode - no terrain entities were modified", LogLevel.NORMAL);
				if (m_bIsUI)
					Workbench.Dialog("Read Only mode", "No terrain entities were modified");

				return true;
			}

			worldEditorAPI.BeginEntityAction();

			Debug.BeginTimeMeasure();
			DeleteEntities(worldEditorAPI, entitiesToProcess);
			Debug.EndTimeMeasure(entitiesToProcessCount.ToString() + " entities deletion");

			Debug.BeginTimeMeasure();
			array<IEntitySource> createdEntitySources = CreateEntities(creationDataList);
			Debug.EndTimeMeasure(string.Format("%1/%2 entities (re-)creation", createdEntitySources.Count(), entitiesToProcessCount));

			worldEditorAPI.EndEntityAction();
		}
		else // set ancestor (w/ or w/o refresh)
		{
			if (m_bReadOnly)
			{
				Print("Read Only mode - no terrain entities were modified", LogLevel.NORMAL);
				if (m_bIsUI)
					Workbench.Dialog("Read Only mode", "No terrain entities were modified");

				return true;
			}

			worldEditorAPI.BeginEntityAction();

			Debug.BeginTimeMeasure();
			SetAncestor(entitiesToProcess, replacementMode == 1);
			Debug.EndTimeMeasure(string.Format("%1/%1 entities re-ancestored", entitiesToProcessCount));

			worldEditorAPI.EndEntityAction();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntitySource> GetentitiesToProcess(int replacedEntities)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		array<IEntitySource> result = {};

		int currentLayerID;
		if (replacedEntities == CURRENT_LAYER_MODE)
			currentLayerID = worldEditorAPI.GetCurrentEntityLayerId();

		IEntitySource entitySource;
		IEntitySource ancestor;
		WBProgressDialog progress;
		int count = worldEditorAPI.GetEditorEntityCount();
		float prevProgress, currProgress;
		if (m_bIsUI)
			progress = new WBProgressDialog("Finding entities to replace...", worldEditor);

		for (int i; i < count; ++i)
		{
			if (m_bIsUI)
			{
				currProgress = i / count;
				if (currProgress - prevProgress >= 0.01)	// min 1%
				{
					progress.SetProgress(currProgress);		// expensive
					prevProgress = currProgress;
				}
			}

			entitySource = worldEditorAPI.GetEditorEntity(i);

			if (replacedEntities == SELECTED_ENTITIES_MODE && !worldEditorAPI.IsEntitySelected(entitySource))
				continue;

			if (!entitySource)
				continue; // huh?

			int entitySourceLayerID = entitySource.GetLayerID();
			if (replacedEntities == CURRENT_LAYER_MODE && entitySourceLayerID != currentLayerID)
				continue;

			ancestor = entitySource.GetAncestor(); // a raw GenericEntity, not a Prefab
			if (!ancestor)
				continue;

			if (!ancestor.GetResourceName()) // IsEmpty()
				continue;

			if (!m_mReplacementMap.Contains(ancestor.GetResourceName()))
				continue;

			result.Insert(entitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected array<ref SCR_TEVR_Data> GetCreationData(notnull array<IEntitySource> entitySources)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		array<ref SCR_TEVR_Data> result = {};

		SCR_TEVR_Data creationData;
		foreach (IEntitySource entitySource : entitySources)
		{
			vector coords;
			if (!entitySource.Get("coords", coords))
			{
				Print("Cannot get coords from entity", LogLevel.WARNING);
				continue;
			}

			vector angles;
			if (!entitySource.Get("angles", angles))
			{
				Print("Cannot get angleX/Y/Z from entity", LogLevel.WARNING);
				continue;
			}

			float scale;
			if (!entitySource.Get("scale", scale))
			{
				Print("Cannot get scale from entity", LogLevel.WARNING);
				continue;
			}

			EntityFlags flags;
			if (!entitySource.Get("Flags", flags))
			{
				Print("Cannot get Flags from entity", LogLevel.WARNING);
				continue;
			}

			creationData = new SCR_TEVR_Data();

			creationData.m_sResourceName = m_mReplacementMap[entitySource.GetAncestor().GetResourceName()];
			if (!creationData.m_sResourceName)
				continue;

			creationData.m_Parent = entitySource.GetParent();
			creationData.m_iLayerID = entitySource.GetLayerID();
			creationData.m_vCoords = coords;
			creationData.m_vAngles = angles;
			creationData.m_fScale = scale;
			creationData.m_eFlags = flags;

			creationData.m_bSelected = worldEditorAPI.IsEntitySelected(entitySource);

			result.Insert(creationData);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// wrap method for Tracy
	protected void DeleteEntities(notnull WorldEditorAPI worldEditorAPI, notnull array<IEntitySource> entitySources)
	{
		worldEditorAPI.DeleteEntities(entitySources);
	}

	//------------------------------------------------------------------------------------------------
	protected array<IEntitySource> CreateEntities(notnull array<ref SCR_TEVR_Data> creationDataList)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		array<IEntitySource> result = {};

		IEntitySource createdSource;
		foreach (SCR_TEVR_Data creationData : creationDataList)
		{
			createdSource = worldEditorAPI.CreateEntity(
				creationData.m_sResourceName,
				"",
				creationData.m_iLayerID,
				creationData.m_Parent,
				creationData.m_vCoords,
				creationData.m_vAngles);
			if (!createdSource)
			{
				Print("cannot create Prefab " + creationData.m_sResourceName, LogLevel.ERROR);
				continue;
			}

			float scale;
			if (createdSource.Get("scale", scale) && scale != creationData.m_fScale)
			{
				if (!worldEditorAPI.SetVariableValue(createdSource, null, "scale", creationData.m_fScale.ToString()))
					Print("Cannot set scale to newly created entity", LogLevel.WARNING);
			}

			EntityFlags flags;
			if (createdSource.Get("Flags", flags) && flags != creationData.m_eFlags)
			{
				if (!worldEditorAPI.SetVariableValue(createdSource, null, "Flags", creationData.m_eFlags.ToString()))
					Print("Cannot set Flags to newly created entity", LogLevel.WARNING);
			}

			if (creationData.m_bSelected)
				worldEditorAPI.AddToEntitySelection(createdSource);

			result.Insert(createdSource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetAncestor(notnull array<IEntitySource> entitySources, bool forceRefresh)
	{
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		WorldEditorAPI worldEditorAPI = worldEditor.GetApi();

		set<int> updatedLayers;
		if (!forceRefresh)
			updatedLayers = new set<int>();

		int entityLayerID;
		vector coords;
		vector angles;

		WBProgressDialog progress;
		int count;
		float prevProgress, currProgress;
		if (m_bIsUI)
		{
			if (forceRefresh)
				progress = new WBProgressDialog("Setting entity ancestors with visual refresh", worldEditor);
			else
				progress = new WBProgressDialog("Setting entity ancestors w/o visual refresh", worldEditor);

			count = entitySources.Count();
		}

		foreach (int i, IEntitySource entitySource : entitySources)
		{
			// one SHOULD normally use the World Editor API for entity source operations
			// unfortunately there is no API equivalent to SetAncestor
			entitySource.SetAncestor(m_mReplacementMap[entitySource.GetAncestor().GetResourceName()]);

			entityLayerID = entitySource.GetLayerID();
			if (forceRefresh || !updatedLayers.Contains(entityLayerID))
			{
				// force visual update / force layer save
				if (
					!entitySource.Get("coords", coords)
					|| !worldEditorAPI.SetVariableValue(entitySource, null, "coords", string.Format("%1 %2 %3", coords[0], coords[1], coords[2])))
				{
					Print("Cannot refresh layer using coords!", LogLevel.WARNING);
				}
				else
				{
					if (!forceRefresh)
						updatedLayers.Insert(entityLayerID);
				}
			}

			if (m_bIsUI)
			{
				currProgress = i / count;
				if (currProgress - prevProgress >= 0.01)	// min 1%
				{
					progress.SetProgress(currProgress);		// expensive
					prevProgress = currProgress;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] terrainFilePath the source world's file path
	//! \param[in] variant the terrain variant's enum value, added lowercase after WorldName_ (after the underscore) e.g "autumn"
	//! \return the new world's file path
	static string GetNewWorldFilePath(string terrainFilePath, SCR_ETerrainEntityVariant variant)
	{
		string variantStr = typename.EnumToString(SCR_ETerrainEntityVariant, variant);
		variantStr.ToLower();

		terrainFilePath.Replace(SCR_StringHelper.ANTISLASH, SCR_StringHelper.SLASH); // cleaner
		string newTerrainFilePath = FilePath.StripFileName(terrainFilePath);
		if (newTerrainFilePath) // !.IsEmpty()
		{
			newTerrainFilePath = newTerrainFilePath.Substring(0, newTerrainFilePath.Length() - 1); // slash or antislash
			if (newTerrainFilePath.EndsWith(":")) // Windows's drive format (C:)
				newTerrainFilePath += SCR_StringHelper.SLASH;
			else
				newTerrainFilePath = string.Format("%1_%2/", newTerrainFilePath, variantStr);
		}

		return string.Format("%1%2_%3.ent", newTerrainFilePath, FilePath.StripExtension(FilePath.StripPath(terrainFilePath)), variantStr);

	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected int ButtonOK()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

class SCR_TEVR_Data
{
	ResourceName m_sResourceName;
	IEntitySource m_Parent;
	int m_iLayerID;
	vector m_vCoords;
	vector m_vAngles;
	float m_fScale;
	EntityFlags m_eFlags;
	bool m_bSelected;
}
#endif

[BaseContainerProps(configRoot: true)]
class SCR_TerrainEntityVariantConfig
{
	[Attribute(desc: "If multiple entries are of the same variant, only the first one is used")]
	ref array<ref SCR_TerrainEntityVariantConfigEntry> m_aEntries;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_sResourceName", true)]
class SCR_TerrainEntityVariantConfigEntry
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	ResourceName m_sResourceName;

	[Attribute(desc: "If multiple entries are of the same variant, only the first one is used")]
	ref array<ref SCR_TerrainEntityVariantConfigValue> m_aVariants;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETerrainEntityVariant, "m_eVariant")]
class SCR_TerrainEntityVariantConfigValue
{
	[Attribute(defvalue: SCR_ETerrainEntityVariant.AUTUMN.ToString(), desc: "Variant type", uiwidget: UIWidgets.ComboBox, enums: SCR_ParamEnumArray.FromEnumSkip(SCR_ETerrainEntityVariant, 1))]
	SCR_ETerrainEntityVariant m_eVariant;

	[Attribute(desc: "The wanted resource name\n- XOB for a model swap\n- Prefab for a Prefab swap\n- left empty for entity deletion", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et xob")]
	ResourceName m_sResourceName;
}

[EnumLinear()]
enum SCR_ETerrainEntityVariant
{
	DEFAULT,

	// seasons
	SPRING,
	SUMMER,
	AUTUMN, // not FALL
	WINTER,

	// settings
	// EASTERN_EUROPE,
	// MEDITERRANEAN,

	// states
	// FLOODED,
	// DESTROYED,
	// NUCLEAR_APOCALYPSE,
}
