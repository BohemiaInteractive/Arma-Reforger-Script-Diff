#ifdef WORKBENCH
//! Plugin that allows to reset all/selected entities' desired flags to Prefab's default
//! WARNING: can take up to serveral minutes depending on what is selected!
[WorkbenchPluginAttribute(
	name: "Entity Flags Manager",
	description: "Reset all/selected entities' desired flags to Prefab's default",
	shortcut: "Ctrl+Shift+F",
	wbModules: { "WorldEditor" })]
class SCR_EntityFlagsManagerPlugin : WorldEditorPlugin
{
	/*
		Category: Flags
	*/

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetTraceableFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetVisibleFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetStaticFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetFeatureFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetNoLinkFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetProxyFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetEditorOnlyFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetDisabledFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetRelativeYFlag;

	[Attribute(defvalue: "0", category: "Flags")]
	protected bool m_bResetOnFilteredNavmeshFlag;

	/*
		Category: Entities
	*/

	[Attribute(defvalue: "0", category: "Entities")]
	protected bool m_bProcessEverythingSelected;
/*
	[Attribute(defvalue: "1", category: "Entities")]
	protected bool m_bCurrentLayerOnly;
*/
	[Attribute(defvalue: "1", category: "Entities")]
	protected bool m_bProcessRocks;

	[Attribute(defvalue: "1", category: "Entities")]
	protected bool m_bProcessTrees;

	[Attribute(defvalue: "1", category: "Entities")]
	protected bool m_bProcessBushes;

	[Attribute(defvalue: "1", category: "Entities")]
	protected bool m_bProcessBuildings;
/*
	[Attribute(category: "Entities")]
	protected array<ResourceName> m_aOtherTopMostPrefabs;
*/
	/*
		Category: Options
	*/

	[Attribute(defvalue: "0", desc: "[COSTY] Process child entities - may be problematic with e.g forest generators", category: "Options")]
	protected bool m_bUpdateChildEntities;

	[Attribute(defvalue: "0", desc: "Force flags cleaning - slower but sure to remove unneeded flags", category: "Options")]
	protected bool m_bForceCleaning;

	protected static const int PROGRESSBAR_TIMEOUT = 3000; //!< in milliseconds - when does the progressbar display
	protected static const int PROGRESSBAR_REFRESH = 1000; //!< in milliseconds - when does the progressbar refresh
	protected static const string COORDS = "coords";
	protected static const string FLAGS = "Flags";
	protected static const ResourceName ROCK_BASE = "{B94DDC61F0B7F9D7}Prefabs/Rocks/Rock_Base.et";
	protected static const ResourceName TREE_BASE = "{388AE316D09D0680}Prefabs/Vegetation/Core/Tree_Base.et";
	protected static const ResourceName BUSH_BASE = "{D7163D1B571F4C0C}Prefabs/Vegetation/Core/Bush_Base.et";
	protected static const ResourceName BUILDING_BASE = "{A43A100E3C377DB2}Prefabs/Structures/Core/Building_Base.et";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
		{
			Print("World Editor API is not available", LogLevel.ERROR);
			return;
		}

		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();
		string message = "Entities currently selected: " + selectedCount;

		if (selectedCount < 1)
		{
			m_bProcessEverythingSelected = false; // safety
			message += "\n\n[ WARNING: you are about to process ALL TERRAIN ENTITIES ]";
		}

		// filter
		if (!Workbench.ScriptDialog("Entity Flags Manager", message, this))
			return;

		if (!(
				m_bResetTraceableFlag ||
				m_bResetVisibleFlag ||
				m_bResetStaticFlag ||
				m_bResetFeatureFlag ||
				m_bResetNoLinkFlag ||
				m_bResetProxyFlag ||
				m_bResetEditorOnlyFlag ||
				m_bResetDisabledFlag ||
				m_bResetRelativeYFlag ||
				m_bResetOnFilteredNavmeshFlag
		))
		{
			Print("No Flags were selected for reset; leaving", LogLevel.WARNING);
			return;
		}

		// prepare
		array<ResourceName> validTopMostAncestors = {};
		if (!m_bProcessEverythingSelected)
		{
			if (m_bProcessRocks)
				validTopMostAncestors.Insert(ROCK_BASE);

			if (m_bProcessTrees)
				validTopMostAncestors.Insert(TREE_BASE);

			if (m_bProcessBushes)
				validTopMostAncestors.Insert(BUSH_BASE);

			if (m_bProcessBuildings)
				validTopMostAncestors.Insert(BUILDING_BASE);

			if (validTopMostAncestors.IsEmpty())
			{
				Print("No Entity Types have been selected; leaving", LogLevel.WARNING);
				return;
			}
		}

