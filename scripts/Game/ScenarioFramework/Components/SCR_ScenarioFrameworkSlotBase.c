[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotBaseClass : SCR_ScenarioFrameworkLayerBaseClass
{
}

class SCR_ScenarioFrameworkSlotBase : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(params: "et", desc: "Resource name of the object to be spawned", category: "Asset")]
	ResourceName m_sObjectToSpawn;

	[Attribute(desc: "Selects which object to spawn based on the selected Faction Key", category: "Asset")]
	ref array<ref SCR_ScenarioFrameworkFactionSwitchedObject> m_aFactionSwitchedObjects;

	[Attribute(desc: "Name of the entity used for identification", category: "Asset")]
	string m_sID;

	[Attribute(desc: "This won't spawn new object, but it will rather use the object already existing in the world", category: "Asset")]
	bool m_bUseExistingWorldAsset;

	[Attribute(desc: "Overrides display name of the spawned object for task purposes", category: "Asset")]
	string m_sOverrideObjectDisplayName;

	[Attribute(defvalue: "1", desc: "If the spawned entity can be garbage-collected", category: "Asset")]
	bool m_bCanBeGarbageCollected;

	[Attribute(desc: "Randomize spawned asset(s) per Faction Attribute which needs to be filled as well. Will override Object To Spawn Attribute.", category: "Randomization")]
	bool m_bRandomizePerFaction;

	[Attribute("0", UIWidgets.SearchComboBox, "Select Entity Catalog type for random spawn", "", ParamEnumArray.FromEnum(EEntityCatalogType), category: "Randomization")]
	EEntityCatalogType m_eEntityCatalogType;

	[Attribute("0", UIWidgets.SearchComboBox, "Select Entity Labels which you want to optionally include to random spawn. If you want to spawn everything, you can leave it out empty and also leave Include Only Selected Labels attribute to false.", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	ref array<EEditableEntityLabel> m_aIncludedEditableEntityLabels;

	[Attribute("0", UIWidgets.SearchComboBox, "Select Entity Labels which you want to exclude from random spawn", "", ParamEnumArray.FromEnum(EEditableEntityLabel), category: "Randomization")]
	ref array<EEditableEntityLabel> m_aExcludedEditableEntityLabels;

	[Attribute(desc: "If true, it will spawn only the entities that are from Included Editable Entity Labels and also do not contain Label to be Excluded.", category: "Randomization")]
	bool m_bIncludeOnlySelectedLabels;

	[Attribute(defvalue: "0", category: "Composition", desc: "When disabled orientation to terrain will be skipped for the next composition")]
	bool m_bIgnoreOrientChildrenToTerrain;

	ref EntitySpawnParams m_SpawnParams = new EntitySpawnParams();
	vector m_Size;
	ResourceName m_sRandomlySpawnedObject;
	vector m_vPosition;
	
	// A fallback entity with the same prefab name as m_sObjectToSpawn (in case exact prefab matching fails)
	IEntity m_fallbackEntityByName;
	
	// A fallback entity whose prefab is the ancestor of m_sObjectToSpawn (in case exact prefab matching fails)
	IEntity m_fallbackEntityByAncestor;

#ifdef WORKBENCH
	protected IEntity m_PreviewEntity;
#endif

	//------------------------------------------------------------------------------------------------
	//! Queries nearby objects within range for the owner entity.
	//! \param[in] fRange Represents search radius for objects in range query.
	protected void QueryObjectsInRange(float fRange = 2.5)
	{
		// We're searching by prefab name, so if this is empty, there's no point to continue
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
			return;
		
		BaseWorld pWorld = GetGame().GetWorld();
		if (!pWorld)
			return;

		m_fallbackEntityByName = null;
		m_fallbackEntityByAncestor = null;
		pWorld.QueryEntitiesBySphere(GetOwner().GetOrigin(), fRange, GetEntity, null, EQueryEntitiesFlags.ALL);
		
		// if no exact prefab match was found, use a fallback entity
		if (!m_Entity)
		{
			if (m_fallbackEntityByAncestor)
				m_Entity = m_fallbackEntityByAncestor;
			else
				m_Entity = m_fallbackEntityByName;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state Checks if object is destroyed, removes damage state change listener, and terminates object if repeated spawn is not enabled and run out of numbers
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Entity)
			return;

		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Entity);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);

		if (!m_bEnableRepeatedSpawn || (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0))
			SetIsTerminated(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Method changes inventory parent slot and sets dynamic despawn exclusion for an entity.
	//! \param[in] oldSlot Represents previous inventory slot where item was before being moved.
	//! \param[in] newSlot Represents new inventory slot for item transfer.
	void OnInventoryParentChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{
		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
		if (!invComp)
			return;

		SetDynamicDespawnExcluded(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Excludes vehicle from dynamic despawning when entering a component.
	//! \param[in] vehicle Excludes vehicle from dynamic despawning when entering a component.
	//! \param[in] mgr BaseCompartmentManagerComponent represents an entity component managing cargo, passengers, and other objects in a vehicle.
	//! \param[in] occupant Occupant represents the entity currently inside the vehicle when the method is called.
	//! \param[in] managerId ManagerId represents the unique identifier for the comparment manager in the vehicle.
	//! \param[in] slotID SlotID represents the specific seat.
	void OnCompartmentEntered(IEntity vehicle, BaseCompartmentManagerComponent mgr, IEntity occupant, int managerId, int slotID)
	{
		m_bExcludeFromDynamicDespawn = true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the display name for an overridden object.
	string GetOverriddenObjectDisplayName()
	{
		return m_sOverrideObjectDisplayName;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name Sets object display name override.
	void SetOverriddenObjectDisplayName(string name)
	{
		m_sOverrideObjectDisplayName = name;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the display name of the spawned entity or overridden display name if provided, otherwise returns an empty string.
	string GetSpawnedEntityDisplayName()
	{
		if (!m_Entity || !m_sOverrideObjectDisplayName.IsEmpty())
			return m_sOverrideObjectDisplayName;

		SCR_EditableEntityComponent editableEntityComp = SCR_EditableEntityComponent.Cast(m_Entity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntityComp)
			return string.Empty;

		return editableEntityComp.GetDisplayName();
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if entity matches prefab data, if not, checks if it matches object to spawn, if not, returns true
	//! \param[in] entity to be outed
	//! \return whether the provided entity matches the expected entity based on prefab data or object name.
	protected bool GetEntity(notnull IEntity entity)
	{
		IEntity pParent = SCR_EntityHelper.GetMainParent(entity, true);
		EntityPrefabData prefabData = pParent.GetPrefabData();
		if (!prefabData)
			return true;

		ResourceName resource = SCR_ResourceNameUtils.GetPrefabName(entity);
		
		if (resource == m_sObjectToSpawn)
		{
			m_Entity = entity;
			return false;
		}
		
		// this isn't the exact prefab we want; try fallback (only one fallback entity is enough)
		if (!m_fallbackEntityByAncestor)
		{
			if (SCR_BaseContainerTools.IsKindOf(prefabData.GetPrefab(), m_sObjectToSpawn))
			{
				// Use this entity as fallback by ancestor
				m_fallbackEntityByAncestor = entity;
			}
			else if (!m_fallbackEntityByName)
			{
				// Check if the prefab names match
				
				TStringArray strs = new TStringArray();
				resource.Split("/", strs, true);
				string resourceName;
				if(strs.Count() > 0)
					resourceName = strs[strs.Count() - 1];
	
				TStringArray strsObject = new TStringArray();
				m_sObjectToSpawn.Split("/", strsObject, true);
				string resourceObject;
				if(strsObject.Count() > 0)
					resourceObject = strsObject[strsObject.Count() - 1];
	
				// if names match, use this entity as fallback by name
				if (resourceName && resourceName == resourceObject)
					m_fallbackEntityByName = entity;
			}
		}
		
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the ScenarioFrameworkArea component from the parent entity or owner, or null if not found.
	SCR_ScenarioFrameworkArea GetAreaWB()
	{
		SCR_ScenarioFrameworkArea area;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (area)
				return area;

			entity = entity.GetParent();
		}

		return area;
	}

	//------------------------------------------------------------------------------------------------
	//! \return The spawned object's unique identifier as a string.
	string GetSpawnedObjectName()
	{
		return m_sID;
	}

	//------------------------------------------------------------------------------------------------
	//! \return spawned object ResourceName.
	ResourceName GetObjectToSpawn()
	{
		return m_sObjectToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! First attempts to get get teh Fction Switched Object for the provided faciton key, then falls back to m_sObjectToSpawn.
	//! Init does not include Catalog entities as the catalog manager has not been initialised yet.
	//! If m_bRandomizePerFaction, then catalog system will ignore m_sObjectToSpawn if it has a catalog match. Otherwise it falls back to m_sObjectToSpawn.
	ResourceName GetSelectedObjectToSpawn()
	{
		if (!m_aFactionSwitchedObjects.IsEmpty())
		{
			// Try get faction switched object
			foreach (SCR_ScenarioFrameworkFactionSwitchedObject factionObject : m_aFactionSwitchedObjects)
			{
				if (factionObject.m_sFactionKey == m_sFactionKey)
					return factionObject.m_sObjectToSpawn;
			}
			
			if (m_sObjectToSpawn.IsEmpty())
				Print(string.Format("ScenarioFramework [SCR_ScenarioFrameworkSlotBase] No faction switch object for \"%1\" FactionKey and no fallback m_sObjectToSpawn.", m_sFactionKey), LogLevel.WARNING);
		}

		return m_sObjectToSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Randomly spawned object ResourceName.
	ResourceName GetRandomlySpawnedObject()
	{
		return m_sRandomlySpawnedObject;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] name Represents an object to spawn randomly in the world.
	void SetRandomlySpawnedObject(ResourceName name)
	{
		m_sRandomlySpawnedObject = name;
	}

	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears random object, resets position, calls superclass method.
	//! \param[in] includeChildren Includes children objects in default restoration process.
	//! \param[in] reinitAfterRestoration Restores object state after restoration, resetting internal variables.
	//! \param[in] affectRandomization Affects randomization during object restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_sRandomlySpawnedObject = string.Empty;
		m_vPosition = vector.Zero;

		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! //! Dynamically despawns this layer.
	//! \param[in] layer Layer represents the scenario framework layer where dynamic despawning occurs.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}

		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		if (m_Entity)
		{
			m_vPosition = m_Entity.GetOrigin();
			InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
			if (invComp)
				invComp.m_OnParentSlotChangedInvoker.Remove(OnInventoryParentChanged);
		}

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aSpawnedEntities.RemoveItem(null);
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			m_vPosition = entity.GetOrigin();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes whether an init has already happened, but for slots it always returns false.
	//! \return false, indicating that the init has not yet occurred.
	override bool InitAlreadyHappened()
	{
		// We do not want to check this condition for Slots
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if entity is spawned, if not, waits for all children to spawn then checks again.
	//! \return true if all children entities have been spawned, false otherwise.
	bool InitEntitySpawnCheck()
	{
		if (!m_Entity)
		{
			GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
			InvokeAllChildrenSpawned();
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! If m_bRandomizePerFaction, then catalog system will ignore m_sObjectToSpawn if it has a catalog match. Otherwise it falls back to m_sObjectToSpawn.
	protected void InitSelectedObjectToSpawn()
	{
		m_sObjectToSpawn = GetSelectedObjectToSpawn();
	}

	//------------------------------------------------------------------------------------------------
	//! If m_bRandomizePerFaction, then it will set m_sObjectToSpawn to empty because the the catalog system has not been initialised.
	//! It would be incorect to show the manually set object or the faction switched object because the catalog will take preference at runtime.
	protected void InitSelectedObjectToPreview()
	{
		// Manually get the faction key from parents because attribute inheritance doesn't run in Workbench.
		m_sFactionKey = GetParentFactionKeyRecursive();
		if (m_bRandomizePerFaction)
		{
			m_sObjectToSpawn = "";  // Maybe put a preview object here that is a mesh of a word saying "Catalog Result" or something.
			return;
		}
		InitSelectedObjectToSpawn();
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns entity if not existing or queries objects in range, checks initiation success.
	//! \return true if entity spawning is successful.
	bool InitEntitySpawn()
	{
		if (m_Entity && !m_Entity.IsDeleted())
			return true; // Savegame will have linked the entity to the slot before SF system init
		
		if (!m_bUseExistingWorldAsset)
		{
			m_Entity = SpawnAsset();
		}
		else
		{
			if (!m_sID.IsEmpty())
			{
				m_Entity = GetGame().GetWorld().FindEntityByName(m_sID);
			}
			
			if (!m_Entity)
				QueryObjectsInRange();	//sets the m_Entity in subsequent callback
		}

		if (!InitEntitySpawnCheck())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes selected object, repeatable spawn, and entity spawn.
	//! \return true if all initialization steps succeed, false otherwise.
	override bool InitOtherThings()
	{
		InitSelectedObjectToSpawn();

		if (!InitEntitySpawn())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Finishes initialization, assigns unique ID, sets up damage manager, event handlers, inventory, and garbage system if applicable
	override void FinishInit()
	{
		if (!m_sID.IsEmpty() && m_Entity)
		{
			IEntity ent = GetGame().GetWorld().FindEntityByName(m_sID);
			if (ent != m_Entity)
			{
				string IDWithSuffix = m_sID;
				int suffixNumber;
				while (GetGame().GetWorld().FindEntityByName(IDWithSuffix))
	            {
					suffixNumber++;
					Print(string.Format("ScenarioFramework Slot: Entity of name %1 was found. The suffix _%2 will be added.", m_sID, suffixNumber), LogLevel.WARNING);
					IDWithSuffix = m_sID + "_" + suffixNumber.ToString();
	            }

				m_Entity.SetName(IDWithSuffix);
			}
		}

		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Entity);
		if (objectDmgManager)
			objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);

		if (Vehicle.Cast(m_Entity))
		{
			EventHandlerManagerComponent ehManager = EventHandlerManagerComponent.Cast(m_Entity.FindComponent(EventHandlerManagerComponent));
			if (ehManager)
				ehManager.RegisterScriptHandler("OnCompartmentEntered", this, OnCompartmentEntered, true);
		}

		InventoryItemComponent invComp = InventoryItemComponent.Cast(m_Entity.FindComponent(InventoryItemComponent));
		if (invComp)
			invComp.m_OnParentSlotChangedInvoker.Insert(OnInventoryParentChanged);

		if (!m_bCanBeGarbageCollected)
		{
			SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Entity);
			if (garbageSystem)
				garbageSystem.UpdateBlacklist(m_Entity, true);
		}

		super.FinishInit();

		InvokeAllChildrenSpawned();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework slot
	//! \param[in] area that this slot is nested into.
	//! \param[in] activation Activates scenario framework action type, sets up for children spawning.
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned.Remove(DynamicDespawn);

		super.Init(area, activation);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes plugins, actions, and checks if parent layer has all children spawned after all children have spawned in scenario framework
	//! \param[in] layer Initializes layer, sets up plugins, actions, and checks parent layer spawning status.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		InitActivationActions();

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Slot cannot have children
	//! \param[in] previouslyRandomized PreviouslyRandomized: Boolean indicating if children were previously randomized before spawning.
	override void SpawnChildren(bool previouslyRandomized = false)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Randomly selects an entity from a catalog based on provided parameters, handles error cases, and returns the prefab of the
	//! \param[out] prefab Randomly selected prefab from entity catalog for randomized spawning.
	//! \return The return value represents a randomly selected prefab from the entity catalog based on the applied labels and filters.
	ResourceName GetRandomAsset(out ResourceName prefab)
	{
		SCR_EntityCatalog entityCatalog;

		SCR_EntityCatalogManagerComponent catalogManager = SCR_EntityCatalogManagerComponent.GetInstance();

		//~ Get data from faction manager
		if (catalogManager)
		{
			//~ Faction set so get catalog from faction
			if (!m_sFactionKey.IsEmpty())
				entityCatalog = catalogManager.GetFactionEntityCatalogOfType(m_eEntityCatalogType, m_sFactionKey);
			//~ No faction set so get factionless list
			else
				entityCatalog = catalogManager.GetEntityCatalogOfType(m_eEntityCatalogType);
		}
		//~ For some reason catalog manager was not found so get data from faction
		else
		{
			if (m_sFactionKey.IsEmpty())
			{
				Print("ScenarioFramework Randomized Spawn: No catalog manager found and faction key is empty!", LogLevel.ERROR);
				return string.Empty;
			}

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
		{
			Print("ScenarioFramework Randomized Spawn: Faction manager not found.", LogLevel.ERROR);
			return string.Empty;
		}

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_sFactionKey));
		if (!faction)
		{
			Print(string.Format("ScenarioFramework Randomized Spawn: Selected faction not found for %1.", GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
		}

			//~ Get entity Catalog from faction
			entityCatalog = faction.GetFactionEntityCatalogOfType(m_eEntityCatalogType);

		if (!entityCatalog)
		{
				Print(string.Format("ScenarioFramework Randomized Spawn: Faction Entity Catalog (type: %1) not found for %2.", typename.EnumToString(EEntityCatalogType, m_eEntityCatalogType), GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
			}
		}

		//~ No catalog found
		if (!entityCatalog)
		{
			if (!m_sFactionKey.IsEmpty())
				Print(string.Format("ScenarioFramework Randomized Spawn: Faction Entity Catalog for faction %1 (type %2) not found for %3.", m_sFactionKey, typename.EnumToString(EEntityCatalogType, m_eEntityCatalogType), GetOwner().GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Randomized Spawn: Non-Faction Entity Catalog (Type: %1) not found for %2.", typename.EnumToString(EEntityCatalogType, m_eEntityCatalogType), GetOwner().GetName()), LogLevel.ERROR);

			return string.Empty;
		}

		array<SCR_EntityCatalogEntry> entityEntries = {};
		entityCatalog.GetFullFilteredEntityListWithLabels(entityEntries, m_aIncludedEditableEntityLabels, m_aExcludedEditableEntityLabels, m_bIncludeOnlySelectedLabels);
		if (entityEntries.IsEmpty())
		{
			Print(string.Format("ScenarioFramework Randomized Spawn: Applied labels resulted in no viable prefabs to be randomly selected for %1.", GetOwner().GetName()), LogLevel.ERROR);
			return string.Empty;
		}

		prefab = entityEntries.GetRandomElement().GetPrefab();
		m_sRandomlySpawnedObject = prefab;
		return prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns random asset, sets position, ignores terrain orientation, requests navmesh rebuild, adds to spawned entities
	//! \return an entity spawned from the specified resource.
	IEntity SpawnAsset()
	{
		//If Randomization is enabled, it will try to apply settings from Attributes.
		//If it fails anywhere, original m_sObjectToSpawn will be used.
		if (m_bRandomizePerFaction)
		{
			ResourceName randomAsset = GetRandomlySpawnedObject();
			if (SCR_StringHelper.IsEmptyOrWhiteSpace(randomAsset))
				GetRandomAsset(randomAsset);

			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(randomAsset))
					m_sObjectToSpawn = randomAsset;
		}

		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource)
			return null;

		GetOwner().GetWorldTransform(m_SpawnParams.Transform);
		m_SpawnParams.TransformMode = ETransformMode.WORLD;
		//--- Apply rotation
		vector angles = Math3D.MatrixToAngles(m_SpawnParams.Transform);
		Math3D.AnglesToMatrix(angles, m_SpawnParams.Transform);

		//--- Spawn the prefab
		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
			return null;

		if (m_bIgnoreOrientChildrenToTerrain)
		{
			IEntityComponentSource slotCompositionComponent = SCR_BaseContainerTools.FindComponentSource(resourceObject, SCR_SlotCompositionComponent);
			if (slotCompositionComponent)
			{
				SCR_SlotCompositionComponent.IgnoreOrientChildrenToTerrain();
			}
		}

		IEntity entity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), m_SpawnParams);
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiWorld)
			aiWorld.RequestNavmeshRebuildEntity(entity);

		m_aSpawnedEntities.Insert(entity);

		if (m_vPosition != vector.Zero)
		{
			entity.SetOrigin(m_vPosition);
			entity.Update();
		}

		return entity;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Sets preview entity's transform or teleports it if it's a BaseGameEntity.
	//! \param[in] owner The owner represents the entity controlling the transformation changes in the method.
	//! \param[in,out] mat Represents transformation matrix for object's position, rotation, and scale.
	//! \param[in] src Source entity representing the object whose transform is being set.
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		if (!m_PreviewEntity)
			return;

		BaseGameEntity baseGameEntity = BaseGameEntity.Cast(m_PreviewEntity);
		if (baseGameEntity)
			baseGameEntity.Teleport(mat);
		else
			m_PreviewEntity.SetTransform(mat);
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns preview entity for given resource at owner's position.
	//! \param[in] owner The owner represents the entity that spawns the preview entity in the method.
	//! \param[in] resource Resource represents an object or entity prefab to spawn as preview for the owner entity.
	void SpawnEntityPreview(IEntity owner, Resource resource)
	{
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetWorldTransform(spawnParams.Transform);

		m_PreviewEntity = GetGame().SpawnEntityPrefab(resource, owner.GetWorld(), spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes preview entity, deletes previous one, loads resource, and spawns entity preview if resource is valid.
	//! \param[in] owner The owner represents the entity initiating the method call, typically an object in the game world.
	//! \param[in,out] mat Mat represents the world space transformation matrix for the object owner in the method.
	//! \param[in] src Source entity represents the object that triggers the initialization of the preview process in the method.
	override void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);

		InitSelectedObjectToPreview();
		Resource resource = Resource.Load(m_sObjectToSpawn);
		if (!resource || !resource.IsValid())
			return;

		SpawnEntityPreview(owner, resource);
	}

	//------------------------------------------------------------------------------------------------
	//! Manages key changes for debug shapes, object spawning, and entity deletion in Workbench.
	//! \param[in] owner The owner represents the entity invoking the method.
	//! \param[in] src represents the source container for key changes in the method.
	//! \param[in] key represents the workbench attribute
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);

		if (key == "coords")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);

			Resource resource = Resource.Load(m_sObjectToSpawn);
			if (!resource)
				return false;

			SpawnEntityPreview(owner, resource);
			return true;
		}
		else if (key == "m_sObjectToSpawn" || key == "m_sFactionKey" || key == "m_aFactionSwitchedObjects")
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
			return false;
		}
		return false;
	}

#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkSlotBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(100, 0x99, 0x10, 0xF2);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes preview entity in workbench mode, returns if in edit mode, otherwise despawns scenario slot base.
	void ~SCR_ScenarioFrameworkSlotBase()
	{
#ifdef WORKBENCH
		SCR_EntityHelper.DeleteEntityAndChildren(m_PreviewEntity);
#endif
		if (SCR_Global.IsEditMode())
			return;

		DynamicDespawn(this);
	}
}