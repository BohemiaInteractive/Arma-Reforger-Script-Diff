#ifdef WORKBENCH
[WorkbenchToolAttribute(
	name: "Destruction indices assign tool",
	description: "Automatically assigns response indices to destructible objects\n\n" +
	"- button  1 = Fix phases\n" +
	"- button  2 = Print all destructible resource names\n" +
	"- button  3 = Revert to parent\n" +
	"- button  4 = Update Prefabs\n" +
	"- button  5 = Remove phases\n" +
	"- button  6 = Filter Prefabs\n" +
	"- button  7 = Show Mass\n" +
	"- button  8 = Convert Prefabs\n" +
	"- button  9 = Assign Indices\n" +
	"- button 10 = Set building destruction effects - TODO",
	awesomeFontCode: 0xF7E4)]
class SCR_DestructionIndicesAssignTool : WorldEditorTool
{
	[Attribute(category: "General")]
	protected ref array<ref SCR_MassResponseIndexPair> m_aPairs;

	[Attribute(category: "General")]
	protected ref array<string> m_aProperties;

	protected const ref array<string> EXTENSIONS = { "et" };
	protected const ref array<string> DESTRUCTIBLE_COMPONENT_CLASSES = { "SCR_DestructionDamageManagerComponent", "SCR_DestructionMultiPhaseComponent" };
	protected const string DESTRUCTION_COMPONENT_CLASS = "SCR_DestructionMultiPhaseComponent";
	protected const string PHYSICS_COMPONENT = "RigidBody";

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("1 FP")]
	protected void FixPhasesButton()
	{
		m_API.BeginEntityAction();
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS);

		Resource resource;
		BaseResourceObject baseResourceObject;
		BaseContainer baseContainer, parent;
		IEntitySource parentEntitySource, entitySource;
		IEntityComponentSource destructionComponent;
		BaseContainerList parentPhasesList, phasesList;
		BaseContainerList parentOldPhasesList, oldPhasesList;
		int count = resourceNames.Count();
		while (count > 0)
		{
			for (int i = count - 1; i >= 0; i--)
			{
				// Only in prefabs folder
				if (!resourceNames[i].Contains("Prefabs/"))// || !resourceNames[i].Contains("BrickWall_01/BrickWall_01_white_2m.et"))
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				// Validation
				resource = Resource.Load(resourceNames[i]);
				if (!resource)
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				// Validation
				baseResourceObject = resource.GetResource();
				if (!baseResourceObject)
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				// Validation
				baseContainer = baseResourceObject.ToBaseContainer();
				if (!baseContainer)
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				// Class must be SCR_DestructibleEntity
				if (baseContainer.GetClassName() != "SCR_DestructibleEntity")
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				entitySource = baseContainer.ToEntitySource();
				destructionComponent = FindComponent(entitySource, "SCR_DestructionMultiPhaseComponent");

				parent = entitySource.GetAncestor();

				if (!parent)
				{
					count--;
					resourceNames.Remove(i);
					continue;
				}

				if (parent.GetClassName() == "SCR_DestructibleEntity" && resourceNames.Find(parent.GetResourceName()) != -1)
					continue; // has parent that wasn't handled yet, keep in array & count, so we determine this prefab later

				if (resourceNames[i] == "{A00A15B03B48A2AF}Prefabs/Structures/Walls/Brick/BrickWall_01/BrickWall_01_8m.et")
					Print("Test");

				parentEntitySource = parent.ToEntitySource();
				phasesList = baseContainer.GetObjectArray("DamagePhases");
				parentPhasesList = parent.GetObjectArray("DamagePhases");

				if (resourceNames[i] == "{A00A15B03B48A2AF}Prefabs/Structures/Walls/Brick/BrickWall_01/BrickWall_01_8m.et")
				{
					Print(parentPhasesList.Count());
					Print(parent.GetResourceName());
				}

				if (parentPhasesList && parentPhasesList.Count() != 0)
				{
					// Parent has some phases, we have to redo the current objects phases
					//if (entitySource.IsVariableSetDirectly("DamagePhases"))
					//	m_API.ClearVariableValue(entitySource, null, "DamagePhases");

					ConvertPhases(entitySource, destructionComponent);

					//oldPhasesList = destructionComponent.GetObjectArray("m_aDamagePhases");
					//parentOldPhasesList = FindComponent(parentEntitySource, "SCR_DestructionMultiPhaseComponent").GetObjectArray("m_aDamagePhases");

					// first compare phases within old list to see which one is inherited and just changed, then adjust the new ones
				}

				// This prefab was handled, remove it from the list
				count--;
				resourceNames.Remove(i);
				continue;
			}
		}

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ConvertPhases(IEntitySource source, IEntityComponentSource componentSource)
	{
		array<ref SCR_DamagePhaseData> parentDamagePhases;
		BaseContainer parentContainer = source.GetAncestor();
		if (parentContainer)
		{
			parentDamagePhases = {};
			FindComponent(parentContainer.ToEntitySource(), "SCR_DestructionMultiPhaseComponent").Get("m_aDamagePhases", parentDamagePhases);
		}

		array<ref SCR_DamagePhaseData> damagePhases = {};
		componentSource.Get("m_aDamagePhases", damagePhases);
		int damagePhasesCount = damagePhases.Count();

		// Get total health of all damage phases
		float totalHealth = GetPhasesTotalHealth(componentSource, damagePhases);
		float healthNormalized = 1;
		float currentPhaseHealth;
		componentSource.Get("m_fBaseHealth", currentPhaseHealth);

		healthNormalized -= currentPhaseHealth / totalHealth; // This will define next phase health normalized

		array<ref ContainerIdPathEntry> path = {};
		bool phaseExists = false;
		for (int j = 0; j < damagePhasesCount; j++)
		{
			if (j == damagePhasesCount - 1) // Last phase
				break;

			phaseExists = false;
			if (parentDamagePhases && parentDamagePhases.IsIndexValid(j))
			{
				for (int k = parentDamagePhases.Count() - 1; k >= 0; k--)
				{
					if (parentDamagePhases[k].m_fPhaseHealth == damagePhases[j].m_fPhaseHealth)
					{
						// Same phase most likely
						phaseExists = true;
					}
				}
			}

			// Regular phase - not first, not last
			// Create damage phase only if we shouldn't override the inherited
			if (!phaseExists)
				m_API.CreateObjectArrayVariableMember(source, path, "DamagePhases", "SCR_BaseDestructionPhase", j);

			// Change path to the current damage phase
			path.Insert(new ContainerIdPathEntry("DamagePhases", j));

			// Set variables of the damage phase
			if (parentDamagePhases && parentDamagePhases.IsIndexValid(j))
			{
				if (parentDamagePhases[j].m_PhaseModel != damagePhases[j].m_PhaseModel)
					m_API.SetVariableValue(source, path, "m_sPhaseModel", damagePhases[j].m_PhaseModel);
			}

			m_API.SetVariableValue(source, path, "Threshold", healthNormalized.ToString());
			healthNormalized -= damagePhases[j].m_fPhaseHealth / totalHealth; // This will define next phase health normalized

			//m_API.ClearVariableValue(source, path, "m_aPhaseDestroySpawnObjects");

			// Use effects of the next phase as exit particles
			if (j + 1 < damagePhasesCount)
			{
				foreach (int k, SCR_BaseSpawnable spawnObject : damagePhases[j + 1].m_PhaseDestroySpawnObjects)
				{
					spawnObject.CopyToSource(m_API, source, path, k, "m_aPhaseDestroySpawnObjects");
				}
			}
			else
			{
				array<ref SCR_BaseSpawnable> destroySpawnObjects = {};
				componentSource.Get("m_DestroySpawnObjects", destroySpawnObjects);

				foreach (int k, SCR_BaseSpawnable destroySpawnObject : destroySpawnObjects)
				{
					destroySpawnObject.CopyToSource(m_API, source, path, k, "m_aPhaseDestroySpawnObjects");
				}
			}

			path.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("2 PADRN")]
	protected void PrintAllDestructibleResourceNamesButton()
	{
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "$ArmaReforger:Prefabs");

		Resource resource;
		BaseResourceObject baseResourceObject;
		BaseContainer baseContainer;
		IEntitySource entitySource;
		IEntityComponentSource destructionComponent;
		foreach (ResourceName resourceName : resourceNames)
		{
			// Validation
			resource = Resource.Load(resourceName);
			if (!resource.IsValid())
				continue;

			// Validation
			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
				continue;

			// Validation
			baseContainer = baseResourceObject.ToBaseContainer();
			if (!baseContainer)
				continue;

			// Check if old destruction is enabled
			if (!HasComponent(baseResourceObject, "SCR_DestructionMultiPhaseComponent", true))
			{
				// Old destruction is disabled, check for new one
				entitySource = baseContainer.ToEntitySource();
				if (!entitySource || entitySource.GetClassName() != "SCR_DestructibleEntity")
					continue; // Not even new destruction is enabled, so destruction is completely disabled on this object
			}

			// Has enabled destruction
			Print(resourceName, LogLevel.NORMAL);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float CalculateDensity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (!physics)
			return 0;

		array<SurfaceProperties> surfaces = {};
		GameMaterial gameMaterial;
		BallisticInfo ballisticInfo;

		float totalDensity;
		int j, densitiesCount;

		for (int i = physics.GetNumGeoms() - 1; i >= 0; i--)
		{
			surfaces.Clear();
			physics.GetGeomSurfaces(i, surfaces);

			for (j = surfaces.Count() - 1; i >= 0; i--)
			{
				gameMaterial = surfaces[j];
				if (!gameMaterial)
					continue;

				ballisticInfo = gameMaterial.GetBallisticInfo();
				if (!ballisticInfo)
					continue;

				totalDensity += ballisticInfo.GetDensity();
				densitiesCount++;
			}
		}

		if (densitiesCount != 0)
			return totalDensity / densitiesCount;
		else
			return 0;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("3 RTP")]
	protected void RevertToParentButton()
	{
		m_API.BeginEntityAction();
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "Prefabs");
		//array<ResourceName> resourceNames = {};
		//resourceNames.Insert("{AAAF45FD8D585128}Prefabs/Structures/Walls/Brick/BrickWall_02/BrickWall_02_8m.et");
		//resourceNames.Insert("{F0F75F23D500A3F2}Prefabs/Props/Agriculture/Beehive_01/Dst/Beehive_01_dst_01_blue.et");

		Resource resource;
		BaseResourceObject baseResourceObject;
		BaseContainer baseContainer;
		IEntitySource entitySource;
		IEntityComponentSource destructionComponent;
		BaseContainer parent;
		string parentValue, childValue, defaultValue;
		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			// Only in prefabs folder
			if (!resourceNames[i].Contains("Prefabs/"))
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			resource = Resource.Load(resourceNames[i]);
			if (!resource)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseContainer = baseResourceObject.ToBaseContainer();
			if (!baseContainer)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only entity type SCR_DestructibleEntity
			entitySource = baseContainer.ToEntitySource();
			if (!entitySource || entitySource.GetClassName() != "SCR_DestructibleEntity")
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only with the "old" destruction component
			if (!HasComponent(baseResourceObject, "SCR_DestructionMultiPhaseComponent", false))
			{
				resourceNames.Remove(i);
				continue;
			}

			parent = entitySource.GetAncestor();

			foreach (int j, string property : m_aProperties)
			{
				if (!entitySource.IsVariableSetDirectly(property)) // Variable is not set directly, we don't need to clear it
					continue;

				entitySource.Get(property, childValue);
				entitySource.GetDefaultAsString(property, defaultValue);

				if (resourceNames[i] == "{B31031F1F682586E}Prefabs/Props/Furniture/BenchWooden_02_Base.et")
				{
					if (property == "MaxHealth")
					{
						Print(defaultValue);
						Print(childValue);
					}
				}

				if (childValue == defaultValue) // default value is either the real default value or the inherited value from parent prefab
				{
					m_API.ClearVariableValue(entitySource, null, property); // Thus we can clear the variable when set directly
					/*if (!parent || parent.GetClassName() != "SCR_DestructibleEntity") // Has no parent or inherits the value from same type parent
					{
						m_API.ClearVariableValue(entitySource, null, property); // Thus we can clear the variable when set directly
						continue;
					}*/
				}
				if (parent && parent.GetClassName() == "SCR_DestructibleEntity") // value is not default, check if it's inherited
				{
					parent.Get(property, parentValue);
					if (childValue == parentValue)
						m_API.ClearVariableValue(entitySource, null, property);
				}
			}

			if (!parent)
				continue;

			// If there is parent prefab, clear these two variables, because we want the inherited version
			if (entitySource.IsVariableSetDirectly("FirstDestructionPhase") && parent.IsVariableSetDirectly("FirstDestructionPhase"))
				m_API.ClearVariableValue(entitySource, null, "FirstDestructionPhase");
			if (entitySource.IsVariableSetDirectly("LastDestructionPhase") && parent.IsVariableSetDirectly("LastDestructionPhase"))
				m_API.ClearVariableValue(entitySource, null, "LastDestructionPhase");
		}

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	// Adds all models to the phases destroy objects
	[ButtonAttribute("4 UP")]
	protected void UpdatePrefabsButton()
	{
		m_API.BeginEntityAction();
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "Prefabs");

		Resource resource;
		BaseResourceObject baseResourceObject;
		BaseContainer baseContainer;
		IEntitySource entitySource;
		IEntityComponentSource destructionComponent;
		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			// Only in prefabs folder
			if (!resourceNames[i].Contains("Prefabs/"))
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			resource = Resource.Load(resourceNames[i]);
			if (!resource)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseContainer = baseResourceObject.ToBaseContainer();
			if (!baseContainer)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only entity type SCR_DestructibleEntity
			entitySource = baseContainer.ToEntitySource();
			if (!entitySource || entitySource.GetClassName() != "SCR_DestructibleEntity")
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only with the "old" destruction component
			if (!HasComponent(baseResourceObject, "SCR_DestructionMultiPhaseComponent", false))
			{
				resourceNames.Remove(i);
				continue;
			}

			destructionComponent = FindComponent(entitySource, "SCR_DestructionMultiPhaseComponent");

			// Setup first phase
			array<ref ContainerIdPathEntry> path = {};
			array<ref SCR_DamagePhaseData> damagePhases = {};
			destructionComponent.Get("m_aDamagePhases", damagePhases);
			int damagePhasesCount = damagePhases.Count();

			BaseContainer phaseObject;
			path.Insert(new ContainerIdPathEntry("FirstDestructionPhase"));
			if (damagePhasesCount > 0)
			{
				// Clear the array first
				phaseObject = entitySource.GetObject("FirstDestructionPhase");
				ClearArrayOfPhaseEffects(phaseObject, path, entitySource);

				foreach (int j, SCR_BaseSpawnable spawnObject : damagePhases[0].m_PhaseDestroySpawnObjects)
				{
					spawnObject.CopyToSource(m_API, entitySource, path, j, "m_aPhaseDestroySpawnObjects");
				}
			}
			// End of first phase setup

			path.Clear();
			if (damagePhasesCount == 0) // making sure last phase is always set
			{
				array<ref SCR_BaseSpawnable> destroySpawnObjects = {};
				destructionComponent.Get("m_DestroySpawnObjects", destroySpawnObjects);
				path.Insert(new ContainerIdPathEntry("LastDestructionPhase"));

				// Clear the array first
				phaseObject = entitySource.GetObject("LastDestructionPhase");
				ClearArrayOfPhaseEffects(phaseObject, path, entitySource);

				foreach (int j, SCR_BaseSpawnable spawnObject : destroySpawnObjects)
				{
					spawnObject.CopyToSource(m_API, entitySource, path, j, "m_aPhaseDestroySpawnObjects");
				}

				path.Clear();
			}

			BaseContainerList phasesList;
			phasesList = entitySource.GetObjectArray("DamagePhases");
			for (int j = 0; j < damagePhasesCount; j++)
			{
				if (j == damagePhasesCount - 1) // Last phase
					continue;

				// Regular phase - not first, not last
				path.Insert(new ContainerIdPathEntry("DamagePhases", j));

				// Set variables of the damage phase
				// Use effects of the next phase as exit particles
				if (j + 1 < damagePhasesCount)
				{
					// Clear the array first
					phaseObject = phasesList.Get(j);
					ClearArrayOfPhaseEffects(phaseObject, path, entitySource);

					foreach (int k, SCR_BaseSpawnable spawnObject : damagePhases[j + 1].m_PhaseDestroySpawnObjects)
					{
						spawnObject.CopyToSource(m_API, entitySource, path, k, "m_aPhaseDestroySpawnObjects");
					}
				}
				else
				{
					array<ref SCR_BaseSpawnable> destroySpawnObjects = {};
					destructionComponent.Get("m_DestroySpawnObjects", destroySpawnObjects);

					// Clear the array first
					phaseObject = phasesList.Get(j);
					ClearArrayOfPhaseEffects(phaseObject, path, entitySource);

					foreach (int k, SCR_BaseSpawnable spawnObject : destroySpawnObjects)
					{
						spawnObject.CopyToSource(m_API, entitySource, path, k, "m_aPhaseDestroySpawnObjects");
					}
				}

				path.Clear();
			}

		}

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	// Removes duplicate phases (where model is the same)
	// Recalculates phase thresholds if there are some phases with the same threshold (inheritance issues after conversion)
	[ButtonAttribute("5 RP")]
	protected void RemovePhasesButton()
	{
		m_API.BeginEntityAction();
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "Prefabs");

		Resource resource;
		BaseResourceObject baseResourceObject;
		BaseContainer baseContainer;
		IEntitySource entitySource;
		IEntityComponentSource destructionComponent;
		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			// Only in prefabs folder
			if (!resourceNames[i].Contains("Prefabs/"))
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			resource = Resource.Load(resourceNames[i]);
			if (!resource)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseResourceObject = resource.GetResource();
			if (!baseResourceObject)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Validation
			baseContainer = baseResourceObject.ToBaseContainer();
			if (!baseContainer)
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only entity type SCR_DestructibleEntity
			entitySource = baseContainer.ToEntitySource();
			if (!entitySource || entitySource.GetClassName() != "SCR_DestructibleEntity")
			{
				resourceNames.Remove(i);
				continue;
			}

			// Only with the "old" destruction component
			if (!HasComponent(baseResourceObject, "SCR_DestructionMultiPhaseComponent", false))
			{
				resourceNames.Remove(i);
				continue;
			}

			array<ref ContainerIdPathEntry> path = {};
			BaseContainer phaseObject;
			BaseContainerList phasesList;
			phasesList = entitySource.GetObjectArray("DamagePhases");
			int phasesCount = phasesList.Count();
			float threshold;
			array<float> thresholds = {};
			ResourceName phaseModel;
			array<ResourceName> phaseModels = {};
			bool recalculate = false;

			for (int j = 0; j < phasesCount; j++)
			{
				phaseObject = phasesList.Get(j);
				phaseObject.Get("Threshold", threshold);

				phaseObject.Get("m_sPhaseModel", phaseModel);

				if (phaseModels.Contains(phaseModel))
				{
					m_API.RemoveObjectArrayVariableMember(entitySource, path, "DamagePhases", j);
					phasesList = entitySource.GetObjectArray("DamagePhases");
					phasesCount = phasesList.Count();
					j--;
					continue; // duplicate model == duplicate phase, just delete it
				}

				phaseModels.Insert(phaseModel);

				if (thresholds.Contains(threshold))
					recalculate = true;
				else
					thresholds.Insert(threshold);
			}

			if (!recalculate)
				continue;

			destructionComponent = FindComponent(entitySource, "SCR_DestructionMultiPhaseComponent");
			array<ref SCR_DamagePhaseData> damagePhases = {};
			destructionComponent.Get("m_aDamagePhases", damagePhases);
			float totalHealth = GetPhasesTotalHealth(destructionComponent, damagePhases);
			float healthNormalized = 1;
