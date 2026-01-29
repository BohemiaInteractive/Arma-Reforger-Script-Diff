//! \see BaseContainerTools
//! \see SCR_ConfigHelper
class SCR_BaseContainerTools
{
	//------------------------------------------------------------------------------------------------
	//! Get Managed Instance from prefab
	//! \param[in] prefab Prefab ResourceName
	//! \param[in] printError If true will print an error if it fails
	//! \return Managed entity or null on error
	static Managed CreateInstanceFromPrefab(ResourceName prefab, bool printError = false)
	{
		Resource resource = Resource.Load(prefab);
		if (!resource.IsValid())
		{
			if (printError)
				PrintFormat("[SCR_BaseContainerTools.CreateInstanceFromPrefab] failed '%1' at the Resource.Load step!", prefab, level: LogLevel.ERROR);

			return null;
		}

		BaseContainer baseContainer = resource.GetResource().ToBaseContainer();
		if (!baseContainer)
		{
			if (printError)
				PrintFormat("[SCR_BaseContainerTools.CreateInstanceFromPrefab] failed '%1' at the BaseContainer step!", prefab, level: LogLevel.ERROR);

			return null;
		}

		Managed managed = BaseContainerTools.CreateInstanceFromContainer(baseContainer);
		if (!managed)
		{
			if (printError)
				PrintFormat("[SCR_BaseContainerTools.CreateInstanceFromPrefab] failed '%1' create instance step!", prefab, level: LogLevel.ERROR);

			return null;
		}

		return managed;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	static bool SaveInstanceToResourceName(notnull Managed instance, ResourceName resourceName, string filePath = "", bool printError = false)
	{
		if (!resourceName && !filePath) // .IsEmpty()
		{
			Print("[SCR_BaseContainerTools.SaveInstanceToPrefab] Provided resourceName is an empty ResourceName and filePath is an empty string", level: LogLevel.ERROR);
			return false;
		}

		Resource resource = BaseContainerTools.CreateContainerFromInstance(instance);
		if (!resource || !resource.IsValid())
		{
			if (printError)
				Print("[SCR_BaseContainerTools.SaveInstanceToPrefab] Provided instance cannot be converted to container", LogLevel.ERROR);

			return false;
		}

		if (!BaseContainerTools.SaveContainer(resource.GetResource().ToBaseContainer(), resourceName, filePath))
		{
			if (printError)
				PrintFormat("[SCR_BaseContainerTools.SaveInstanceToPrefab] Cannot save instance to resourceName/filePath (%1/%2)", resourceName, filePath, level: LogLevel.ERROR);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get class name of the prefab.
	//! Use to check if prefab class is matching requirements before you spawn it.
	//! \param[in] prefab Prefab path
	//! \return Class name
	static string GetContainerClassName(ResourceName prefab)
	{
		return GetContainerClassName(Resource.Load(prefab));
	}

	//------------------------------------------------------------------------------------------------
	//! Get class name of prefab resource.
	//! Use to check if prefab class is matching requirements before you spawn it.
	//! \param[in] prefab Prefab resource
	//! \return Class name
	static string GetContainerClassName(Resource prefabResource)
	{
		if (!prefabResource || !prefabResource.IsValid())
			return string.Empty;

		BaseResourceObject prefabContainer = prefabResource.GetResource();
		if (!prefabContainer)
			return string.Empty;

		BaseContainer prefabBase = prefabContainer.ToBaseContainer();
		if (!prefabBase)
			return string.Empty;

		return prefabBase.GetClassName();
	}

	//------------------------------------------------------------------------------------------------
	//! Get entity source from prefab resource.
	//! \param[in] prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	//! \return Entity source or null if not found or on error
	static IEntitySource FindEntitySource(Resource prefabResource)
	{
		if (!prefabResource || !prefabResource.IsValid())
			return null;

		BaseResourceObject prefabBase = prefabResource.GetResource();
		if (!prefabBase)
			return null;

		return prefabBase.ToEntitySource();
	}

	//------------------------------------------------------------------------------------------------
	//! Get component source of given class from prefab resource.
	//! \param[in] prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	//! \param[in] componentClassName Class name of desired component
	//! \return Component source or null if not found or on error
	static IEntityComponentSource FindComponentSource(Resource prefabResource, string componentClassName)
	{
		if (!prefabResource || !prefabResource.IsValid())
			return null;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return null;

		return FindComponentSource(prefabEntity, componentClassName);
	}

	//------------------------------------------------------------------------------------------------
	//! Get component source inherited from given class from prefab resource.
	//! \param[in] prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	//! \param[in] componentClassName Class of desired component
	//! \return Component source
	static IEntityComponentSource FindComponentSource(Resource prefabResource, typename componentClass)
	{
		if (!prefabResource || !prefabResource.IsValid())
			return null;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return null;

		return FindComponentSource(prefabEntity, componentClass);
	}

	//------------------------------------------------------------------------------------------------
	//! Get component index in entity source.
	//! \param[in] entitySource Entity source
	//! \param[in] componentClassName Class name of desired component
	//! \return 0-based index, -1 if not found
	static int FindComponentIndex(IEntitySource entitySource, string componentClassName)
	{
		if (!entitySource)
			return -1;

		int componentsCount = entitySource.GetComponentCount();
		for (int i; i < componentsCount; i++)
		{
			IEntityComponentSource componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName() == componentClassName)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get component index in entity source.
	//! \param[in] entitySource Entity source
	//! \param[in] componentClass Class of desired component
	//! \return 0-based index, -1 if not found
	static int FindComponentIndex(IEntitySource entitySource, typename componentClass)
	{
		if (!entitySource)
			return -1;

		int componentsCount = entitySource.GetComponentCount();
		IEntityComponentSource componentSource;
		for (int i; i < componentsCount; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType() &&
				componentSource.GetClassName().ToType().IsInherited(componentClass))
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get component source of given class from entity source.
	//! \param[in] prefabEntity Entity source
	//! \param[in] componentClassName Class name of desired component
	//! \return Component source or null if not found
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, string componentClassName)
	{
		if (!prefabEntity)
			return null;

		IEntityComponentSource componentSource;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName() == componentClassName)
				return componentSource;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get component source inherited from given class from entity source.
	//! \param[in] prefabEntity Entity source
	//! \param[in] componentClass Class of desired component
	//! \return Component source or null if not found
	static IEntityComponentSource FindComponentSource(IEntitySource prefabEntity, typename componentClass)
	{
		if (!prefabEntity)
			return null;

		IEntityComponentSource componentSource;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);
			if (componentSource.GetClassName().ToType() &&
				componentSource.GetClassName().ToType().IsInherited(componentClass))
				return componentSource;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get component sources of given classes from prefab resource.
	//! \param[in] prefabResource Loaded entity prefab, use Resource.Load(prefab) to retrieve it from ResourceName
	//! \param[in] componentClassNames Class names of desired components
	//! \param[out] Array to be filled with component sources (in the same order as componentClassNames)
	//! \return Number of components
	static int FindComponentSources(Resource prefabResource, notnull array<string> componentClassNames, notnull out array<ref array<IEntityComponentSource>> componentSources)
	{
		int classNamesCount = componentClassNames.Count();
		componentSources.Clear();
		componentSources.Resize(classNamesCount);

		if (!prefabResource || !prefabResource.IsValid())
			return classNamesCount;

		IEntitySource prefabEntity = FindEntitySource(prefabResource);
		if (!prefabEntity)
			return classNamesCount;

		return FindComponentSources(prefabEntity, componentClassNames, componentSources);
	}

	//------------------------------------------------------------------------------------------------
	//! Get component sources of given classes from entity source.
	//! \param[in] prefabEntity Entity source
	//! \param[in] componentClassNames Class names of desired components
	//! \param[out] Array to be filled with component sources (in the same order as componentClassNames)
	//! \return Number of components
	static int FindComponentSources(IEntitySource prefabEntity, notnull array<string> componentClassNames, notnull out array<ref array<IEntityComponentSource>> componentSources)
	{
		int classNamesCount = componentClassNames.Count();
		componentSources.Clear();
		componentSources.Resize(classNamesCount);

		if (!prefabEntity)
			return classNamesCount;

		IEntityComponentSource componentSource;
		array<IEntityComponentSource> components;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);
			string componentClassName = componentSource.GetClassName();
			for (int j = 0; j < classNamesCount; j++)
			{
				if (componentClassName == componentClassNames[j])
				{
					components = componentSources[j];
					if (!components)
					{
						components = {};
						componentSources.Set(j, components);
					}

					components.Insert(componentSource);
					break;
				}
			}
		}

		return classNamesCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all component sources of given class from entity source.
	//! \param[in] prefabEntity Entity source
	//! \param[in] componentClass Class names of desired components
	//! \param[in] GetChildComponentsOfComponents if true it will itterate over all children of the component (not entity) and get the components from it. Will search two layers deep
	//! \param[out] componentSources Array filled with the found IEntityComponentSource of given class
	//! \return Number of components found
	static int FindComponentSourcesOfClass(IEntitySource prefabEntity, typename componentClass, bool GetChildComponentsOfComponents, notnull out array<IEntityComponentSource> componentSources)
	{
		componentSources.Clear();
		if (!prefabEntity)
			return 0;

		IEntityComponentSource componentSource;
		array<IEntityComponentSource> components;
		for (int i, componentsCount = prefabEntity.GetComponentCount(); i < componentsCount; i++)
		{
			componentSource = prefabEntity.GetComponent(i);

			if (componentSource.GetClassName().ToType().IsInherited(componentClass))
				componentSources.Insert(componentSource);

			//~ Search children of components and the children of components children
			if (GetChildComponentsOfComponents)
			{
				array<IEntityComponentSource> componentSourceChildren = {};
				array<IEntityComponentSource> componentSourceChildChildren = {};

				//~ Search children of components
				if (GetComponentSourceChildren(componentSource, componentSourceChildren) > 0)
				{
					foreach (IEntityComponentSource componentSourceChild : componentSourceChildren)
					{
						if (componentSourceChild.GetClassName().ToType().IsInherited(componentClass))
							componentSources.Insert(componentSourceChild);

						//~ Search children of components children
						if (GetComponentSourceChildren(componentSourceChild, componentSourceChildChildren) > 0)
						{
							foreach (IEntityComponentSource componentSourceChildChild : componentSourceChildChildren)
							{
								if (componentSourceChildChild.GetClassName().ToType().IsInherited(componentClass))
									componentSources.Insert(componentSourceChildChild);
							}
						}
					}
				}
			}
		}

		return componentSources.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get all component sources children from given component source
	//! \param[in] componentSource Component source
	//! \param[out] componentSources Array filled with the found IEntityComponentSource
	//! \return Number of components found
	static int GetComponentSourceChildren(notnull IEntityComponentSource componentSource, notnull out array<IEntityComponentSource> componentSources)
	{
		componentSources.Clear();

		BaseContainerList containerList = componentSource.GetObjectArray("components");
		if (!containerList)
			return 0;

		for (int i, count = containerList.Count(); i < count; i++)
		{
			componentSource = containerList.Get(i);
			if (!componentSource)
				continue;

			componentSources.Insert(componentSource);
		}

		return componentSources.Count();
	}

//	//------------------------------------------------------------------------------------------------
//	//! Check if the container contains any changes as opposed to its ancestor
//	//! \param[in] container Queried container
//	//! \return True if the container has been modified
//	static bool IsChanged(BaseContainer container)
//	{
//		string varName;
//		BaseContainerList objectArray;
//		for (int i = 0, varsCount = container.GetNumVars(); i < varsCount; i++)
//		{
//			varName = container.GetVarName(i);
//			if (container.GetObject(varName))
//			{
//				if (IsChanged(container.GetObject(varName)))
//					return true;
//			}
//			else if (container.GetObjectArray(varName))
//			{
//				objectArray = container.GetObjectArray(varName);
//				for (int a = 0, arrayCount = objectArray.Count(); a < arrayCount; a++)
//				{
//					if (IsChanged(objectArray.Get(a)))
//						return true;
//				}
//			}
//			else
//			{
//				//PrintFormat("%1: %2: %3", container.GetClassName(), varName, container.IsVariableSetDirectly(varName));
//				if (container.IsVariableSetDirectly(varName))
//					return true;
//			}
//		}
//		return false;
//	}

	//------------------------------------------------------------------------------------------------
	//! Get first container in hierarchy that comes from a prefab.
	//! \param[in] container Evaluated container
	//! \return Container (or the input container when there is no prefab in the hierarchy)
	static BaseContainer GetPrefabContainer(BaseContainer container)
	{
		BaseContainer ancestor = container;
		while (ancestor)
		{
			if (container.GetResourceName().Contains("/"))
				return ancestor;

			ancestor = ancestor.GetAncestor();
		}

		return container;
	}

	//------------------------------------------------------------------------------------------------
	//! Get resource name of the first container in hierarchy that comes from a prefab.
	//! \param[in] container Evaluated container
	//! \return Prefab resource name (or empty string when no prefab was found in the hierarchy)
	static ResourceName GetPrefabResourceName(BaseContainer container)
	{
		while (container)
		{
			if (container.GetResourceName().Contains("/"))
				return container.GetResourceName();

			container = container.GetAncestor();
		}

		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Get a Prefab's modifications from its parent
	//! \param[in] sourcePrefab
	//! \return Prefab-set variable names without parent Prefab values
	static array<string> GetPrefabSetValueNames(notnull BaseContainer baseContainer)
	{
		array<string> result = {};
		for (int i, count = baseContainer.GetNumVars(); i < count; i++)
		{
			string varName = baseContainer.GetVarName(i);
			if (baseContainer.IsVariableSetDirectly(varName))
				result.Insert(varName);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the topmost BaseContainer parent
	//! If it is the topmost, returns the provided baseContainer
	//! \param[in] baseContainer
	//! \return topmost ancestor (e.g tree base)
	static BaseContainer GetTopMostAncestor(notnull BaseContainer baseContainer)
	{
		BaseContainer ancestorContainer = baseContainer.GetAncestor();
		if (!ancestorContainer)
			return baseContainer;

		while (ancestorContainer.GetAncestor())
		{
			ancestorContainer = ancestorContainer.GetAncestor();
		}

		return ancestorContainer;
	}

	//------------------------------------------------------------------------------------------------
	//! Says if the provided container is of type or inherits from a resourcename
	//! \param[in] container the container to check
	//! \param[in] resourceName the supposed ancestor
	//! \return true if the provided container has resourceName in its ancestry, false if not or if the provided resourceName is empty
	static bool IsKindOf(notnull BaseContainer container, ResourceName resourceName)
	{
		if (resourceName.IsEmpty())
			return false;

		while (container)
		{
			if (container.GetResourceName() == resourceName)
				return true;

			container = container.GetAncestor();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Says if the provided resourcename is of type or inherits from another resourcename
	//! \param[in] checkedResourceName the Prefab to check
	//! \param[in] supposedAncestor the supposed ancestor
	//! \return true if the provided container has resourceName in its ancestry, false if not or if the provided resourceName is empty
	static bool IsKindOf(ResourceName checkedResourceName, ResourceName supposedAncestor)
	{
		if (checkedResourceName.IsEmpty())
			return false;

		Resource resource = Resource.Load(checkedResourceName);
		if (!resource.IsValid())
			return string.Empty;

		BaseResourceObject container = resource.GetResource();
		if (!container)
			return string.Empty;

		BaseContainer prefabBase = container.ToBaseContainer();
		if (!prefabBase)
			return string.Empty;

		return IsKindOf(prefabBase, supposedAncestor);
	}

	//------------------------------------------------------------------------------------------------
	//! Get world coordinates of position local to given entity source.
	//! \param[in] entitySource Entity source
	//! \param[in] coords Local coordinates
	//! \return World coordinates
	static vector GetWorldCoords(IEntitySource entitySource, vector coords)
	{
		vector coordsEntity;
		while (entitySource)
		{
			if (entitySource.Get("coords", coordsEntity))
				coords += coordsEntity;

			entitySource = entitySource.GetParent();
		}

		return coords;
	}

	//------------------------------------------------------------------------------------------------
	//! Get coordinates local to given entity source.
	//! \param[in] entitySource Entity source
	//! \param[in] coords World coordinates
	//! \return Local coordinates
	static vector GetLocalCoords(IEntitySource entitySource, vector coords)
	{
		vector coordsEntity;
		while (entitySource)
		{
			if (entitySource.Get("coords", coordsEntity))
				coords -= coordsEntity;

			entitySource = entitySource.GetParent();
		}

		return coords;
	}

	//------------------------------------------------------------------------------------------------
	//! Convert integer array to string in format acceptable by WorldEditorAPI.SetVariableValue
	//! \param[in] values Input array
	//! \return Array converted to string
	static string GetArrayValue(notnull array<int> values)
	{
		string result;
		foreach (int i, int value : values)
		{
			if (i > 0)
				result += ",";

			result += value.ToString();
		}

		return result;
	}
}