		// get entities
		array<IEntitySource> entitySources = GetEntitySources(validTopMostAncestors);

		int entitySourcesCount = entitySources.Count();
		if (entitySourcesCount < 1)
		{
			Print("No entities found; exiting", LogLevel.NORMAL);
			return;
		}

		// process
		ProcessEntitySources(entitySources);

		// leave!
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessEntitySources(array<IEntitySource> entitySources)
	{
		int entitySourcesCount = entitySources.Count();

		int lastRefresh = System.GetTickCount();
		WBProgressDialog progress;

		vector entityPos;
		int clearedFlags, editedFlags, fixed;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		Debug.BeginTimeMeasure();
		worldEditorAPI.BeginEntityAction();

		foreach (int i, IEntitySource entitySource : entitySources)
		{
			if (!entitySource.GetAncestor())
				continue;

			EntityFlags oldFlags;
			if (!entitySource.Get(FLAGS, oldFlags))
			{
				Print("Could not get flags from entitySource", LogLevel.WARNING);
				continue;
			}

			EntityFlags prefabFlags;
			if (!entitySource.GetAncestor().Get(FLAGS, prefabFlags))
			{
				Print("Could not get flags from Prefab", LogLevel.WARNING);
				continue;
			}

			EntityFlags newFlags = oldFlags;

			bool useEditSequence = m_bResetRelativeYFlag; // due to flag + pos
			if (useEditSequence)
				worldEditorAPI.BeginEditSequence(entitySource);

			/*
				FLAGS
			*/

			if (m_bResetTraceableFlag)
			{
				if (prefabFlags & EntityFlags.TRACEABLE)
					newFlags |= EntityFlags.TRACEABLE;
				else
					newFlags &= ~EntityFlags.TRACEABLE;
			}

			if (m_bResetVisibleFlag)
			{
				if (prefabFlags & EntityFlags.VISIBLE)
					newFlags |= EntityFlags.VISIBLE;
				else
					newFlags &= ~EntityFlags.VISIBLE;
			}

			if (m_bResetStaticFlag)
			{
				if (prefabFlags & EntityFlags.STATIC)
					newFlags |= EntityFlags.STATIC;
				else
					newFlags &= ~EntityFlags.STATIC;
			}

			if (m_bResetFeatureFlag)
			{
				if (prefabFlags & EntityFlags.FEATURE)
					newFlags |= EntityFlags.FEATURE;
				else
					newFlags &= ~EntityFlags.FEATURE;
			}

			if (m_bResetNoLinkFlag)
			{
				if (prefabFlags & EntityFlags.NO_LINK)
					newFlags |= EntityFlags.NO_LINK;
				else
					newFlags &= ~EntityFlags.NO_LINK;
			}

			if (m_bResetProxyFlag)
			{
				if (prefabFlags & EntityFlags.PROXY)
					newFlags |= EntityFlags.PROXY;
				else
					newFlags &= ~EntityFlags.PROXY;
			}

			if (m_bResetEditorOnlyFlag)
			{
				if (prefabFlags & EntityFlags.EDITOR_ONLY)
					newFlags |= EntityFlags.EDITOR_ONLY;
				else
					newFlags &= ~EntityFlags.EDITOR_ONLY;
			}

			if (m_bResetDisabledFlag)
			{
				if (prefabFlags & EntityFlags.DISABLED)
					newFlags |= EntityFlags.DISABLED;
				else
					newFlags &= ~EntityFlags.DISABLED;
			}

			if (m_bResetRelativeYFlag)
			{
				if (prefabFlags & EntityFlags.RELATIVE_Y)
					newFlags |= EntityFlags.RELATIVE_Y;
				else
					newFlags &= ~EntityFlags.RELATIVE_Y;

				if (oldFlags & EntityFlags.RELATIVE_Y)			// if currently ATL
				{
					if (!(newFlags & EntityFlags.RELATIVE_Y))	// to World
					{
						// ATL to World
						float y;
						entitySource.Get(COORDS, entityPos);
						if (worldEditorAPI.TryGetTerrainSurfaceY(entityPos[0], entityPos[2], y))
							entityPos[1] = entityPos[1] + y;
						else
							Print("Entity " + worldEditorAPI.SourceToEntity(entitySource).GetID() + " position cannot be changed (ATL to World)", LogLevel.WARNING);
					}
					else
					{
						entityPos = vector.Zero;
					}
				}
				else											// if currently World
				{
					if (newFlags & EntityFlags.RELATIVE_Y)		// to ATL
					{
						// World to ATL
						float y;
						entitySource.Get(COORDS, entityPos);
						if (worldEditorAPI.TryGetTerrainSurfaceY(entityPos[0], entityPos[2], y))
							entityPos[1] = entityPos[1] - y;
						else
							Print("Entity " + worldEditorAPI.SourceToEntity(entitySource).GetID() + " position cannot be changed (World to ATL)", LogLevel.WARNING);
					}
					else
					{
						entityPos = vector.Zero;
					}
				}
			}

			if (m_bResetOnFilteredNavmeshFlag)
			{
				if (prefabFlags & EntityFlags.USER4) // OnFilteredNavmesh
					newFlags |= EntityFlags.USER4;
				else
					newFlags &= ~EntityFlags.USER4;
			}

			/*
				/FLAGS
			*/

			// identical
			if (!m_bForceCleaning && oldFlags == newFlags && newFlags == prefabFlags)
				continue;

			if (newFlags == prefabFlags)
			{
				worldEditorAPI.ClearVariableValue(entitySource, null, FLAGS);
				clearedFlags++;
			}
			else
			if (newFlags != oldFlags)
			{
				worldEditorAPI.SetVariableValue(entitySource, null, FLAGS, newFlags.ToString());
				editedFlags++;
			}

			// relativeY aftermath
			if (m_bResetRelativeYFlag && entityPos != vector.Zero)
				worldEditorAPI.SetVariableValue(entitySource, null, COORDS, entityPos.ToString(false));

			if (oldFlags != newFlags)
				fixed++;

			if (useEditSequence)	 // due to flag + pos
				worldEditorAPI.EndEditSequence(entitySource);

			int timeDiff = System.GetTickCount() - lastRefresh;
			if (!progress)
			{
				if (timeDiff > PROGRESSBAR_TIMEOUT)
				{
					progress = new WBProgressDialog("Processing " + entitySourcesCount + " entities...", Workbench.GetModule(WorldEditor));
					progress.SetProgress(i / entitySourcesCount);
				}
			}
			else
			if (timeDiff > PROGRESSBAR_REFRESH)
			{
				progress.SetProgress(i / entitySourcesCount);
				lastRefresh = System.GetTickCount();
			}
		}

		worldEditorAPI.EndEntityAction();
		Debug.EndTimeMeasure("Entity flag fixing");

		Print("Fixed " + fixed + "/" + entitySourcesCount + " entities (edited: " + editedFlags + " / cleared " + clearedFlags + ")", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] validAncestors - which topmost ancestors are accepted
	//! \param[out] originalCount - how many entities were selected/found
	//  \param[out] filteredCount - how many entities remain after filter < removed for now
	//! \return filtered entitySources
	protected array<IEntitySource> GetEntitySources(array<ResourceName> validAncestors)
	{
		int originalCount;
		array<IEntitySource> result = {};
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		int selectedCount = worldEditorAPI.GetSelectedEntitiesCount();

		Debug.BeginTimeMeasure();
		if (selectedCount > 0)
		{
			IEntitySource entitySource;
			originalCount = selectedCount;
			result.Reserve(selectedCount);
			for (int i = 0; i < selectedCount; i++)
			{
				entitySource = worldEditorAPI.GetSelectedEntity(i);
				
				if (!entitySource)
					continue;
				
				if (!m_bUpdateChildEntities && entitySource.GetParent())
					continue;

				if (m_bProcessEverythingSelected || validAncestors.Contains(SCR_BaseContainerTools.GetTopMostAncestor(entitySource).GetResourceName()))
					result.Insert(entitySource);
			}
		}
		else
		{
			int entitiesCount = worldEditorAPI.GetEditorEntityCount(); // maximum possible (almost)
			originalCount = entitiesCount;
			result.Reserve(entitiesCount);
			IEntitySource entitySource;
			for (int i = 0; i < entitiesCount; i++)
			{
				entitySource = worldEditorAPI.GetEditorEntity(i);
				if (!entitySource)
					continue;

				if (!m_bUpdateChildEntities && entitySource.GetParent())
					continue;

				// m_bProcessEverythingSelected must not be introduced here
				if (validAncestors.Contains(SCR_BaseContainerTools.GetTopMostAncestor(entitySource).GetResourceName()))
					result.Insert(entitySource);
			}
		}
		Debug.EndTimeMeasure("Filtering " + result.Count() + "/" + originalCount + " entities");

		return result;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected bool BtnOK()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool BtnCancel()
	{
		return false;
	}
}
#endif // WORKBENCH
