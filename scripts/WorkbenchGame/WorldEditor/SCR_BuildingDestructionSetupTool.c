#ifdef WORKBENCH
// TODO: use BaseContainer and remove direct property methods?
// doing so will force tool's config -file- usage
[WorkbenchToolAttribute(
	name: "Prefab Destruction Setup Tool",
	description: "Set building destruction effects",
	category: "Building Destruction",
	awesomeFontCode: 0xE50C)]
class SCR_BuildingDestructionSetupTool : WorldEditorTool
{
//	[Attribute(defvalue: "0", desc: "Automatically adds " + DESTRUCTION_COMPONENT + " if missing", category: "Fix Prefab Ruins")]
//	protected bool m_bAddMissingDestructibleBuildingComponent;

	[Attribute(category: "Set Building Destruction Effects")]
	protected ref SCR_BuildingDestructionSetupToolConfig m_DestructionEffectsConfig;

	protected static const string PREFAB_EXTENSION = "et";
	protected static const string BUILDING_CLASS = "SCR_DestructibleBuildingEntity";
	protected static const string RUIN_FILTER = "_Ruin"; // case-sensitive
	protected static const string FILE_NAME_FILTER[] = { "_Ruin", "_ruin", "_Ruins", "_ruins", "_Base", "_base", "_Dst_", "_dst_" }; // case-sensitive
	protected static const int FILE_NAME_FILTER_COUNT = 8;

	protected static const ResourceName DEFAULT_RUIN = "{11B789B5F476A4B5}Prefabs/Structures/Debris/DebrisPile_HouseVillage/DebrisPile_HouseVillage_01_Large.et";

	protected static const string COMPONENTS_ARRAY = "components";
	protected static const string DESTRUCTION_COMPONENT = "SCR_DestructibleBuildingComponent";
	protected static const string DESTRUCTION_COMPONENT_EFFECTS_ARRAY = "m_aEffects";

	protected static const string TIMED_PARTICLE_CLASSNAME = "SCR_TimedParticle";
	protected static const string TIMED_PARTICLE_PARTICLE = "m_Particle";
	protected static const string TIMED_PARTICLE_PARTICLE_CLASSNAME = "SCR_ParticleSpawnable";

	protected static const string TIMED_SOUND_CLASSNAME = "SCR_TimedSound";
	protected static const string TIMED_SOUND_AUDIO_CONFIG = "m_AudioSourceConfiguration";
	protected static const string TIMED_SOUND_AUDIO_CONFIG_CLASSNAME = "SCR_AudioSourceConfiguration";

	protected static const string TIMED_PREFAB_CLASSNAME = "SCR_TimedPrefab";
	protected static const string TIMED_PREFAB_PATH = "m_sRuinsPrefab";

