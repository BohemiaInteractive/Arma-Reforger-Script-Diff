#ifdef WORKBENCH
// Only SlotAI, SlotKill and SlotDefend have WaypointSets

//! This plugin converts old scenarios using Scenario Framework from 1.0.0 to the new 1.1.0 format.\n
//! The plugin must be run once, then the world must be saved and loaded before running part 2.
//! \see SCR_ScenarioFrameworkConversionPlugin_Phase2
[WorkbenchPluginAttribute(
	name: "Scenario Framework 1.0.0 -> 1.1.0 Update - phase 1",
	shortcut: "", // "Ctrl+Alt+Z",
	category: "Update/1.0.0 to 1.1.0",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0x0031, // 0xE54E, // F4D7? meh, more like pathing
	description: "This plugin updates Scenario Framework Slots from the old (1.0.0) to the new (1.1.0+) standard")]
class SCR_ScenarioFrameworkConversionPlugin_Phase1 : WorkbenchPlugin
{
	static const string GENERIC_CLASSNAME = "GenericEntity";

	// Layer
	static const string SLOT_WAYPOINT_PREFAB = "{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et";
	static const string SLOT_WAYPOINT_COMPONENT_CLASSNAME = "SCR_ScenarioFrameworkSlotWaypoint";
	protected static const string LAYER_PREFAB = "{5F9FFF4BF027B3A3}Prefabs/ScenarioFramework/Components/Layer.et";
	protected static const string CYCLE_WAYPOINT_CLASSNAME = "SCR_ScenarioFrameworkWaypointCycle";

	// Base Slot
	protected static const string BASE_SLOT_CLASSNAME = "SCR_ScenarioFrameworkSlotBase";							// for EXACT match
	protected static const string SLOT_PREFAB = "{AA01691FDC4E9167}Prefabs/ScenarioFramework/Components/Slot.et";	// isKindOf
	protected static const string BASE_SLOT_FIELD = "m_sObjectToSpawn";												// for EXACT match

	// Waypoint Slots
	protected static const string SLOT_AI_PREFAB = "{8D43830F02C3F114}Prefabs/ScenarioFramework/Components/SlotAI.et";				// isKindOf
	protected static const string SLOT_KILL_PREFAB = "{C70DC6CBD1AAEC9A}Prefabs/ScenarioFramework/Components/SlotKill.et";			// isKindOf
	protected static const string SLOT_DEFEND_PREFAB = "{E123BAC59A9B3D5F}Prefabs/ScenarioFramework/Components/SlotDefend.et";		// isKindOf
	protected static const ref array<string> BASE_SLOTWP_CLASSNAMES = {						// for EXACT match
		"SCR_ScenarioFrameworkSlotAI",
		"SCR_ScenarioFrameworkSlotKill",
		"SCR_ScenarioFrameworkSlotDefend",
	};
	protected static const string BASE_SLOTWP_FIELD_FROM = "m_aWaypointGroupNames";
	protected static const string BASE_SLOTWP_FIELD_TO = "m_WaypointSet";
			static const string WAYPOINT_COMPONENT_FIELD = "m_Waypoint";
	protected static const string WAYPOINT_CLASSNAME = "SCR_ScenarioFrameworkWaypointSet";
	protected static const string CYCLE_SUFFIX = "_Cycle";

	protected static const ref array<ref SCR_ScenarioFrameworkConversionPlugin_Rune> CONVERSION_TABLE = {};

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		// TODO: optimise by obtaining all layers + all slots in one go
		Debug.BeginTimeMeasure();
		array<IEntitySource> allEntitySources = GetEntitySources();
		Debug.EndTimeMeasure("Entities obtention (" + allEntitySources.Count() + " found)");

		Debug.BeginTimeMeasure();
		CreateCycleWaypoints(allEntitySources);
		Debug.EndTimeMeasure("Cycle waypoints conversion");

