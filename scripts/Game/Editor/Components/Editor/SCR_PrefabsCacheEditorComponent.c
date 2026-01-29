[ComponentEditorProps(category: "GameScripted/Editor", description: "Attribute for managing editable entities. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PrefabsCacheEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/
/*!
Cache of editable entity prefabs and their ancestors.

Used for example when identifying compatible slots for snapping.
*/
class SCR_PrefabsCacheEditorComponent : SCR_BaseEditorComponent
{
	private ref map<ResourceName, ref array<ResourceName>> m_PrefabsCache = new map<ResourceName, ref array<ResourceName>>;
	
	bool IsPrefabInherited(ResourceName prefab, ResourceName inheritedFrom)
	{
		if (prefab == ResourceName.Empty || inheritedFrom == ResourceName.Empty) return false;
		
		array<ResourceName> inheritance = new array<ResourceName>;
		if (!m_PrefabsCache.Contains(inheritedFrom))
		{
			Resource resource = Resource.Load(inheritedFrom);
			if (resource.IsValid())
			{
				IEntitySource source = SCR_BaseContainerTools.FindEntitySource(resource);
				while (source)
				{
					inheritance.Insert(source.GetResourceName());
					source = source.GetAncestor();
				}
				m_PrefabsCache.Insert(inheritedFrom, inheritance);
			}
		}
		else
		{
			inheritance = m_PrefabsCache.Get(inheritedFrom);
		}
		
		return inheritance.Find(prefab) != -1;
	}
};