	protected static const float LEVENSHTEIN_THRESHOLD = 0.75; // equal or above to this matching value, the ruin's file name will be accepted

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Fix Prefab Ruins")]
	protected void BtnFixBuildingPrefabRuins()
	{
		array<ResourceName> resourceNames = SCR_WorldEditorToolHelper.GetSelectedOrOpenedResources(PREFAB_EXTENSION);
		if (resourceNames.IsEmpty())
		{
			Print("No resources are selected, exiting", LogLevel.WARNING);
			return;
		}

		// process'em all
		FixBuildingPrefabRuins(resourceNames);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Set Building Destruction Effects")]
	protected void BtnSetBuildingDestructionEffects()
	{
		if (!m_DestructionEffectsConfig)
		{
			Print("Config (object) is not defined, exiting", LogLevel.WARNING);
			return;
		}

		if (!m_DestructionEffectsConfig.m_aEffects)
		{
			Print("Config array is null, exiting", LogLevel.WARNING);
			return;
		}

		array<ResourceName> resourceNames = SCR_WorldEditorToolHelper.GetSelectedOrOpenedResources(PREFAB_EXTENSION);
		if (resourceNames.IsEmpty())
		{
			Print("No resources are selected, exiting", LogLevel.WARNING);
			return;
		}

		// process'em all
		SetBuildingDestructionEffects(resourceNames, m_DestructionEffectsConfig.m_aEffects);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resourceNames
	protected void FixBuildingPrefabRuins(notnull array<ResourceName> resourceNames)
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return;
		}

		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		array<ResourceName> allRuinPrefabs = SCR_WorkbenchHelper.SearchWorkbenchResources({ PREFAB_EXTENSION }, { RUIN_FILTER });
		if (allRuinPrefabs.IsEmpty())
		{
			Print("No Ruins found - leaving", LogLevel.WARNING);
			return;
		}

		map<ResourceName, string> ruinPrefabFileNameMap = new map<ResourceName, string>();
		foreach (ResourceName ruinPrefab : allRuinPrefabs)
		{
			ruinPrefabFileNameMap.Insert(ruinPrefab, FilePath.StripPath(ruinPrefab.GetPath()));
		}

		IEntitySource entitySource;
		IEntitySource actualPrefab;

		int layerId = m_API.GetCurrentEntityLayerId();
		foreach (ResourceName resourceName : resourceNames)
		{
			if (allRuinPrefabs.Contains(resourceName))
			{
				Print("Skipping Ruin Prefab " + resourceName, LogLevel.NORMAL);
				continue;
			}

			bool skip;
			string fileName = FilePath.StripPath(resourceName.GetPath());
			for (int i; i < FILE_NAME_FILTER_COUNT; ++i)
			{
				if (fileName.Contains(FILE_NAME_FILTER[i]))
				{
					PrintFormat("Skipping \"%1\" Prefab %2", FILE_NAME_FILTER[i], resourceName, level: LogLevel.NORMAL);
					skip = true;
					break;
				}
			}

			if (skip)
				continue;

			entitySource = m_API.CreateEntity(resourceName, string.Empty, layerId, null, vector.Zero, vector.Zero);
			if (!entitySource)
			{
				Print("Invalid Prefab " + resourceName, LogLevel.WARNING);
				continue;
			}

			if (!entitySource.GetClassName().ToType() || !entitySource.GetClassName().ToType().IsInherited(SCR_DestructibleBuildingEntity))
			{
				Print("Prefab is not SCR_DestructibleBuildingEntity, skipping " + resourceName, LogLevel.NORMAL);
				SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
				continue;
			}

			actualPrefab = entitySource.GetAncestor();
			if (!actualPrefab)
			{
				Print("Prefab has no ancestor?? " + resourceName, LogLevel.WARNING);
				SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
				continue;
			}

			if (!FixPrefabRuin(actualPrefab, ruinPrefabFileNameMap, worldEditorAPI))
			{
				Print("Cannot fix ruin for " + resourceName, LogLevel.ERROR);
				SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
				continue;
			}

			if (SCR_PrefabHelper.UpdatePrefabFromEntitySourceAncestor(actualPrefab))
				Print("Successfully saved " + resourceName.GetPath(), LogLevel.NORMAL);
			else
				Print("Writing failure on " + resourceName.GetPath(), LogLevel.ERROR);

			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
		}

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] ancestor
	//! \param[in] ruinPrefabFileNameMap
	//! \param[in] worldEditorAPI
	protected bool FixPrefabRuin(notnull IEntitySource ancestor, notnull map<ResourceName, string> ruinPrefabFileNameMap, notnull WorldEditorAPI worldEditorAPI)
	{
		int componentIndex = -1;
		IEntityComponentSource componentSource;
		for (int i, count = ancestor.GetComponentCount(); i < count; ++i)
		{
			componentSource = ancestor.GetComponent(i);
			if (componentSource.GetClassName().ToType() &&
				componentSource.GetClassName().ToType().IsInherited(SCR_DestructibleBuildingComponent))
			{
				componentIndex = i;
				break;
			}
		}

		if (componentIndex < 0)
		{
//			if (m_bAddMissingDestructibleBuildingComponent)
//			{
//				componentIndex = ancestor.GetComponentCount();
//				componentSource = worldEditorAPI.CreateComponent(ancestor, DESTRUCTION_COMPONENT);
//				if (!componentSource)
//				{
//					Print("Failed to create " + DESTRUCTION_COMPONENT, LogLevel.WARNING);
//					return false;
//				}
//			}
//			else
//			{
				Print("No SCR_DestructibleBuildingComponent found", LogLevel.WARNING);
				return false;
//			}
		}

		BaseContainerList effects = componentSource.GetObjectArray(DESTRUCTION_COMPONENT_EFFECTS_ARRAY);
		if (!effects)
		{
			Print(DESTRUCTION_COMPONENT_EFFECTS_ARRAY + " not present", LogLevel.WARNING);
			return false;
		}

		int effectsCount = effects.Count();

		BaseContainer effect;
		ResourceName ruin;
		typename type;
		int effectIndex;
		bool found;
		for (; effectIndex < effectsCount; ++effectIndex)
		{
			effect = effects.Get(effectIndex);
			type = effect.GetClassName().ToType();
			if (!type || !type.IsInherited(SCR_TimedPrefab))
				continue;

			if (!effect.Get(TIMED_PREFAB_PATH, ruin)) // huh
				continue;

			if (ruin != DEFAULT_RUIN)
				return true;

//			if (effect.IsVariableSetDirectly(TIMED_PREFAB_PATH))
//				return true;

			found = true;
			break;
		}

		array<ref ContainerIdPathEntry> path = { new ContainerIdPathEntry(COMPONENTS_ARRAY, componentIndex) };
		if (!found)
		{
			effectIndex = effectsCount;
			if (!worldEditorAPI.CreateObjectArrayVariableMember(ancestor, path, DESTRUCTION_COMPONENT_EFFECTS_ARRAY, TIMED_PREFAB_CLASSNAME, effectIndex))
				return false;
		}

		path.Insert(new ContainerIdPathEntry(DESTRUCTION_COMPONENT_EFFECTS_ARRAY, effectIndex));

		string fileName = FilePath.StripPath(ancestor.GetResourceName().GetPath());
		ResourceName bestRuin = GetBestRuinByName(fileName, ruinPrefabFileNameMap);
		if (bestRuin) // !.IsEmpty()
			Print("RUIN FOUND: " + bestRuin, LogLevel.NORMAL);
		else
			Print("no best ruin found - best match = " + GetBestRuinScore(fileName, ruinPrefabFileNameMap), LogLevel.NORMAL);

		return false;

		return worldEditorAPI.SetVariableValue(ancestor, path, TIMED_PREFAB_PATH, bestRuin);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] fileName
	//! \param[in] ruinPrefabFileNameMap
	//! \param[in] threshold value above (or equal) which a name matching is accepted
	//! \return the best-matching ResourceName, empty if no good enough match was found
	protected ResourceName GetBestRuinByName(string fileName, notnull map<ResourceName, string> ruinPrefabFileNameMap)
	{
		float bestScore = -1;
		ResourceName bestResult;
		string fileNameWithoutExtension = FilePath.StripExtension(fileName);
		foreach (ResourceName ruinPrefab, string ruinFileName : ruinPrefabFileNameMap)
		{
			if (ruinFileName == string.Format("%1%2.%3", fileNameWithoutExtension, RUIN_FILTER, PREFAB_EXTENSION))
				return ruinPrefab;

			float score = SCR_StringHelper.GetLevenshteinDistanceScore(fileName, ruinFileName, false);
			if (score >= LEVENSHTEIN_THRESHOLD && score > bestScore)
			{
				bestScore = score;
				bestResult = ruinPrefab;
			}

			if (score >= 1)
				break;
		}

		return bestResult;
	}

