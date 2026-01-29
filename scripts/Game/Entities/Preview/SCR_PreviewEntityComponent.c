[ComponentEditorProps(category: "GameScripted/Editor", description: "")]
class SCR_PreviewEntityComponentClass : ScriptComponentClass
{
	[Attribute("1", desc: "When enabled, preview prefab can be used also in runtime, not only when spawning from prefab.")]
	protected bool m_bRuntime;
	
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_PreviewPrefab;
	
	[Attribute()]
	protected ref array<ref SCR_BasePreviewEntry> m_aEntries;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPreviewPrefab()
	{
		return m_PreviewPrefab;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outEntries
	//! \param[in] entry
	//! \return
	int GetPreviewEntries(out notnull array<ref SCR_BasePreviewEntry> outEntries, SCR_BasePreviewEntry entry)
	{
		//outEntries.Clear();
		int count = m_aEntries.Count();
		SCR_BasePreviewEntry entryCopy;
		for (int i = 0; i < count; i++)
		{
			if (m_aEntries[i].m_iParentID == -1)
			{
				entry.m_Mesh = m_aEntries[i].m_Mesh;
			}
			else
			{
				entryCopy = new SCR_BasePreviewEntry();
				entryCopy.CopyFrom(m_aEntries[i]);
				//entryCopy.m_iParentID += entry.m_iParentID + 1;
				outEntries.Insert(entryCopy);
			}	
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsRuntime()
	{
		return m_bRuntime;
	}
}

class SCR_PreviewEntityComponent : ScriptComponent
{
	protected const string FILE_SUFFIX = "_Preview";
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetPreviewPrefab()
	{
		SCR_PreviewEntityComponentClass prefabData = SCR_PreviewEntityComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetPreviewPrefab();
		else
			return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] outEntries
	//! \param[in] entry
	//! \return
	int GetPreviewEntries(out notnull array<ref SCR_BasePreviewEntry> outEntries, SCR_BasePreviewEntry entry)
	{
		SCR_PreviewEntityComponentClass prefabData = SCR_PreviewEntityComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.GetPreviewEntries(outEntries, entry);
		else
			return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsRuntime()
	{
		SCR_PreviewEntityComponentClass prefabData = SCR_PreviewEntityComponentClass.Cast(GetComponentData(GetOwner()));
		if (prefabData)
			return prefabData.IsRuntime();
		else
			return false;
	}

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] entitySource
	static void GeneratePreviewEntries(WorldEditorAPI api, IEntitySource entitySource)
	{
		//--- Find this component
		IEntityComponentSource componentSource;
		for (int i = 0, count = entitySource.GetComponentCount(); i < count; i++)
		{
			componentSource = entitySource.GetComponent(i);
			if (componentSource.GetClassName().ToType().IsInherited(SCR_PreviewEntityComponent))
				break;
		}
		if (!componentSource)
			return;
		
		//--- Clear existing array
		BaseContainerList entriesList = componentSource.GetObjectArray("m_aEntries");
		int index = 0;
		while (entriesList && entriesList.Count() > index)
		{
			if (!api.RemoveObjectArrayVariableMember(entitySource, {ContainerIdPathEntry("SCR_PreviewEntityComponent")}, "m_aEntries", 0))
				index++;
		}
		
		array<ref SCR_BasePreviewEntry> entries = {};
		SCR_PrefabPreviewEntity.GetPreviewEntries(entitySource, entries, flags: EPreviewEntityFlag.IGNORE_PREFAB);
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			if (api.CreateObjectArrayVariableMember(entitySource, {ContainerIdPathEntry("SCR_PreviewEntityComponent")}, "m_aEntries", "SCR_BasePreviewEntry", i))
			{
				entries[i].Log(i);
				entries[i].SaveToContainer(api, entitySource, {ContainerIdPathEntry("SCR_PreviewEntityComponent"), ContainerIdPathEntry("m_aEntries", i)});
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] api
	//! \param[in] entitySource
	//! \param[in] componentSource
	void GeneratePreviewPrefab(WorldEditorAPI api, IEntitySource entitySource, IEntityComponentSource componentSource = null)
	{
		//--- Find this component
		if (!componentSource)
		{
			for (int i = 0, count = entitySource.GetComponentCount(); i < count; i++)
			{
				componentSource = entitySource.GetComponent(i);
				if (componentSource.GetClassName() == "SCR_PreviewEntityComponent")
					break;
			}
		}
		
		ResourceName prefab, previewPrefab;
		componentSource.Get("m_PreviewPrefab", previewPrefab);
		
		prefab = SCR_BaseContainerTools.GetPrefabResourceName(entitySource);
		if (!prefab)
		{
			Print("Entity is not a prefab!", LogLevel.WARNING);
			return;
		}

		//--- No preview prefab assigned, create one		
		if (!previewPrefab)
		{			
			//--- Get absolute path
			string absolutePath, absolutePathPreview, ext;
			Workbench.GetAbsolutePath(prefab.GetPath(), absolutePath, false);
			absolutePathPreview = FilePath.AppendExtension(FilePath.StripExtension(absolutePath, ext) + FILE_SUFFIX, ext);
			
			//--- Create dummy preview prefab
			IEntitySource emptyEntitySource = api.CreateEntity("SCR_PrefabPreviewEntity", string.Empty, 0, null, vector.Zero, vector.Zero);
			api.SetVariableValue(emptyEntitySource, null, "m_SourcePrefab", prefab);
			api.CreateEntityTemplate(emptyEntitySource, absolutePathPreview);
			api.DeleteEntity(emptyEntitySource);
			
			//--- Link preview prefab
			ResourceManager resourceManager = Workbench.GetModule(ResourceManager);
			MetaFile meta = resourceManager.GetMetaFile(absolutePathPreview);
			previewPrefab = meta.GetResourceID();
			api.SetVariableValue(entitySource, {ContainerIdPathEntry("SCR_PreviewEntityComponent")}, "m_PreviewPrefab", previewPrefab);
			//api.CreateEntityTemplate(entitySource, absolutePath);
		}
		
		IEntitySource previewSource = api.CreateEntity(previewPrefab, "", 0, null, vector.Zero, vector.Zero);
		SCR_PrefabPreviewEntity previewEntity = SCR_PrefabPreviewEntity.Cast(api.SourceToEntity(previewSource));
		previewEntity.CreatePrefabFromSource(api, prefab);
		api.DeleteEntity(previewSource);
	}
	
	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		switch (id)
		{
			case 0:
			{
				GenericEntity genericOwner = GenericEntity.Cast(owner);
				WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
				IEntitySource ownerSrc = api.EntityToSource(owner);
				//ownerSrc = ownerSrc.GetAncestor(); //--- Get prefab
				
				WBProgressDialog progress = new WBProgressDialog("Processing...", Workbench.GetModule(WorldEditor));
				
				api.BeginEntityAction();
				api.BeginEditSequence(ownerSrc);
				GeneratePreviewEntries(api, ownerSrc);
				api.EndEditSequence(ownerSrc);
				api.EndEntityAction();
				break;
			}

			case 1:
			{
				GenericEntity genericOwner = GenericEntity.Cast(owner);
				WorldEditorAPI api = genericOwner._WB_GetEditorAPI();
				IEntitySource ownerSrc = api.EntityToSource(owner);
				
				api.BeginEntityAction();
				GeneratePreviewPrefab(api, ownerSrc);
				api.EndEntityAction();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		return { new WB_UIMenuItem("Generate preview entries", 0), new WB_UIMenuItem("Generate preview prefab", 1) };
	}
#endif
}
