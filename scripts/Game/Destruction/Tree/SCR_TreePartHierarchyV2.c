//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_TreePartHierarchyV2 : Managed
{
	[Attribute("-1", UIWidgets.ResourceNamePicker, "Pick a prefab for this tree part", "et")]
	ResourceName m_Prefab;
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_TreePartHierarchyV2> m_aChildren;
	
#ifdef ENABLE_DESTRUCTION
	void SpawnAllChildren(IEntity parent, array<SCR_TreePartV2> treeParts = null)
	{
		if (!parent)
			return;
		
		int count = m_aChildren.Count();
		for (int i = 0; i < count; i++)
		{
			if (!m_aChildren[i])
				return;
			if (m_aChildren[i].m_Prefab.GetPath() == "-1")
				return;
			
			Resource resource = Resource.Load(m_aChildren[i].m_Prefab);		
			if (!resource.IsValid())
				return;
			
			vector mat[4];
			parent.GetTransform(mat);
			
			ArmaReforgerScripted game = GetGame();
			if (!game)
				continue;
			
			IEntity ent = game.SpawnEntityPrefab(resource);
			
			if (RplSession.Mode() == RplMode.Client)
				parent.AddChild(ent, -1, EAddChildFlags.AUTO_TRANSFORM);
			else parent.AddChild(ent, -1, EAddChildFlags.NONE);
			ent.SetTransform(mat);
			
			if (treeParts)
			{
				SCR_TreePartV2 treePart = SCR_TreePartV2.Cast(ent);
				if (treePart)
				{
					treePart.m_vLockedOrigin = parent.GetOrigin();
					treeParts.Insert(treePart);
				}
			}
			
			m_aChildren[i].SpawnAllChildren(ent, treeParts);
		}
	}
	
	void ~SCR_TreePartHierarchyV2()
	{
		foreach (SCR_TreePartHierarchyV2 hierarchy : m_aChildren)
		{
			hierarchy = null;
		}
		m_aChildren = null;
	}
#endif
};
