[BaseContainerProps(configRoot: true), SCR_PlaceableEntitiesRegistryTitleField()]
/** @ingroup Editor_Containers_Root
*/

/*!
List of entities that will be available for placing in the content browser. Generated automatically by EditableEntityCollector.
*/
class SCR_PlaceableEntitiesRegistry
{	
	[Attribute("ArmaReforger",  "When using EditableEntityCollector, plugin will be looking for valid prefabs only in this addon", "")]
	private string m_sAddon;
	
	[Attribute(params: "unregFolders", desc: "When using EditableEntityCollector, the list will be filled with all compatible prefabs at this directory.")]
	private ResourceName m_sSourceDirectory;
	
	[Attribute(defvalue: "1", desc: "When true, prefabs will appear in the content browser.\nWhen false, they will be hidden, but stil available for placing by other systems.")]
	private bool m_bExposed;

	[Attribute(params: "et", desc: "List of placeable prefabs.")]
	private ref array<ResourceName> m_Prefabs;
	
	/*!
	Get the addon which the list covers.
	\return Addon name
	*/
	ResourceName GetAddon()
	{
		return m_sAddon;
	}
	/*!
	Get the directory which the list covers.
	\return Path to directory
	*/
	ResourceName GetSourceDirectory()
	{
		return m_sSourceDirectory;
	}
	/*!
	Check if the registry can be exposed in asset browser.
	\return True when exposed
	*/
	bool IsExposed()
	{
		return m_bExposed;
	}
	/*!
	Get all prefab infos in the list.
	\param[out] infos Array to be filled with prefabs
	*/
	array<ResourceName> GetPrefabs(bool onlyExposed = false)
	{
		if (onlyExposed && !m_bExposed)
		{
			//--- Return dummy array of the same size as prefabs array
			//--- ToDo: Don't create dummy array, but store starting index
			array<ResourceName> emptyPrefabs = {};
			emptyPrefabs.Resize(m_Prefabs.Count());
			return emptyPrefabs;
		}
		else
		{
			return m_Prefabs;
		}
	}
	
	void SetPrefabs(notnull array<ResourceName> prefabs)
	{
		if (!m_Prefabs) m_Prefabs = new array<ResourceName>;
		m_Prefabs.Copy(prefabs);
	}
	
	void AddPrefabs(notnull array<ResourceName> prefabs)
	{
		if (!m_Prefabs) 
			m_Prefabs = {};
		
		foreach (ResourceName prefab: prefabs)
		{
			m_Prefabs.Insert(prefab)
		}
		
	}
};

class SCR_PlaceableEntitiesRegistryTitleField: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		ResourceName sourceDirectory;
		source.Get("m_sSourceDirectory", sourceDirectory);
		title = sourceDirectory.GetPath();
		return true;
	}
};