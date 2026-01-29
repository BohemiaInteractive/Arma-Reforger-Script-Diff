#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Maintain Editable Entities", category: "In-game Editor", wbModules: { "WorldEditor", "ResourceManager" }, awesomeFontCode: 0xF0AD)]
class SCR_EditableEntityMaintenancePlugin : WorkbenchPlugin
{
	[Attribute("ArmaReforger")]
	protected string m_sAddon;

	[Attribute("PrefabsEditable/Auto", params: "unregFolders")]
	protected ResourceName m_sDirectory;

	//[Attribute("MeshObject RigidBody RplComponent Hierarchy SCR_DestructionMultiPhaseComponent")]
	//private ref array<string> m_aCheckedComponents;

	protected static const ref array<string> UNIQUE_COMPONENTS = { "MeshObject", "RigidBody", "RplComponent", "Hierarchy", "SCR_DestructionMultiPhaseComponent" };

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		string uniqueComponentClasses;
		foreach (int i, string uniqueComponent : UNIQUE_COMPONENTS)
		{
			if (i > 0)
				uniqueComponentClasses += "\n";
			uniqueComponentClasses += "  - " + uniqueComponent;
		}

		if (!Workbench.ScriptDialog("Maintain Editable Entities", string.Format("Go through all entity prefabs in the folder and remove duplicate components of following types:\n%1", uniqueComponentClasses), this))
			return;

		array<ResourceName> resources = {};
		string path = SCR_AddonTool.ToFileSystem(m_sAddon) + m_sDirectory.GetPath();
		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.fileExtensions = { "et" };
		filter.rootPath = path;
		ResourceDatabase.SearchResources(filter, resources.Insert);

		int repairedCount;
		Resource resource;
		IEntitySource entitySource, ancestorSource;
		BaseContainerList componentsArray, ancestorComponentsArray;
		array<BaseContainer> ancestorComponents = {};
		array<BaseContainer> componentsToRemove = {};
		array<string> componentClasses = {};
		string componentClass;
		IEntityComponentSource component;
		foreach (ResourceName prefab : resources)
		{
			resource = Resource.Load(prefab);
			if (!resource.IsValid())
			{
				PrintFormat("Cannot load %1 | %2:%3", prefab + FilePath.StripPath(__FILE__), __LINE__, level: LogLevel.WARNING);
				return;
			}

			entitySource = resource.GetResource().ToEntitySource();

			componentsToRemove.Clear();
			componentClasses.Clear();

			//--- Get ancestor components
			ancestorSource = entitySource.GetAncestor();
			if (ancestorSource)
			{
				ancestorComponentsArray = ancestorSource.GetObjectArray("components");
				for (int i = 0, count = ancestorComponentsArray.Count(); i < count; i++)
				{
					ancestorComponents.Insert(ancestorComponentsArray.Get(i));
				}
			}

			//--- Get prefab's components
			componentsArray = entitySource.SetObjectArray("components");
			int componentCount = componentsArray.Count();
			for (int i = 0; i < componentCount; i++)
			{
				component = componentsArray.Get(i);
				componentClass = component.GetClassName();

				if (componentClasses.Contains(componentClass) && UNIQUE_COMPONENTS.Contains(componentClass) && !ancestorComponents.Contains(component))
				{
					//--- Duplicate component which is on the list of checked components, and is also defined in this prefab and not in ancestor
					componentsToRemove.Insert(component);
				}
				else
				{
					//--- Not duplicate
					componentClasses.Insert(componentClass);
				}
			}

			if (!componentsToRemove.IsEmpty())
			{
				foreach (BaseContainer componentToRemove : componentsToRemove)
				{
					Print(string.Format("@\"%1\": Removed duplicate component %2", entitySource.GetResourceName().GetPath(), componentToRemove.GetClassName()), LogLevel.WARNING);
					componentsArray.Remove(componentToRemove);
				}

				BaseContainerTools.SaveContainer(entitySource, prefab);
				repairedCount++;
			}
		}

		Print(string.Format("%1 prefabs checked, %2 repaired.", resources.Count(), repairedCount), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Run")]
	protected bool ButtonRun()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return false;
	}
}
#endif // WORKBENCH