//			float currentPhaseHealth;

			for (int j = 0; j < phasesCount; j++)
			{
				path.Insert(new ContainerIdPathEntry("DamagePhases", j));
				healthNormalized -= damagePhases[j].m_fPhaseHealth / totalHealth;
				m_API.SetVariableValue(entitySource, path, "Threshold", healthNormalized.ToString());
				path.Clear();
			}
		}

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	void ClearArrayOfPhaseEffects(BaseContainer phaseObject, array<ref ContainerIdPathEntry> path, IEntitySource entitySource)
	{
		if (!phaseObject)
			return;

		BaseContainerList effectsList = phaseObject.GetObjectArray("m_aPhaseDestroySpawnObjects");
		if (!effectsList)
			return;

		for (int i = effectsList.Count() - 1; i >= 0; --i)
		{
			m_API.RemoveObjectArrayVariableMember(entitySource, path, "m_aPhaseDestroySpawnObjects", 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("6 FP")]
	protected void FilterPrefabsButton()
	{
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "Prefabs");

		array<string> componentClassNames = {};
		componentClassNames.Insert("SCR_DestructionMultiPhaseComponent");

		array<ResourceName> outResourceNames = {};
		FilterByComponents(resourceNames, outResourceNames, componentClassNames, false, true);

		resourceNames = {};
		componentClassNames = {};
		componentClassNames.Insert("ActionsManagerComponent");
		FilterByComponents(outResourceNames, resourceNames, componentClassNames, true);

		Print("" + resourceNames.Count() + " resources", LogLevel.NORMAL);
		foreach (ResourceName resourceName : resourceNames)
		{
			if (resourceName.Contains("Glass") || resourceName.Contains("Lamp"))
				continue;

			int startIndex = resourceName.IndexOf("{") + 1;
			int endIndex = resourceName.IndexOf("}");
			int length = endIndex - startIndex;
			Print(resourceName.Substring(startIndex, length));
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("7 SM")]
	protected void ShowMassButton()
	{
		IEntitySource entitySource = m_API.GetSelectedEntity();
		IEntity entity = m_API.SourceToEntity(entitySource);
		float volume = MeshObjectVolumeCalculator.GetVolumeFromColliders(entity, EPhysicsLayerDefs.FireGeometry);
		float density = CalculateDensity(entity);
		Print(density * 1000 * volume);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("8 CP")]
	protected void ConvertPrefabsButton()
	{
		m_API.BeginEntityAction();
		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS, rootPath: "Prefabs");

		array<string> componentClassNames = {};
		componentClassNames.Insert("SCR_DestructionMultiPhaseComponent");
		//componentClassNames.Insert("");
		array<ResourceName> outResourceNames = {};
		//FilterByComponents(resourceNames, outResourceNames, componentClassNames);
		FilterByClass(resourceNames, outResourceNames, "SCR_DestructibleEntity");

		Resource resource;
		BaseResourceObject baseResource;
		IEntitySource source;
		IEntityComponentSource componentSource;
		BaseContainer defaultHitZone;
		array<ref ContainerIdPathEntry> path = {};
		bool deleteAfterFinalPhase = false;
		SCR_EMaterialSoundTypeBreak materialSoundType;
		float momentumToDamage;
		foreach (ResourceName resourceName : outResourceNames)
		{
			resource = Resource.Load(resourceName);
			if (!resource) // test
				continue;

			baseResource = resource.GetResource();
			source = baseResource.ToEntitySource();

			for (int j = source.GetComponentCount() - 1; j >= 0; j--)
			{
				componentSource = source.GetComponent(j);
				if (componentSource.GetClassName() != DESTRUCTION_COMPONENT_CLASS)
					continue;

				break;
			}

			if (!componentSource)
				continue;

			if (source.GetClassName() != "SCR_DestructibleEntity")
			{
				Print("The following prefab is not SCR_DestructibleEntity type!");
				Print("Cannot convert!");
				Print(source.GetClassName());
				Print(source.GetResourceName());
				continue;
			}

			array<ref SCR_DamagePhaseData> damagePhases = {};
			componentSource.Get("m_aDamagePhases", damagePhases);

			array<ref SCR_BaseDestructionPhase> newPhases = {};
			SCR_BaseDestructionPhase newPhase;
			for (int j = damagePhases.Count() - 1; j >= 0; j--)
			{
				newPhase = new SCR_BaseDestructionPhase();
				newPhase.m_sPhaseModel = damagePhases[j].m_PhaseModel;
				newPhase.m_aPhaseDestroySpawnObjects = {};
				CopySpawnObjectsArray(newPhase.m_aPhaseDestroySpawnObjects, damagePhases[j].m_PhaseDestroySpawnObjects);

				//calculate HP and set the percentage
				newPhases.Insert(newPhase);
			}

			// Reset path
			path = {};

			// Copy damage multipliers from hitzones
			defaultHitZone = GetHitZone(source);
			if (defaultHitZone)
			{
				float multiplier;
				defaultHitZone.Get("Collision multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Collision multiplier", multiplier.ToString());

				defaultHitZone.Get("Melee multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Melee multiplier", multiplier.ToString());

				defaultHitZone.Get("Kinetic multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Kinetic multiplier", multiplier.ToString());

				defaultHitZone.Get("Fragmentation multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Fragmentation multiplier", multiplier.ToString());

				defaultHitZone.Get("Explosive multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Explosive multiplier", multiplier.ToString());

				defaultHitZone.Get("Incendiary multiplier", multiplier);
				m_API.SetVariableValue(source, path, "Incendiary multiplier", multiplier.ToString());

				defaultHitZone.Get("BaseDamageMultiplier", multiplier);
				m_API.SetVariableValue(source, path, "BaseDamageMultiplier", multiplier.ToString());

				defaultHitZone.Get("DamageReduction", multiplier);
				m_API.SetVariableValue(source, path, "DamageReduction", multiplier.ToString());

				defaultHitZone.Get("DamageThreshold", multiplier);
				m_API.SetVariableValue(source, path, "DamageThreshold", multiplier.ToString());
			}
			// End of copying multipliers

			// Copy other relevant attributes
			componentSource.Get("m_bDeleteAfterFinalPhase", deleteAfterFinalPhase);
			m_API.SetVariableValue(source, path, "DestroyAtNoHealth", deleteAfterFinalPhase.ToString(true));

			componentSource.Get("m_eMaterialSoundType", materialSoundType);
			m_API.SetVariableValue(source, path, "m_eMaterialSoundType", materialSoundType.ToString());

			componentSource.Get("m_fMomentumToDamageScale", momentumToDamage);
			m_API.SetVariableValue(source, path, "m_fMomentumToDamageScale", momentumToDamage.ToString());
			// End

			// Setup first phase
			int damagePhasesCount = damagePhases.Count();
			IEntityComponentSource meshObject = FindComponent(source, "MeshObject");
			ResourceName firstPhaseResourceName;
			if (meshObject)
				meshObject.Get("Object", firstPhaseResourceName);

			m_API.CreateObjectVariableMember(source, path, "FirstDestructionPhase", "SCR_BaseDestructionPhase");

			path.Insert(new ContainerIdPathEntry("FirstDestructionPhase"));
			m_API.SetVariableValue(source, path, "m_sPhaseModel", firstPhaseResourceName);

			if (damagePhasesCount > 0)
			{
				foreach (int k, SCR_BaseSpawnable spawnObject : damagePhases[0].m_PhaseDestroySpawnObjects)
				{
					spawnObject.CopyToSource(m_API, source, path, k, "m_aPhaseDestroySpawnObjects");
				}
			}
			// End of first phase setup

			// Get total health of all damage phases
			float totalHealth = GetPhasesTotalHealth(componentSource, damagePhases);
			float healthNormalized = 1;
			float currentPhaseHealth;
			componentSource.Get("m_fBaseHealth", currentPhaseHealth);

			healthNormalized -= currentPhaseHealth / totalHealth; // This will define next phase health normalized

			path = {};
			m_API.SetVariableValue(source, path, "MaxHealth", totalHealth.ToString());
			if (damagePhasesCount == 0) // making sure last phase is always set
			{
				m_API.CreateObjectVariableMember(source, path, "LastDestructionPhase", "SCR_BaseDestructionPhase");

				array<ref SCR_BaseSpawnable> destroySpawnObjects = {};
				componentSource.Get("m_DestroySpawnObjects", destroySpawnObjects);
				path.Insert(new ContainerIdPathEntry("LastDestructionPhase"));

				foreach (int j, SCR_BaseSpawnable spawnObject : destroySpawnObjects)
				{
					spawnObject.CopyToSource(m_API, source, path, j, "m_aPhaseDestroySpawnObjects");
				}

				m_API.SetVariableValue(source, path, "Threshold", "0");

				path.Clear();
			}

			for (int j = 0; j < damagePhasesCount; j++)
			{
				if (j == damagePhasesCount - 1) // Last phase
				{
					// Change path to the current damage phase
					m_API.CreateObjectVariableMember(source, path, "LastDestructionPhase", "SCR_BaseDestructionPhase");

					path.Insert(new ContainerIdPathEntry("LastDestructionPhase"));
					healthNormalized = 0;
				}
				else //Regular phase - not first, not last
				{
					// Create damage phase
					m_API.CreateObjectArrayVariableMember(source, path, "DamagePhases", "SCR_BaseDestructionPhase", j);

					// Change path to the current damage phase
					path.Insert(new ContainerIdPathEntry("DamagePhases", j));
				}

				// Set variables of the damage phase
				m_API.SetVariableValue(source, path, "m_sPhaseModel", damagePhases[j].m_PhaseModel);

				m_API.SetVariableValue(source, path, "Threshold", healthNormalized.ToString());
				healthNormalized -= damagePhases[j].m_fPhaseHealth / totalHealth; // This will define next phase health normalized

				// Use effects of the next phase as exit particles
				if (j + 1 < damagePhasesCount)
				{
					foreach (int k, SCR_BaseSpawnable spawnObject : damagePhases[j + 1].m_PhaseDestroySpawnObjects)
					{
						spawnObject.CopyToSource(m_API, source, path, k, "m_aPhaseDestroySpawnObjects");
					}
				}
				else
				{
					array<ref SCR_BaseSpawnable> destroySpawnObjects = {};
					componentSource.Get("m_DestroySpawnObjects", destroySpawnObjects);

					foreach (int k, SCR_BaseSpawnable spawnObject : destroySpawnObjects)
					{
						spawnObject.CopyToSource(m_API, source, path, k, "m_aPhaseDestroySpawnObjects");
					}
				}

				path.Clear();
			}

			// Deletes the now no longer used destruction component
			path.Insert(new ContainerIdPathEntry("SCR_DestructionMultiPhaseComponent"));
			m_API.SetVariableValue(source, path, "Enabled", "0");
			path.Clear();

			path.Insert(new ContainerIdPathEntry("RplComponent"));
			m_API.SetVariableValue(source, path, "Enabled", "0");
			path.Clear();
		}

		m_API.EndEntityAction();
	}

	//------------------------------------------------------------------------------------------------
	BaseContainer GetHitZone(IEntitySource source)
	{
		IEntityComponentSource componentSource;
		for (int i = source.GetComponentCount() - 1; i >= 0; i--)
		{
			componentSource = source.GetComponent(i);
			if (componentSource.GetClassName() != DESTRUCTION_COMPONENT_CLASS)
				continue;

			break;
		}

		BaseContainerList hitZonesList = componentSource.GetObjectArray("Additional hit zones");
		if (!hitZonesList || hitZonesList.Count() == 0)
			return null;

		return hitZonesList[0];
	}

	//------------------------------------------------------------------------------------------------
	float GetPhasesTotalHealth(IEntityComponentSource componentSource, array<ref SCR_DamagePhaseData> phases)
	{
		float health;
		componentSource.Get("m_fBaseHealth", health);

		foreach (SCR_DamagePhaseData phase : phases)
		{
			health += phase.m_fPhaseHealth;
		}

		return health;
	}

	//------------------------------------------------------------------------------------------------
	IEntityComponentSource FindComponent(IEntitySource source, string componentType)
	{
		for (int i = source.GetComponentCount() - 1; i >= 0; i--)
		{
			if (source.GetComponent(i).GetClassName() == componentType)
				return source.GetComponent(i);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void CopySpawnObjectsArray(out array<ref SCR_BaseSpawnable> to, array<ref SCR_BaseSpawnable> from)
	{
		foreach (SCR_BaseSpawnable object : from)
		{
			to.Insert(object);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasComponent(BaseResourceObject baseResource, string componentClassName, bool checkEnabled)
	{
		IEntitySource source = baseResource.ToEntitySource();
		string currentComponentClassName;
		for (int i = source.GetComponentCount() - 1; i >= 0; i--)
		{
			currentComponentClassName = source.GetComponent(i).GetClassName();
			if (currentComponentClassName == componentClassName)
			{
				if (!checkEnabled)
					return true;
				else
				{
					bool enabled;
					source.GetComponent(i).Get("Enabled", enabled);

					return enabled;
				}
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterByClass(notnull array<ResourceName> resourceNames, notnull out array<ResourceName> outResourceNames, string className)
	{
		Resource resource;
		BaseResourceObject baseResource;
		IEntitySource source;
		foreach (ResourceName resourceName : resourceNames)
		{
			resource = Resource.Load(resourceName);
			if (!resource)
				continue;

			baseResource = resource.GetResource();
			if (!baseResource)
				continue;

			source = baseResource.ToEntitySource();
			if (source && source.GetClassName() == className)
				outResourceNames.Insert(resourceName);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void FilterByComponents(notnull array<ResourceName> resourceNames, notnull out array<ResourceName> outResourceNames, notnull array<string> componentClassNames, bool removeWithComponent = false, bool checkEnabled = false)
	{
		Resource resource;
		BaseResourceObject baseResource;
		IEntitySource entitySource;
		IEntityComponentSource component;

		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			resource = Resource.Load(resourceNames[i]);
			baseResource = resource.GetResource();
			if (!baseResource)
			{
				resourceNames.Remove(i);
				continue;
			}

			entitySource = baseResource.ToEntitySource();

			if (!entitySource || entitySource.GetNumChildren() > 0 || entitySource.GetParent())
			{
				resourceNames.Remove(i);
				continue;
			}

			for (int j = componentClassNames.Count() - 1; j >= 0; j--)
			{
				if (HasComponent(baseResource, componentClassNames[j], checkEnabled))
				{
					if (removeWithComponent)
						continue;
					else
						outResourceNames.Insert(resourceNames[i]);
				}
				else
				{
					if (removeWithComponent)
						outResourceNames.Insert(resourceNames[i]);
					else
						continue;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("9 AI")]
	protected void AssignIndicesButton()
	{
		Debug.BeginTimeMeasure();

		array<ResourceName> resourceNames = SCR_WorkbenchHelper.SearchWorkbenchResources(EXTENSIONS);

		Resource resource;
		BaseResourceObject baseResource;
		IEntitySource source;
		IEntitySource runtimeSource;
		string componentClassName;
		IEntity entity;
		int componentCount;
		int j, k, l;
		bool hasDestruction, hasPhysics;
		int componentClassesCount = DESTRUCTIBLE_COMPONENT_CLASSES.Count();
		float density;
		float volume;
		float mass;
		int pairsCount = m_aPairs.Count();
		string indexName;

		IEntityComponentSource physicsSource;

		array<ref ContainerIdPathEntry> entryPath = {ContainerIdPathEntry(PHYSICS_COMPONENT)};

		m_API.BeginEntityAction();

		for (int i = resourceNames.Count() - 1; i >= 0; i--)
		{
			hasDestruction = false;
			hasPhysics = false;
			resource = Resource.Load(resourceNames[i]);
			baseResource = resource.GetResource();
			if (!baseResource)
			{
				resourceNames.Remove(i);
				continue;
			}

			source = baseResource.ToEntitySource();
			componentCount = source.GetComponentCount();
			for (j = componentCount - 1; j >= 0; j--)
			{
				componentClassName = source.GetComponent(j).GetClassName();
				if (!hasPhysics && componentClassName == PHYSICS_COMPONENT)
					hasPhysics = true;

				for (k = componentClassesCount - 1; k >= 0; k--)
				{
					if (componentClassName == DESTRUCTIBLE_COMPONENT_CLASSES[k])
					{
						physicsSource = source.GetComponent(j);
						hasDestruction = true;
						break;
					}
				}

				if (hasPhysics && hasDestruction)
					break;
			}

			if (!hasDestruction || !hasPhysics)
				continue;
			
			runtimeSource = m_API.CreateEntity(resourceNames[i], "", 0, null, vector.Zero, vector.Zero);
			if (!runtimeSource)
				continue;

			entity = m_API.SourceToEntity(runtimeSource);

			volume = MeshObjectVolumeCalculator.GetVolumeFromColliders(entity, EPhysicsLayerDefs.FireGeometry);

			density = CalculateDensity(entity);

			mass = density * 1000 * volume; // 1000 g/cm3 -> kg/m3

			for (l = 0; l < pairsCount; l++)
			{
				if (mass < m_aPairs[l].m_fMass)
				{
					indexName = m_aPairs[l].m_sIndexName;
					break;
				}
				else if (l == pairsCount - 1) // last pair, mass is bigger than last entry -> set max response index
				{
					indexName = SCR_DamageManagerComponent.MAX_DESTRUCTION_RESPONSE_INDEX_NAME;
				}
			}

			//Edit prefab here
			m_API.SetVariableValue(source, entryPath, "ResponseIndex", indexName);

			m_API.DeleteEntity(runtimeSource);
		}

		m_API.EndEntityAction();

		Debug.EndTimeMeasure("Assigning done");
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("10 SBDE")]
	protected void SetBuildingDestructionEffectButton()
	{
		// TODO
	}
}

[BaseContainerProps()]
class SCR_MassResponseIndexPair
{
	[Attribute(desc: "Objects with smaller weight will have this index. [kg]")]
	float m_fMass;

	[Attribute(desc: "Refer to physics settings of the project.")]
	int m_iIndex;

	[Attribute(desc: "Refer to physics settings of the project.")]
	string m_sIndexName;
}
#endif // WORKBENCH