		Debug.BeginTimeMeasure();
		Convert(allEntitySources);
		Debug.EndTimeMeasure("" + allEntitySources.Count() + " entities conversion");

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] getAllEntitiesAndNotJustSelectedOnes
	//! \return entitysources that are GenericEntity and have an ancestor
	protected array<IEntitySource> GetEntitySources()
	{
		IEntitySource entitySource;
		array<IEntitySource> result = {};
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		for (int i, count = worldEditorAPI.GetEditorEntityCount(); i < count; i++)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!entitySource)
				continue;

			if (!entitySource.GetAncestor())
				continue;

			if (entitySource.GetClassName() != GENERIC_CLASSNAME)
				continue;

			result.Insert(entitySource);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Create cycle waypoints under targeted layers (with the name LayerName_Cycle)
	//! \param[in] allEntitySources
	protected void CreateCycleWaypoints(notnull array<IEntitySource> allEntitySources)
	{
		map<string, IEntitySource> namedLayers = new map<string, IEntitySource>();
		array<ref SCR_WaypointSet> cycleWaypointSets = {};

		GetLayersAndWaypointSets(allEntitySources, namedLayers, cycleWaypointSets);

		if (namedLayers.IsEmpty())
			return;

		array<ref ContainerIdPathEntry> waypointPath = { new ContainerIdPathEntry(SLOT_WAYPOINT_COMPONENT_CLASSNAME) };
		array<ref ContainerIdPathEntry> randomOrderBoolPath = {
			new ContainerIdPathEntry(SLOT_WAYPOINT_COMPONENT_CLASSNAME),
			new ContainerIdPathEntry(WAYPOINT_COMPONENT_FIELD),
		};
		IEntitySource namedLayer;
		IEntitySource entitySource;
		IEntity entity;
		IEntityComponentSource componentSource;

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		foreach (SCR_WaypointSet cycleWaypointSet : cycleWaypointSets)
		{
			namedLayer = namedLayers.Get(cycleWaypointSet.m_sName);
			if (!namedLayer)
			{
				Print("Cycle waypoint set is targeting an unknown layer " + cycleWaypointSet.m_sName, LogLevel.WARNING);
				continue;
			}

			string cycleWaypointEntityName = cycleWaypointSet.m_sName + CYCLE_SUFFIX;
			entitySource = worldEditorAPI.FindEntityByName(cycleWaypointEntityName);
			if (entitySource)
			{
				Print("Cycle waypoint already exists - " + cycleWaypointEntityName, LogLevel.WARNING);
				continue;
			}

			entitySource = worldEditorAPI.CreateEntity(SLOT_WAYPOINT_PREFAB, cycleWaypointEntityName, namedLayer.GetLayerID(), namedLayer, vector.Zero, vector.Zero);
			if (!entitySource)
			{
				Print("Failed to create cycle waypoint - " + cycleWaypointEntityName, LogLevel.WARNING);
				continue;
			}

			if (!worldEditorAPI.FindEntityByName(cycleWaypointEntityName)) // CreateEntity bug
			{
				if (!worldEditorAPI.RenameEntity(entitySource, cycleWaypointEntityName) || !worldEditorAPI.FindEntityByName(cycleWaypointEntityName))
				{
					Print("Could not name cycle waypoint entity properly - " + cycleWaypointEntityName, LogLevel.WARNING);
					worldEditorAPI.DeleteEntity(entitySource);
					continue;
				}
			}

			bool dealt;
			for (int i, count = entitySource.GetComponentCount(); i < count; i++)
			{
				componentSource = entitySource.GetComponent(i);
				if (!componentSource)
					continue;

				if (componentSource.GetClassName() != SLOT_WAYPOINT_COMPONENT_CLASSNAME) // exact match
					continue;

				// let's go, any issue is an error from here

				if (!worldEditorAPI.CreateObjectVariableMember(entitySource, waypointPath, WAYPOINT_COMPONENT_FIELD, CYCLE_WAYPOINT_CLASSNAME))
				{
					Print("Cannot create component's waypoint - " + cycleWaypointEntityName, LogLevel.WARNING);
					break;
				}

				if (!worldEditorAPI.SetVariableValue(entitySource, randomOrderBoolPath, "m_bUseRandomOrder", "1"))
				{
					Print("Cannot set component waypoint's UseRandomOrder value - " + cycleWaypointEntityName, LogLevel.WARNING);
					break;
				}

				dealt = true;
				break;
			}

			if (!dealt)
			{
				Print("Waypoint could not be set in SlotWaypoint - " + cycleWaypointEntityName, LogLevel.WARNING);
				worldEditorAPI.DeleteEntity(entitySource);
			}
		}

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] allEntitySources
	//! \param[out] namedLayers
	//! \param[out] cycleWaypointSets
	protected void GetLayersAndWaypointSets(notnull array<IEntitySource> allEntitySources, out notnull map<string, IEntitySource> namedLayers, out notnull array<ref SCR_WaypointSet> cycleWaypointSets)
	{
		namedLayers.Clear();
		cycleWaypointSets.Clear();

		SCR_WaypointSet waypointSet;

		BaseContainer ancestor;
		IEntityComponentSource componentSource;
		BaseContainerList waypointGroupNames;
		BaseContainer waypointGroupName;
		string name;
		bool found, randomOrder, cycle;
		foreach (IEntitySource entitySource : allEntitySources)
		{
			ancestor = entitySource.GetAncestor();
			if (SCR_BaseContainerTools.IsKindOf(ancestor, LAYER_PREFAB))
			{
				name = entitySource.GetName();
				if (name)
					namedLayers.Insert(name, entitySource);
			}
			else
			if (
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_AI_PREFAB) ||
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_KILL_PREFAB) ||
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_DEFEND_PREFAB))
			{
				for (int i, count = entitySource.GetComponentCount(); i < count; i++)
				{
					found = false;

					componentSource = entitySource.GetComponent(i);
					if (!componentSource) // how
						continue;

					name = componentSource.GetClassName();
					if (!BASE_SLOTWP_CLASSNAMES.Contains(name)) // EXACT match
						continue;

					waypointGroupNames = componentSource.GetObjectArray("m_aWaypointGroupNames");
					if (!waypointGroupNames)
						continue;

					for (int j, countJ = waypointGroupNames.Count(); j < countJ; j++)
					{
						waypointGroupName = waypointGroupNames.Get(j);
						if (!waypointGroupName)
							continue;

						if (!waypointGroupName.Get("m_bCycleWaypoints", cycle)
							|| !cycle
							|| !waypointGroupName.Get("m_sName", name)
							|| !waypointGroupName.Get("m_bUseRandomOrder", randomOrder))
							continue;

						waypointSet = new SCR_WaypointSet();
						waypointSet.m_sName = name;
						waypointSet.m_bCycleWaypoints = cycle;
						waypointSet.m_bUseRandomOrder = randomOrder;

						cycleWaypointSets.Insert(waypointSet);
						found = true;
					}

					if (found)
						break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] allEntitySources
	protected void Convert(notnull array<IEntitySource> allEntitySources)
	{
		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();

		IEntity entity;
		IEntitySource ancestor;
		foreach (IEntitySource entitySource : allEntitySources)
		{
			entity = worldEditorAPI.SourceToEntity(entitySource);
			if (!entity) // how
				continue;

			ancestor = entitySource.GetAncestor();
			if (SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_PREFAB))
			{
				TryConvertSlot(entitySource);
			}
			else if (
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_AI_PREFAB) ||
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_KILL_PREFAB) ||
				SCR_BaseContainerTools.IsKindOf(ancestor, SLOT_DEFEND_PREFAB))
			{
				TryAndConvertSlotWaypointGroups(entitySource);
			}
		}

		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entitySource
	protected void TryConvertSlot(notnull IEntitySource entitySource)
	{
		IEntityComponentSource componentSource;
		ResourceName spawnedResourceName;
		BaseContainer resourceBaseContainer;

		for (int i, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (!componentSource) // how
				continue;

			if (componentSource.GetClassName() != BASE_SLOT_CLASSNAME) // EXACT match
				continue;

			if (!componentSource.Get(BASE_SLOT_FIELD, spawnedResourceName)) // how
			{
				Print("Cannot obtain " + BASE_SLOT_FIELD + " value", LogLevel.WARNING);
				continue;
			}

			if (spawnedResourceName.IsEmpty()) // OK
				continue;

			Resource resource = Resource.Load(spawnedResourceName);
			if (!resource.IsValid())
			{
				Print("Invalid resource " + spawnedResourceName, LogLevel.WARNING);
				continue;
			}

			resourceBaseContainer = resource.GetResource().ToBaseContainer();
			if (!resourceBaseContainer) // how
			{
				Print("No base container for " + spawnedResourceName, LogLevel.WARNING);
				continue;
			}

			bool didConvert;
			foreach (SCR_ScenarioFrameworkConversionPlugin_Rune rune : CONVERSION_TABLE)
			{
				if (SCR_BaseContainerTools.IsKindOf(resourceBaseContainer, rune.m_sOldWaypointPrefab))
				{
					if (ConvertSlotTo(entitySource, componentSource, rune))
					{
						Print("Successfully temp converted " + rune.m_sOldWaypointPrefab + " to " + rune.m_sNewSlotPrefab, LogLevel.NORMAL);
						didConvert = true;
					}
					else
					{
						Print("Failed to convert " + rune.m_sOldWaypointPrefab + " to " + rune.m_sNewSlotPrefab, LogLevel.WARNING);
					}

					break;
				}
			}

			if (didConvert)
				Print("Be sure to save, load the world and run the plugin's part two", LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entitySource
	//! \param[in] componentSource
	//! \param[in] rune
	//! \return true on success, false otherwise
	protected bool ConvertSlotTo(notnull IEntitySource entitySource, notnull IEntitySource componentSource, notnull SCR_ScenarioFrameworkConversionPlugin_Rune rune)
	{
		array<IEntityComponentSource> oldComponentSources = {};
		for (int i = entitySource.GetComponentCount() - 1; i >= 0; --i)
		{
			oldComponentSources.Insert(entitySource.GetComponent(i));
		}

		entitySource.SetAncestor(rune.m_sNewSlotPrefab);
		if (entitySource.GetAncestor().GetResourceName() != rune.m_sNewSlotPrefab)
		{
			Print("Cannot change entitySource ancestor", LogLevel.ERROR);
			return false;
		}

		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		string componentType = rune.m_ComponentType.ToString();
		IEntityComponentSource newComponentSource = worldEditorAPI.CreateComponent(entitySource, componentType);
		if (!newComponentSource)
		{
			Print("New component " + rune.m_ComponentType + " was not found in the new ancestor", LogLevel.ERROR);
			return false;
		}

		if (!SCR_EntitySourceHelper.CopyDataFromOldToNewComponent(entitySource, componentSource, entitySource, newComponentSource))
		{
			Print("Old data cannot be ported over to new component", LogLevel.ERROR);
			if (!worldEditorAPI.DeleteComponent(entitySource, newComponentSource))
				Print("And the new component cannot be deleted - do it manually", LogLevel.ERROR);

			return false;
		}

		int newComponentId = -1;
		for (int i = entitySource.GetComponentCount() - 1; i >= 0; --i)
		{
			if (newComponentSource == entitySource.GetComponent(i))
			{
				newComponentId = i;
				break;
			}
		}

		if (newComponentId == -1)
		{
			Print("Cannot find the new component's id for path", LogLevel.ERROR);
			return false;
		}

		if (!worldEditorAPI.ClearVariableValue(entitySource, { new ContainerIdPathEntry(componentType, newComponentId) }, BASE_SLOT_FIELD))
		{
			Print("Cannot clear field, do it manually - " + BASE_SLOT_FIELD, LogLevel.WARNING);
		}

		// create Waypoint instance
		BaseContainer baseContainer = newComponentSource.GetObject(WAYPOINT_COMPONENT_FIELD);
		if (baseContainer)
		{
			Print("Waypoint already set, skipping", LogLevel.WARNING);
		}
		else
		{
			array<ref ContainerIdPathEntry> path = { new ContainerIdPathEntry("components", newComponentId) };
			string waypointClassname = ((typename)rune.m_WaypointClassType).ToString();
			if (!worldEditorAPI.CreateObjectVariableMember(entitySource, path, WAYPOINT_COMPONENT_FIELD, waypointClassname))
			{
				Print("Cannot create the new waypoint", LogLevel.ERROR);
				if (!worldEditorAPI.DeleteComponent(entitySource, newComponentSource))
					Print("And the new component cannot be deleted - do it manually", LogLevel.ERROR);

				return false;
			}

			// reset ALL values to default
			// baseContainer.ClearVariables();
		}

		foreach (IEntityComponentSource oldComponentSource : oldComponentSources)
		{
			if (componentSource)
				worldEditorAPI.DeleteComponent(entitySource, oldComponentSource);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! all-in-one Try AND Convert method
	//! \param[in] entitySource
	protected void TryAndConvertSlotWaypointGroups(notnull IEntitySource entitySource)
	{
		string classname;
		IEntityComponentSource componentSource;
		array<ref SCR_WaypointSet> waypointGroupNameSets;
		SCR_ScenarioFrameworkWaypointSet existingWaypointSet;
		BaseContainer resourceBaseContainer;
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		for (int i, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (!componentSource) // how
				continue;

			classname = componentSource.GetClassName();
			if (!BASE_SLOTWP_CLASSNAMES.Contains(classname)) // EXACT match
				continue;

			if (!componentSource.Get(BASE_SLOTWP_FIELD_FROM, waypointGroupNameSets))
				continue;

			if (!waypointGroupNameSets) // OK
				continue;

			if (!componentSource.Get(BASE_SLOTWP_FIELD_TO, existingWaypointSet))
				continue;

			if (existingWaypointSet)
				continue;

			array<ref ContainerIdPathEntry> path = { new ContainerIdPathEntry("components", i) };

			if (!worldEditorAPI.CreateObjectVariableMember(entitySource, path, BASE_SLOTWP_FIELD_TO, WAYPOINT_CLASSNAME))
			{
				Print("Cannot create " + WAYPOINT_CLASSNAME + " - " + entitySource.GetName(), LogLevel.ERROR);
				continue;
			}

			path.Insert(new ContainerIdPathEntry(BASE_SLOTWP_FIELD_TO));

			array<string> waypointNames = {};
			foreach (int j, SCR_WaypointSet waypointSet : waypointGroupNameSets)
			{
				if (waypointSet.m_sName) // !.IsEmpty()
				{
					if (waypointSet.m_bCycleWaypoints)
						waypointNames.Insert(waypointSet.m_sName + CYCLE_SUFFIX);
					else
						waypointNames.Insert(waypointSet.m_sName);
				}
			}

			if (!worldEditorAPI.SetVariableValue(entitySource, path, "m_aLayerName", SCR_StringHelper.Join(",", waypointNames)))
			{
				Print("Cannot set waypoint name array values - " + entitySource.GetName(), LogLevel.ERROR);
				continue;
			}

			if (!worldEditorAPI.ClearVariableValue(entitySource, { new ContainerIdPathEntry("components", i) }, BASE_SLOTWP_FIELD_FROM))
				componentSource.ClearVariable(BASE_SLOTWP_FIELD_FROM);

			if (componentSource.Get(BASE_SLOTWP_FIELD_FROM, waypointGroupNameSets) && (!waypointGroupNameSets || !waypointGroupNameSets.IsEmpty()))
			{
				Print("Cannot clear previous waypoint set; do it manually - " + entitySource.GetName(), LogLevel.WARNING);
			}

			Print("Successfully converted waypoints", LogLevel.DEBUG);
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_ScenarioFrameworkConversionPlugin_Phase1()
	{
		if (!CONVERSION_TABLE.IsEmpty())
			return;

		array<ref SCR_ScenarioFrameworkConversionPlugin_Rune> runes = {
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Attack
				"{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointAttack),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Capture Relay
				"{EAAE93F98ED5D218}Prefabs/AI/Waypoints/AIWaypoint_CaptureRelay.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointCaptureRelay),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Defend
				"{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointDefend),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Defend CP
				"{2A81753527971941}Prefabs/AI/Waypoints/AIWaypoint_Defend_CP.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointDefendCP),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Defend Hierarchy
				"{AAE8882E0DE0761A}Prefabs/AI/Waypoints/AIWaypoint_Defend_Hierarchy.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointDefendHierarchy),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Defend Large
				"{FAD1D789EE291964}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointDefendLarge),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Defend Large CO
				"{A33AF7FC5004F294}Prefabs/AI/Waypoints/AIWaypoint_Defend_Large_CO.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointDefendLargeCO),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Follow
				"{A0509D3C4DD4475E}Prefabs/AI/Waypoints/AIWaypoint_Follow.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointFollow),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Forced Move
				"{06E1B6EBD480C6E0}Prefabs/AI/Waypoints/AIWaypoint_ForcedMove.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointForcedMove),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Get In
				"{712F4795CF8B91C7}Prefabs/AI/Waypoints/AIWaypoint_GetIn.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointGetIn),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Get In Nearest
				"{B049D4C74FBC0C4D}Prefabs/AI/Waypoints/AIWaypoint_GetInNearest.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointGetInNearest),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Get Out
				"{C40316EE26846CAB}Prefabs/AI/Waypoints/AIWaypoint_GetOut.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointGetOut),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Heal
				"{36ED7C150D5BB654}Prefabs/AI/Waypoints/AIWaypoint_Heal.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointHeal),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Loiter CO
				"{4ECD14650D82F5CA}Prefabs/AI/Waypoints/AIWaypoint_Loiter_CO.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointLoiterCO),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Move
				"{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointMove),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Observation Post
				"{97FB527ECC7CA49E}Prefabs/AI/Waypoints/AIWaypoint_ObservationPost.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointObservationPost),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Open Gate
				"{1966BC58CE769D69}Prefabs/AI/Waypoints/AIWaypoint_OpenGate.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointOpenGate),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Patrol
				"{22A875E30470BD4F}Prefabs/AI/Waypoints/AIWaypoint_Patrol.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointPatrol),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Attack
				"{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointPatrolHierarchy),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Scout
				"{A88F0B6CF25BD1DE}Prefabs/AI/Waypoints/AIWaypoint_Scout.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointScout),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Search and Destroy
				"{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointSearchAndDestroy),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// User Action
				"{04A06A6742FB0AF8}Prefabs/AI/Waypoints/AIWaypoint_UserAction.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointUserAction),
			new SCR_ScenarioFrameworkConversionPlugin_Rune(									// Wait
				"{531EC45063C1F57B}Prefabs/AI/Waypoints/AIWaypoint_Wait.et",
				"{EBD91177954E8236}Prefabs/ScenarioFramework/Components/SlotWaypoint.et",
				SCR_ScenarioFrameworkSlotWaypoint,
				SCR_ScenarioFrameworkWaypointWait),
		};

		SCR_ArrayHelperRefT<SCR_ScenarioFrameworkConversionPlugin_Rune>.CopyReferencesFromTo(runes, CONVERSION_TABLE);
	}
}

// Only SlotAI, SlotKill and SlotDefend have WaypointSets

//! This plugin converts old scenarios using Scenario Framework from 1.0.0 to the new 1.1.0 format.\n
//! The plugin must be run once, AFTER having run phase 1's plugin.
//! \see SCR_ScenarioFrameworkConversionPlugin_Phase1
[WorkbenchPluginAttribute(
	name: "Scenario Framework 1.0.0 -> 1.1.0 Update - phase 2",
	shortcut: "", // "Ctrl+Alt+S",
	category: "Update/1.0.0 to 1.1.0",
	wbModules: { "WorldEditor" },
	awesomeFontCode: 0x0032, // 0xE54E,
	description: "This plugin updates Scenario Framework Slots from the old (1.0.0) to the new (1.1.0+) standard")]
class SCR_ScenarioFrameworkConversionPlugin_Phase2 : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		GetAllSlotWaypointWithTwoSlotWaypointComponentsAndRemoveUnsetOne();
	}

	//------------------------------------------------------------------------------------------------
	//! as the name says
	protected void GetAllSlotWaypointWithTwoSlotWaypointComponentsAndRemoveUnsetOne()
	{
		WorldEditorAPI worldEditorAPI = SCR_WorldEditorToolHelper.GetWorldEditorAPI();
		if (!worldEditorAPI)
			return;

		bool manageEditAction = SCR_WorldEditorToolHelper.BeginEntityAction();

		IEntitySource entitySource;
		IEntitySource ancestor;
		IEntityComponentSource componentSource;
		array<IEntityComponentSource> sameComponents = {};
		Debug.BeginTimeMeasure();
		for (int i, count = worldEditorAPI.GetEditorEntityCount(); i < count; i++)
		{
			entitySource = worldEditorAPI.GetEditorEntity(i);
			if (!entitySource)
				continue;

			ancestor = entitySource.GetAncestor();
			if (!ancestor)
				continue;

			if (entitySource.GetClassName() != SCR_ScenarioFrameworkConversionPlugin_Phase1.GENERIC_CLASSNAME)
				continue;

			if (!SCR_BaseContainerTools.IsKindOf(ancestor, SCR_ScenarioFrameworkConversionPlugin_Phase1.SLOT_WAYPOINT_PREFAB))
				continue;

			int compNum;
			sameComponents.Clear();
			for (int j, countJ = entitySource.GetComponentCount() - 1; j < countJ; j++)
			{
				componentSource = entitySource.GetComponent(j);
				if (!componentSource)
					continue;

				if (componentSource.GetClassName() == SCR_ScenarioFrameworkConversionPlugin_Phase1.SLOT_WAYPOINT_COMPONENT_CLASSNAME)
				{
					compNum++;
					sameComponents.Insert(componentSource);
					if (compNum == 3)
						break;
				}
			}

			if (compNum == 2) // no more, no less
			{
				componentSource = sameComponents[0];
				IEntityComponentSource componentSource2 = sameComponents[1];
				if (componentSource.IsVariableSetDirectly(SCR_ScenarioFrameworkConversionPlugin_Phase1.WAYPOINT_COMPONENT_FIELD))
				{
					SCR_EntitySourceHelper.CopyDataFromOldToNewComponent(entitySource, componentSource, entitySource, componentSource2);
					worldEditorAPI.DeleteComponent(entitySource, componentSource);
				}
				else
				{
					SCR_EntitySourceHelper.CopyDataFromOldToNewComponent(entitySource, componentSource2, entitySource, componentSource);
					worldEditorAPI.DeleteComponent(entitySource, componentSource2);
				}
			}
		}

		Debug.EndTimeMeasure("Moving data from temp component to Prefab's component");
		SCR_WorldEditorToolHelper.EndEntityAction(manageEditAction);
	}
}

// conversion runes, like in Indiana Jones and stuff
class SCR_ScenarioFrameworkConversionPlugin_Rune
{
	ResourceName m_sOldWaypointPrefab;
	ResourceName m_sNewSlotPrefab;
	typename m_ComponentType;
	typename m_WaypointClassType;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] oldWaypointPrefab what Prefab kind must be searched
	//! \see SCR_BaseContainerTools.IsKindOf()
	//! \param[in] newWaypointPrefab the waypoint Prefab type to which to convert
	//! \param[in] componentType the component to create in the created Prefab
	//! \param[in] waypointClassType the waypoint type to create in the component
	void SCR_ScenarioFrameworkConversionPlugin_Rune(
		ResourceName oldWaypointPrefab,
		ResourceName newSlotPrefab,
		typename componentType,
		typename waypointClassType)
	{
		m_sOldWaypointPrefab = oldWaypointPrefab;
		m_sNewSlotPrefab = newSlotPrefab;
		m_ComponentType = componentType;
		m_WaypointClassType = waypointClassType;
	}
}
#endif
