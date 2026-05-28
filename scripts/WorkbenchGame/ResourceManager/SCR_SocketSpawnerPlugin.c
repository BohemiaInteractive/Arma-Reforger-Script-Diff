#ifdef WORKBENCH
//! Socket Spawner - GUID-based Workbench plugin
//! Processes selected prefab (.et) assets from the Resource Browser.
//! Scans bones named like "socket_<GUID16>_<suffix>" and spawns children using ResourceName "{GUID}".
[WorkbenchPluginAttribute(name: "Socket Spawner", category: "Tools", description: "Spawn prefab children at GUID-based socket bones for selected .et prefabs", awesomeFontCode: 0xf1b3, wbModules: {"ResourceManager"})]
class SCR_SocketSpawnerPlugin : WorkbenchPlugin
{
	// Options
	[Attribute(defvalue: "0", UIWidgets.CheckBox, "Clear existing children of prefab (remove prefab members)")]
	protected bool m_bClearChildren;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, "Show detailed output")]
	protected bool m_bVerboseOutput;

	// Result tracking (integrated from SCR_SocketSpawnerResult)
	protected int m_iProcessedPrefabs;
	protected int m_iCreatedEntities;
	protected int m_iTotalSockets;
	protected ref array<string> m_aErrors;

	//------------------------------------------------------------------------------------------------
	//! Execute button handler - triggers the plugin execution
	[ButtonAttribute("Execute")]
	protected void Execute()
	{
		Run();
	}

	//------------------------------------------------------------------------------------------------
	//! Close button handler - closes the configuration dialog
	[ButtonAttribute("Close")]
	protected bool Close()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Display configuration dialog with plugin settings and usage instructions
	override void Configure()
	{
		if (Workbench.ScriptDialog(
			"Socket Spawner",
			"Process selected prefab (.et) assets from the Resource Browser.\n\n" +
			"Rules:\n" +
			"• Scans Animation bones with names starting 'socket_'\n" +
			"• Expected pattern: socket_<GUID16>_<suffix> e.g., socket_B57077CC7F07D01D_01\n" +
			"• Spawns a child entity using ResourceName \"{<GUID16>}\" at that bone's transform\n\n" +
			"Options:\n" +
			"• Clear existing children: remove all current prefab children before spawning\n" +
			"• Verbose output: print detailed progress to the console",
			this) == 0)
			return;
	}

	//------------------------------------------------------------------------------------------------
	//! Main execution method - processes selected prefabs and spawns entities at socket bones
	override void Run()
	{
		// Initialize result tracking
		m_iProcessedPrefabs = 0;
		m_iCreatedEntities = 0;
		m_iTotalSockets = 0;
		m_aErrors = {};

		array<ResourceName> selectedEtPrefabs = {};
		if (!ProcessSelectedPrefabs(selectedEtPrefabs))
		{
			ShowResults();
			return;
		}

		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
		{
			m_aErrors.Insert("Failed to get WorldEditor module");
			ShowResults();
			return;
		}

		foreach (ResourceName prefabRes : selectedEtPrefabs)
		{
			if (!ProcessSinglePrefab(prefabRes, worldEditor, m_bClearChildren))
				continue;

			m_iProcessedPrefabs++;
		}

		// Save changes to prefab files
		worldEditor.Save();

		ShowResults();
	}

	//------------------------------------------------------------------------------------------------
	//! Get selected resources from Resource Browser
	//! Can be overridden by inherited classes to use different selection methods
	//! \param selectedResources Output array to receive selected resource names
	protected void GetSelectedResources(out array<ResourceName> selectedResources)
	{
		selectedResources = {};

		// Get ResourceManager module
		ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
		if (!resourceManager)
			return;

		// Get list of currently selected resources
		resourceManager.GetResourceBrowserSelection(selectedResources.Insert, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Collect selected resources from Resource Browser and filter to .et prefabs
	//! \param outPrefabs Output array to receive filtered prefab resource names
	//! \return True if at least one prefab is found, false otherwise
	protected bool ProcessSelectedPrefabs(out array<ResourceName> outPrefabs)
	{
		outPrefabs = {};

		// Get list of currently selected resources
		array<ResourceName> selectedResources = {};
		GetSelectedResources(selectedResources);

		if (selectedResources.IsEmpty())
		{
			m_aErrors.Insert("No resources selected in Resource Browser. Select one or more .et prefabs.");
			return false;
		}

		// Filter for .et files (prefabs)
		foreach (ResourceName resourceName : selectedResources)
		{
			string resourcePath = resourceName.GetPath();
			if (resourcePath.EndsWith(".et"))
			{
				outPrefabs.Insert(resourceName);
			}
		}

		if (outPrefabs.IsEmpty())
		{
			m_aErrors.Insert("No .et prefab files in selection.");
			return false;
		}

		if (m_bVerboseOutput)
			PrintFormat("SocketSpawner: %1 prefab(s) queued from selection.", outPrefabs.Count());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Process a single prefab: create temp entity, optionally clear children, scan bones, spawn children
	//! \param prefabResource The prefab resource to process
	//! \param worldEditor WorldEditor module instance
	//! \param clearExistingChildren If true, removes existing prefab children before spawning new ones
	//! \return True if processing succeeded, false on error
	protected bool ProcessSinglePrefab(ResourceName prefabResource, WorldEditor worldEditor, bool clearExistingChildren)
	{
		if (prefabResource.IsEmpty())
		{
			m_aErrors.Insert("Invalid prefab resource");
			return false;
		}

		// Load prefab resource
		Resource resource = Resource.Load(prefabResource);
		if ( !resource.IsValid())
		{
			m_aErrors.Insert(string.Format("Failed to load prefab: %1", prefabResource));
			return false;
		}

		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
		{
			m_aErrors.Insert("Failed to get WorldEditor API");
			return false;
		}

		// Create a working entity source from the prefab
		api.BeginEntityAction();
		IEntitySource entitySource = api.CreateEntity(prefabResource, "", api.GetCurrentEntityLayerId(), null, vector.Zero, vector.Zero);

		// Optionally clear existing children (remove prefab members from ancestor)
		array<IEntitySource> childEntitySources;
		if (clearExistingChildren && entitySource && entitySource.GetNumChildren() > 0)
		{
			IEntitySource actualPrefab = IEntitySource.Cast(entitySource.GetAncestor());
			childEntitySources = {};
			for (int i, count = entitySource.GetNumChildren(); i < count; i++)
			{
				childEntitySources.Insert(actualPrefab.GetChild(i));
			}

			if (!childEntitySources.IsEmpty()) // only call if non-empty
				api.RemovePrefabMembers(childEntitySources);
		}
		api.EndEntityAction();

		if (!entitySource)
		{
			m_aErrors.Insert(string.Format("Failed to get entity source for: %1", prefabResource));
			return false;
		}

		// Build bone map
		map<string, ref array<vector>> boneMatrixMap = new map<string, ref array<vector>>();
		if (!RefreshBoneMatrixMap(entitySource, boneMatrixMap, api))
		{
			if (m_bVerboseOutput)
				PrintFormat("No bones found in prefab: %1", prefabResource);
			// Cleanup entity source
			api.BeginEntityAction();
			api.DeleteEntities({entitySource});
			api.EndEntityAction();
			return true; // not an error
		}

		// Iterate bones; look for "socket_" prefix and GUID pattern
		int createdInThisPrefab = 0;
		int validSocketCount = 0;

		for (int boneIndex = 0, boneCount = boneMatrixMap.Count(); boneIndex < boneCount; boneIndex++)
		{
			string boneName = boneMatrixMap.GetKey(boneIndex);

			// Skip common root
			if (boneName == "Scene_Root")
				continue;

			if (!boneName.StartsWith("socket_"))
				continue;

			string guid16;
			string suffix;
			if (!ExtractGuidFromBoneName(boneName, guid16, suffix))
			{
				// Warn when verbose only
				if (m_bVerboseOutput)
					PrintFormat("SocketSpawner: Bone '%1' ignored - GUID16 not found.", boneName, level: LogLevel.WARNING);
				continue;
			}

			validSocketCount++;

			array<vector> matrices = boneMatrixMap.GetElement(boneIndex);
			vector position = "0 0 0";
			if (matrices && matrices.Count() >= 4)
				position = matrices[3];

			if (CreateEntityAtBone(boneName, guid16, position, matrices, worldEditor, entitySource))
			{
				createdInThisPrefab++;
				m_iCreatedEntities++;
			}
		}

		m_iTotalSockets += validSocketCount;

		if (m_bVerboseOutput && createdInThisPrefab > 0)
			PrintFormat("SocketSpawner: Created %1 entities in prefab %2", createdInThisPrefab, prefabResource);

		// Cleanup working entity
		api.BeginEntityAction();
		api.DeleteEntities({entitySource});
		api.EndEntityAction();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Build a map of bone names to their transformation matrices from entity animation data
	//! Based on RefreshBoneMatrixMap pattern used in SCR_PrefabSocketProcessorPlugin.c
	//! \param entitySource The entity source to extract bone data from
	//! \param boneMatrixMap Output map of bone names to 4x4 transformation matrices (as array of 4 vectors)
	//! \param api WorldEditorAPI instance for entity operations
	//! \return True if at least one bone was found, false otherwise
	protected bool RefreshBoneMatrixMap(notnull IEntitySource entitySource, notnull out map<string, ref array<vector>> boneMatrixMap, WorldEditorAPI api)
	{
		boneMatrixMap.Clear();

		api.BeginEntityAction();

		IEntity entity = api.SourceToEntity(entitySource);
		if (!entity)
		{
			if (m_bVerboseOutput)
				PrintFormat("Could not resolve entity for: %1", entitySource.GetResourceName());
			api.EndEntityAction();
			return false;
		}

		Animation animation = entity.GetAnimation();
		if (animation)
		{
			array<string> boneNames = {};
			animation.GetBoneNames(boneNames);
			vector mat[4];

			foreach (string boneName : boneNames)
			{
				TNodeId boneIndex = animation.GetBoneIndex(boneName);
				if (!animation.GetBoneMatrix(boneIndex, mat))
				{
					if (m_bVerboseOutput)
						Print("Cannot get bone " + boneName + "'s matrix", LogLevel.WARNING);
					continue;
				}

				array<vector> matrixArray = { mat[0], mat[1], mat[2], mat[3] };
				boneMatrixMap.Insert(boneName, matrixArray);
			}
		}

		api.EndEntityAction();

		return boneMatrixMap.Count() > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Extract GUID16 from bone name pattern: "socket_<GUID16>_<suffix>" (case-insensitive)
	//! \param boneName The bone name to parse
	//! \param guid16 Output: extracted GUID as uppercase hex string (16 characters)
	//! \param suffix Output: optional suffix after the GUID (text after second underscore)
	//! \return True if valid GUID16 pattern found, false otherwise
	protected bool ExtractGuidFromBoneName(string boneName, out string guid16, out string suffix)
	{
		guid16 = "";
		suffix = "";

		if (!boneName)
			return false;

		// Convert to lowercase for case-insensitive matching
		string lowerBoneName = boneName;
		lowerBoneName.ToLower();

		// Quick prefix check (case-insensitive)
		if (!lowerBoneName.StartsWith("socket_"))
			return false;

		// Position after "socket_" (7 characters)
		const int start = 7;

		// Ensure at least 16 characters are available for GUID
		if (lowerBoneName.Length() < start + 16)
			return false;

		// Extract exactly 16 characters
		string candidate = lowerBoneName.Substring(start, 16);

		// Validate all characters are hex using lookup
		static const string HEX_CHARS = "0123456789abcdef";
		for (int i = 0; i < 16; i++)
		{
			if (HEX_CHARS.IndexOf(candidate.Get(i)) == -1)
				return false;
		}

		// Convert to uppercase
		candidate.ToUpper();
		guid16 = candidate;

		// Extract suffix after GUID (if underscore exists at position start + 16)
		int suffixStart = start + 16;
		if (suffixStart < boneName.Length() && boneName.Get(suffixStart) == "_")
			suffix = boneName.Substring(suffixStart + 1, boneName.Length() - (suffixStart + 1));
		else
			suffix = "";

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create a child entity at a bone location using GUID-based ResourceName "{GUID16}"
	//! \param boneName Name of the socket bone (for logging)
	//! \param guid16 16-character GUID hex string to construct resource name
	//! \param position World position for the new entity
	//! \param boneMatrix 4x4 transformation matrix from bone (used for rotation)
	//! \param worldEditor WorldEditor module instance
	//! \param parentEntitySource Parent entity source to attach the new entity to
	//! \return True if entity created successfully, false on error
	protected bool CreateEntityAtBone(string boneName, string guid16, vector position, array<vector> boneMatrix, WorldEditor worldEditor, IEntitySource parentEntitySource)
	{
		WorldEditorAPI api = worldEditor.GetApi();
		if (!api)
		{
			m_aErrors.Insert("Failed to get WorldEditor API");
			return false;
		}

		// Compose resource name like "{B57077CC7F07D01D}"
		ResourceName childRes = string.Format("{%1}", guid16);

		// Create entity at position; orientation from bone matrix
		api.BeginEntityAction();

		vector zeroAngles = "0 0 0";
		IEntitySource newEntitySource = api.CreateEntity(childRes, "", api.GetCurrentEntityLayerId(), null, position, zeroAngles);
		if (!newEntitySource)
		{
			api.EndEntityAction();
			m_aErrors.Insert(string.Format("Failed to create entity for socket %1 using resource %2", boneName, childRes));
			return false;
		}

		// Extract rotation from bone matrix (right, up, dir) then to angles
		vector mat3[3] = {
			boneMatrix[0],
			boneMatrix[1],
			boneMatrix[2]
		};
		vector angles = Math3D.MatrixToAngles(mat3);

		api.SetVariableValue(newEntitySource, null, "coords", position.ToString(false));
		// Note: same Euler order as reference patterns
		api.SetVariableValue(newEntitySource, null, "angles", string.Format("%1 %2 %3", angles[1], angles[0], angles[2]));

		// Attach under prefab ancestor as a child
		if (parentEntitySource && parentEntitySource.GetAncestor())
		{
			array<IEntitySource> toMove = { newEntitySource };
			api.MoveEntitiesToPrefab(parentEntitySource, parentEntitySource.GetAncestor(), toMove);
		}

		api.EndEntityAction();

		if (m_bVerboseOutput)
			PrintFormat("SocketSpawner: Created child at '%1' using %2 at %3", boneName, childRes, position);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Display results summary in console and dialog box
	protected void ShowResults()
	{
		string summary = string.Format("Processed %1 prefabs, created %2 entities from %3 socket(s)",
			m_iProcessedPrefabs, m_iCreatedEntities, m_iTotalSockets);

		if (!m_aErrors.IsEmpty())
		{
			summary += "\nErrors:\n";
			foreach (string e : m_aErrors)
				summary += "- " + e + "\n";
		}

		SCR_WorkbenchHelper.PrintDialog(summary, "Socket Spawner Results", LogLevel.NORMAL);
	}
}

//------------------------------------------------------------------------------------------------
//! World Editor version of Socket Spawner Plugin
//! Uses WorldEditor API for resource selection instead of ResourceManager
//------------------------------------------------------------------------------------------------
[WorkbenchPluginAttribute(name: "Socket Spawner", category: "Tools", description: "Spawn prefab children at GUID-based socket bones for selected .et prefabs", awesomeFontCode: 0xf1b3, wbModules: {"WorldEditor"})]
class SCR_SocketSpawnerPluginWE : SCR_SocketSpawnerPlugin
{
	//------------------------------------------------------------------------------------------------
	//! Override to use WorldEditor API for resource selection instead of ResourceManager
	//! \param selectedResources Output array to receive selected resource names
	override protected void GetSelectedResources(out array<ResourceName> selectedResources)
	{
		selectedResources = {};

		// Get WorldEditor module
		WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
		if (!worldEditor)
			return;

		// Get list of currently selected resources using WorldEditor API
		worldEditor.GetResourceBrowserSelection(selectedResources.Insert, true);
	}
}
#endif