	//------------------------------------------------------------------------------------------------
	// temp
	protected float GetBestRuinScore(string fileName, notnull map<ResourceName, string> ruinPrefabFileNameMap)
	{
		float bestScore;
		foreach (ResourceName ruinPrefab, string ruinFileName : ruinPrefabFileNameMap)
		{
			float score = SCR_StringHelper.GetLevenshteinDistanceScore(fileName, ruinFileName, false);
			if (bestScore < score)
				bestScore = score;
		}

		return bestScore;
	}

	//------------------------------------------------------------------------------------------------
	//! BeginEntityAction/EndEntityAction must be dealt with a level above
	//! \param[in] resourceNames
	//! \param[in] effects
	protected void SetBuildingDestructionEffects(notnull array<ResourceName> resourceNames, notnull array<ref SCR_TimedEffect> effects)
	{
		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		IEntitySource entitySource;
		BaseContainer actualPrefab;
		foreach (ResourceName resourceName : resourceNames)
		{
			entitySource = m_API.CreateEntity(resourceName, string.Empty, m_API.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);
			if (!entitySource)
			{
				Print(resourceName + " is not a valid Prefab", LogLevel.WARNING);
				continue;
			}

			actualPrefab = entitySource.GetAncestor();

			if (!SetEffectsValue(actualPrefab, effects))
			{
				Print("Could not apply Effects", LogLevel.ERROR);
				SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
				continue;
			}

			if (SCR_PrefabHelper.UpdatePrefabFromEntitySourceAncestor(actualPrefab))
				Print("Successfully saved " + resourceName.GetPath(), LogLevel.NORMAL);
			else
				Print("Writing failure on " + resourceName.GetPath(), LogLevel.ERROR);

			SCR_WorldEditorToolHelper.DeleteEntityFromSource(entitySource);
		}

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] actualPrefab
	//! \param[in] timedEffects
	//! \return
	protected bool SetEffectsValue(notnull BaseContainer actualPrefab, notnull array<ref SCR_TimedEffect> timedEffects)
	{
		array<ref ContainerIdPathEntry> destructionComponentPath = GetDestructionComponentPath(actualPrefab);
		if (!destructionComponentPath)
		{
			Print(DESTRUCTION_COMPONENT + " component is missing", LogLevel.ERROR);
			return false;
		}

		// TODO: fix that VM Exception if the array exists
		if (!m_API.SetVariableValue(actualPrefab, destructionComponentPath, DESTRUCTION_COMPONENT_EFFECTS_ARRAY, string.Empty))
		{
			Print("Could not clear variable value (1)", LogLevel.ERROR);
			return false;
		}

		if (!m_API.ClearVariableValue(actualPrefab, destructionComponentPath, DESTRUCTION_COMPONENT_EFFECTS_ARRAY))
		{
			Print("Could not clear variable value (2)", LogLevel.ERROR);
			return false;
		}

		if (timedEffects.IsEmpty())
			return true;

		// no need to create an array, using CreateObjectArrayVariableMember does it

		foreach (int effectIndex, SCR_TimedEffect timedEffect : timedEffects)
		{
			if (SCR_TimedParticle.Cast(timedEffect))
			{
				if (!InsertTimedParticleValue(actualPrefab, destructionComponentPath, effectIndex, SCR_TimedParticle.Cast(timedEffect)))
					return false;

				continue;
			}

			if (SCR_TimedSound.Cast(timedEffect))
			{
				if (!InsertTimedSoundValue(actualPrefab, destructionComponentPath, effectIndex, SCR_TimedSound.Cast(timedEffect)))
					return false;

				continue;
			}

			Print("Encountered an unsupported type: " + timedEffect.Type(), LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actualPrefab
	//! \param[in] destructionComponentPath
	//! \param[in] effectIndex
	//! \param[in] effect
	//! \return true on success, false otherwise
	protected bool InsertTimedParticleValue(
		notnull BaseContainer actualPrefab,
		notnull array<ref ContainerIdPathEntry> destructionComponentPath,
		int effectIndex,
		notnull SCR_TimedParticle effect)
	{
		if (!m_API.CreateObjectArrayVariableMember(
			actualPrefab,
			destructionComponentPath,
			DESTRUCTION_COMPONENT_EFFECTS_ARRAY,
			TIMED_PARTICLE_CLASSNAME,
			effectIndex))
		{
			Print("Could not create SCR_TimedParticle array item", LogLevel.ERROR);
			return false;
		}

		array<ref ContainerIdPathEntry> path = GetTimedEffectPath(destructionComponentPath, effectIndex);

		if (!InsertTimedEffectValue(actualPrefab, path, effect))
			return false;

		if (!m_API.SetVariableValue(actualPrefab, path, "m_fParticlesMultiplier", effect.GetParticlesMultiplier().ToString()))
			return false;

		// object must be the last change as 'path' gets changed
		SCR_ParticleSpawnable particle = effect.GetParticle();
		if (particle)
		{
			if (!m_API.CreateObjectVariableMember(actualPrefab, path, TIMED_PARTICLE_PARTICLE, TIMED_PARTICLE_PARTICLE_CLASSNAME))
				return false;

			path.Insert(new ContainerIdPathEntry(TIMED_PARTICLE_PARTICLE));

			array<vector> positionAndRotation = {};
			particle.GetPositionAndRotation(positionAndRotation);

			if (!m_API.SetVariableValue(actualPrefab, path, "m_vOffsetPosition", positionAndRotation[0].ToString(false)))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_vOffsetRotation", positionAndRotation[1].ToString(false)))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_Particle", particle.m_Particle))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_bAtCenter", particle.m_bAtCenter.ToString(true)))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_bDirectional", particle.m_bDirectional.ToString(true)))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] actualPrefab
	//! \param[in] destructionComponentPath
	//! \param[in] effectIndex
	//! \param[in] effect
	//! \return true on success, false otherwise
	protected bool InsertTimedSoundValue(
		notnull BaseContainer actualPrefab,
		notnull array<ref ContainerIdPathEntry> destructionComponentPath,
		int effectIndex,
		notnull SCR_TimedSound effect)
	{
		if (!m_API.CreateObjectArrayVariableMember(
			actualPrefab,
			destructionComponentPath,
			DESTRUCTION_COMPONENT_EFFECTS_ARRAY,
			TIMED_SOUND_CLASSNAME,
			effectIndex))
		{
			Print("Could not create SCR_TimedSound array item", LogLevel.ERROR);
			return false;
		}

		array<ref ContainerIdPathEntry> path = GetTimedEffectPath(destructionComponentPath, effectIndex);

		if (!InsertTimedEffectValue(actualPrefab, path, effect))
			return false;

		// object must be the last change as 'path' gets changed
		SCR_AudioSourceConfiguration audioSourceConfig = effect.GetAudioSourceConfiguration();
		if (audioSourceConfig)
		{
			if (!m_API.CreateObjectVariableMember(actualPrefab, path, TIMED_SOUND_AUDIO_CONFIG, TIMED_SOUND_AUDIO_CONFIG_CLASSNAME))
				return false;

			path.Insert(new ContainerIdPathEntry(TIMED_SOUND_AUDIO_CONFIG));

			if (!m_API.SetVariableValue(actualPrefab, path, "m_sSoundProject", audioSourceConfig.m_sSoundProject))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_sSoundEventName", audioSourceConfig.m_sSoundEventName))
				return false;

			if (!m_API.SetVariableValue(actualPrefab, path, "m_eFlags", audioSourceConfig.m_eFlags.ToString()))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! called by
	//! - InsertTimedParticleValue
	//! - InsertTimedSoundValue
	//! \param[in] actualPrefab
	//! \param[in] path
	//! \param[in] effect
	//! \return
	protected bool InsertTimedEffectValue(
		notnull BaseContainer actualPrefab,
		notnull array<ref ContainerIdPathEntry> path,
		notnull SCR_TimedEffect effect)
	{
		if (!m_API.SetVariableValue(actualPrefab, path, "m_fSpawnTime", effect.m_fSpawnTime.ToString()))
			return false;

		if (!m_API.SetVariableValue(actualPrefab, path, "m_bSnapToTerrain", effect.m_bSnapToTerrain.ToString(true)))
			return false;

		if (!m_API.SetVariableValue(actualPrefab, path, "m_bAttachToParent", effect.m_bAttachToParent.ToString(true)))
			return false;

		if (!m_API.SetVariableValue(actualPrefab, path, "m_bPersistent", effect.m_bPersistent.ToString(true)))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] actualPrefab
	//! \return
	protected static array<ref ContainerIdPathEntry> GetDestructionComponentPath(BaseContainer actualPrefab)
	{
		int destructionComponentIndex = SCR_BaseContainerTools.FindComponentIndex(actualPrefab, DESTRUCTION_COMPONENT);
		if (destructionComponentIndex < 0)
			return null;

		return { new ContainerIdPathEntry(COMPONENTS_ARRAY, destructionComponentIndex) };
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] destructionComponentPath
	//! \param[in] effectIndex
	//! \return a copy of destructionComponentPath with the timed effect path entry inserted, never returns null
	protected static array<ref ContainerIdPathEntry> GetTimedEffectPath(notnull array<ref ContainerIdPathEntry> destructionComponentPath, int effectIndex)
	{
		array<ref ContainerIdPathEntry> result = SCR_ArrayHelperRefT<ContainerIdPathEntry>.GetCopy(destructionComponentPath);
		result.Insert(new ContainerIdPathEntry(DESTRUCTION_COMPONENT_EFFECTS_ARRAY, effectIndex));
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] containerIdPathEntry
	//! \return
	protected static string ToString(notnull ContainerIdPathEntry containerIdPathEntry)
	{
		return containerIdPathEntry.PropertyName + "/" + containerIdPathEntry.Index;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] containerIdPathEntries
	//! \return
	protected static string ToString(notnull array<ref ContainerIdPathEntry> containerIdPathEntries)
	{
		array<string> bits = {};
		foreach (ContainerIdPathEntry pathEntry : containerIdPathEntries)
		{
			bits.Insert(ToString(pathEntry));
		}

		return "(array<ContainerIdPathEntry>) " + SCR_StringHelper.Join(", ", bits);
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_BuildingDestructionSetupToolConfig
{
	[Attribute(desc: "Timed Effects to be applied to provided building(s)")]
	ref array<ref SCR_TimedEffect> m_aEffects;
}
#endif
