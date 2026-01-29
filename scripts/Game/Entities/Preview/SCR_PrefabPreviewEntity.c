[EntityEditorProps(category: "GameScripted/Preview", color: "0 0 0 0", dynamicBox: true)]
class SCR_PrefabPreviewEntityClass: SCR_BasePreviewEntityClass
{
};
/*!
Preview entity created from entity prefab.
*/
class SCR_PrefabPreviewEntity: SCR_BasePreviewEntity
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, category: "Prefab Preview Entity", params: "et")]
	protected ResourceName m_SourcePrefab;
	
	[Attribute("{58F07022C12D0CF5}Assets/Editor/PlacingPreview/Preview.emat", UIWidgets.ResourcePickerThumbnail, "Material used when regenerating the preview.", category: "Preview", params: "emat")]
	private ResourceName m_PreviewMaterial;
	
	/*!
	Spawn preview entity from prefab resource.
	\param prefabResource Entity prefab resource
	\param previewPrefab Prefab from which the preview entity will be spawned from. Apart from file path, it can be also class name
	\param world World in which the preview will be spawned
	\param spawnParams Spawn params of the preview
	\param material Material of the preview
	\return Entity preview
	*/
	static SCR_BasePreviewEntity SpawnPreviewFromPrefab(Resource prefabResource, ResourceName previewPrefab, BaseWorld world = null, EntitySpawnParams spawnParams = null, ResourceName material = ResourceName.Empty, EPreviewEntityFlag flags = 0)
	{
		return SpawnPreview(GetPreviewEntriesFromPrefab(prefabResource), previewPrefab, world, spawnParams, material, flags);
	}
	
	/*!
	Get unifiedpreview configuration from prefab resource.
	\param prefabResource Prefab resource
	\return Array of configuration entries.
	*/
	static array<ref SCR_BasePreviewEntry> GetPreviewEntriesFromPrefab(Resource prefabResource)
	{
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(prefabResource);
		array<ref SCR_BasePreviewEntry> entries = {};
		GetPreviewEntries(entitySource, entries);
		return entries;
	}
	
	/*!
	Get preview entries from entity source.
	\param entitySource Entity source
	\param[out] outEntries Array filled with preview entries
	*/
	static void GetPreviewEntries(IEntitySource entitySource, out notnull array<ref SCR_BasePreviewEntry> outEntries, int parentID = -1, SCR_BasePreviewEntry entry = null, EPreviewEntityFlag flags = 0)
	{
		EntityFlags entityFlags;
		entitySource.Get("Flags", entityFlags);
		if (entityFlags & EntityFlags.EDITOR_ONLY)
			return;
		
		//--- Get transformation (if it's not defined in a param)
		SCR_BasePreviewEntry entryLocal = entry;
		if (!entryLocal)
		{
			entryLocal = new SCR_BasePreviewEntry(true);
			
			if (parentID != -1) //--- Always center prefab root to 0,0,0
				entitySource.Get("coords", entryLocal.m_vPosition);
			
			
			float scale; 
			entitySource.Get("scale", scale);
			entryLocal.SetScale(scale);
			
			//if (parentID == 0)
			//	entryLocal.m_fScale *= entryLocal.m_fScale; //--- Compensate for parent scale (why though?)
			
			entitySource.Get("angles", entryLocal.m_vAngles);

		}
		entryLocal.m_EntitySource = entitySource;
		entryLocal.m_iParentID = parentID;
		entryLocal.m_vAnglesTerrain = Vector(0, entryLocal.m_vAngles[0], entryLocal.m_vAngles[2]);
		entryLocal.m_vHeightTerrain = entryLocal.m_vPosition[1];
		parentID = outEntries.Insert(entryLocal);
		
		//--- Find relevant components
		int componentCount = entitySource.GetComponentCount();
		IEntityComponentSource componentSource;
		bool componentEnabled;
		
		//--- First pass - get flags and check if it has preview prefab
		for (int i = 0; i < componentCount; i++)
		{
			componentSource = entitySource.GetComponent(i);
			componentSource.Get("Enabled", componentEnabled);
			if (!componentEnabled)
				continue;
			
			if (componentSource.GetClassName().ToType().IsInherited(SCR_PreviewEntityComponent) && !SCR_Enum.HasFlag(flags, EPreviewEntityFlag.IGNORE_PREFAB))
			{
				//--- Entries pre-defined in the component, use them instead of scanning entity source dynamically
				BaseContainerList entryList = componentSource.GetObjectArray("m_aEntries");
				int entryCount = entryList.Count();
				if (entryCount > 0)
				{
					//--- Preview defined as an array of entries
					int indexStart = 0;
					for (int e = entryCount - 1; e >= 0; e--)
					{
						int itemParentID;
						if (entryList.Get(e).Get("m_iParentID", itemParentID) && itemParentID == -1)
						{
							//--- If multiple parents are detected, use only the last one and following children. Can happen in inherited prefabs that contain parent's preview.
							indexStart = e;
							break;
						}
					}
					
					SCR_BasePreviewEntry listEntry;
					for (int e = indexStart; e < entryCount; e++)
					{
						listEntry = SCR_BasePreviewEntry.Cast(BaseContainerTools.CreateInstanceFromContainer(entryList.Get(e)));
						if (listEntry.m_iParentID == -1)
						{
							//--- Apply custom icon offset (only if the entity is root)
							if (parentID == 0)
								entryLocal.m_vPosition == listEntry.m_vPosition;
							
							entryLocal.m_Mesh = listEntry.m_Mesh;
							entryLocal.m_Shape = listEntry.m_Shape;
							entryLocal.m_Flags = listEntry.m_Flags;
						}
						else
						{
							outEntries.Insert(listEntry);
							listEntry.m_iParentID += parentID;
							listEntry.m_vAnglesTerrain = Vector(0, listEntry.m_vAngles[0], listEntry.m_vAngles[2]);
							listEntry.m_vHeightTerrain = listEntry.m_vPosition[1];
						}
					}
					if (entryCount > 0)
						return;
				}
				else
				{
					//--- Preview defined as a prefab, use that one
					componentSource.Get("m_PreviewPrefab", entryLocal.m_Mesh);
					entryLocal.m_Shape = EPreviewEntityShape.PREFAB;
					return;
				}
			}
		}
		
		//--- Second pass (when no preview prefab exists) - get mesh and process children
		string componentClassName;
		typename componentType;
		EAreaMeshShape areaMeshShape;
		for (int i = 0; i < componentCount; i++)
		{
			componentSource = entitySource.GetComponent(i);
			componentSource.Get("Enabled", componentEnabled);
			if (!componentEnabled)
				continue;
			
			componentClassName = componentSource.GetClassName();
			componentType = componentClassName.ToType();
			switch (true)
			{
				case (componentClassName == "Hierarchy" && !entryLocal.m_iPivotID):
				{
					componentSource.Get("PivotID", entryLocal.m_iPivotID);
					break;
				}
				case (componentClassName == "MeshObject"):
				{
					componentSource.Get("Object", entryLocal.m_Mesh);
					entryLocal.m_Shape = EPreviewEntityShape.MESH;
					break;
				}
				case (componentType.IsInherited(SlotManagerComponent)):
				{
					BaseContainerList slots = componentSource.GetObjectArray("Slots");
					for (int s = 0, slotCount = slots.Count(); s < slotCount; s++)
					{
						GetPreviewEntriesFromSlot(slots.Get(s), outEntries, parentID);
					}
					break;
				}
				case (componentType.IsInherited(BaseSlotComponent) || componentType.IsInherited(WeaponSlotComponent)):
				{
					BaseContainer attachType = componentSource.GetObject("AttachType");
					if (!attachType)
						break;
					
					ResourceName slotPrefab;
					componentSource.Get("EntityPrefab", slotPrefab);
					if (!slotPrefab)
						componentSource.Get("WeaponTemplate", slotPrefab);
					
					GetPreviewEntriesFromSlot(attachType, outEntries, parentID, slotPrefab);
					break;
				}
				case (componentType.IsInherited(SCR_EditorLinkComponent)):
				{
					BaseContainerList entries = componentSource.GetObjectArray("m_aEntries");
					SCR_EditorLinkEntry link;
					for (int e = 0, linkCount = entries.Count(); e < linkCount; e++)
					{
						link = SCR_EditorLinkEntry.Cast(BaseContainerTools.CreateInstanceFromContainer(entries.Get(e)));
						GetPreviewEntriesFromLink(link, outEntries, parentID, entryLocal.GetScale());
					}
					break;
				}
				case (componentType.IsInherited(SCR_EditableEntityComponent)):
				{
					EEditableEntityFlag editableFlags = SCR_EditableEntityComponentClass.GetEntityFlags(componentSource);
					
					entryLocal.m_Flags |= EPreviewEntityFlag.EDITABLE;
					
					if (SCR_Enum.HasFlag(editableFlags, EEditableEntityFlag.HORIZONTAL))
						entryLocal.m_Flags |= EPreviewEntityFlag.HORIZONTAL;
					
					if (SCR_Enum.HasFlag(editableFlags, EEditableEntityFlag.ORIENT_CHILDREN))
						entryLocal.m_Flags |= EPreviewEntityFlag.ORIENT_CHILDREN;
					
					vector iconPos;
					componentSource.Get("m_vIconPos", iconPos);
					if (iconPos != vector.Zero)
						entryLocal.m_vPosition -= iconPos;
					
					break;
				}
				case (componentType.IsInherited(SCR_SlotCompositionComponent)):
				{
					bool orientChildren;
					componentSource.Get("m_bOrientChildrenToTerrain", orientChildren);
					if (orientChildren)
						entryLocal.m_Flags |= EPreviewEntityFlag.ORIENT_CHILDREN;
					
					break;
				}				
				case (componentType.IsInherited(SCR_HorizontalAlignComponent)):
				{
					entryLocal.m_Flags |= EPreviewEntityFlag.HORIZONTAL;
					break;
				}
				//~ Area mesh previews. Add any new area mesh preview here that inherent from SCR_BaseAreaMeshComponent!
				case (componentType.IsInherited(SCR_BaseAreaMeshComponent)):
				{
					//~ Only show area mesh preview if editor is open and unlimited
					SCR_EditorModeEntity mode = SCR_EditorModeEntity.GetInstance();
					if (!SCR_EditorManagerEntity.IsOpenedInstance() || !mode || mode.IsLimited())
						break;
						
					//~ Area mesh previews
					switch (true)
					{
						case (componentType.IsInherited(SCR_TriggerAreaMeshComponent)):
						{
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
							
							float radius, height;
							entitySource.Get("SphereRadius", radius);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_WaypointAreaMeshComponent)):
						{
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
							
							float radius, height;
							entitySource.Get("CompletionRadius", radius);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_SpawnPointAreaMeshComponent)):
						{
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
							
							float radius, height;
							entitySource.Get("m_fSpawnRadius", radius);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_CustomAreaMeshComponent)):
						{
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
							
							float radius, height;
							componentSource.Get("m_fRadius", radius);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_ZoneRestrictionAreaMeshComponent)):
						{
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
							
							float radius, height;
							entitySource.Get("m_fZoneRadius", radius);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_SupportStationAreaMeshComponent)):
						{						
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
								
							//~ Get SCR_BaseSupportStationComponent
							for (int c = 0; c < componentCount; c++)
							{
								IEntityComponentSource supportStation_componentSource = entitySource.GetComponent(c);
								
								bool supportStation_componentEnabled;
								supportStation_componentSource.Get("Enabled", supportStation_componentEnabled);
								if (!supportStation_componentEnabled)
									continue;
								
								string supportStation_componentClassName = supportStation_componentSource.GetClassName();
								if (!supportStation_componentClassName.ToType().IsInherited(SCR_BaseSupportStationComponent))
									continue;
								
								float radius, height;
								supportStation_componentSource.Get("m_fRange", radius);
								componentSource.Get("m_fHeight", height);
								
								entryLocal.m_vScale = Vector(radius, height, radius);
								break;
							}
							
							//~ Fails to get the radius from the Support Station component so use preview instead
							float radius, height;
							componentSource.Get("m_fRadius", radius);
							componentSource.Get("m_fHeight", height);
							entryLocal.m_vScale = Vector(radius, height, radius);
							
							break;
						}
						case (componentType.IsInherited(SCR_EffectsModuleAreaMeshComponent)):
						{						
							//~ Get Shape
							componentSource.Get("m_eShape", areaMeshShape);
							if (areaMeshShape == EAreaMeshShape.ELLIPSE)
								entryLocal.m_Shape = EPreviewEntityShape.ELLIPSE;
							else if (areaMeshShape == EAreaMeshShape.RECTANGLE)
								entryLocal.m_Shape = EPreviewEntityShape.RECTANGLE;
							else
								break;
								
							float width, lenght, height;
							componentSource.Get("m_fWidth", width);
							componentSource.Get("m_fLenght", lenght);
							componentSource.Get("m_fHeight", height);
							
							entryLocal.m_vScale = Vector(width, height, lenght);							
							break;
						}
						
						break;
					}
				}
			}
		}
		
		//--- Spawn AI group members
		array<ResourceName> groupPrefabs = new array<ResourceName>;
		array<vector> groupOffsets = new array<vector>;
		int groupSize = SCR_AIGroupClass.GetMembers(entitySource, groupPrefabs, groupOffsets);
		if (groupSize != 0)
		{
			SCR_BasePreviewEntry groupMemberEntry;
			IEntitySource groupMemberSource;
			for (int i = 0; i < groupSize; i++)
			{
				Resource resource = Resource.Load(groupPrefabs[i]);
				groupMemberSource = SCR_BaseContainerTools.FindEntitySource(resource);
				groupMemberEntry = new SCR_BasePreviewEntry(true);
				groupMemberEntry.m_vPosition = groupOffsets[i];
				GetPreviewEntries(groupMemberSource, outEntries, parentID, groupMemberEntry);
			}
		}
		
		//--- Process children
		for (int i = 0, count = entitySource.GetNumChildren(); i < count; i++)
		{
			GetPreviewEntries(entitySource.GetChild(i), outEntries, parentID);
		}
	}
	
	protected static void GetPreviewEntriesFromSlot(BaseContainer slotSource, out notnull array<ref SCR_BasePreviewEntry> outEntries, int parentID = -1, ResourceName slotPrefab = ResourceName.Empty)
	{
		if (!slotPrefab)
			slotSource.Get("Prefab", slotPrefab);
		
		if (!slotPrefab)
			return;
		
		bool isEnabled;
		slotSource.Get("Enabled", isEnabled);
		
		if (!isEnabled)
			return;
		
		//Don't show supply boxes at vehicle previews
		if (slotSource.GetName().Contains("SupplyStorage"))
			return;
		
		Resource resource = Resource.Load(slotPrefab);
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;
		
		SCR_BasePreviewEntry entry = new SCR_BasePreviewEntry(true);
		slotSource.Get("Offset", entry.m_vPosition);
		slotSource.Get("Angles", entry.m_vAngles);
		slotSource.Get("PivotID", entry.m_iPivotID);
		
		GetPreviewEntries(entitySource, outEntries, parentID, entry);
	}
	protected static void GetPreviewEntriesFromLink(SCR_EditorLinkEntry link, out notnull array<ref SCR_BasePreviewEntry> outEntries, int parentID = -1, float parentScale = 1)
	{
		Resource resource = Resource.Load(link.m_Prefab);
		IEntitySource entitySource = SCR_BaseContainerTools.FindEntitySource(resource);
		if (!entitySource)
			return;
		
		SCR_BasePreviewEntry entry = new SCR_BasePreviewEntry(true);
		entry.m_vPosition = link.m_vPosition;
		entry.m_vAngles = link.m_vAngles;
		entry.SetScale(link.m_fScale);
		
		GetPreviewEntries(entitySource, outEntries, parentID, entry);
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef WORKBENCH
	void CreatePrefabFromSource(WorldEditorAPI api, ResourceName basePrefab)
	{
		Resource baseResource = Resource.Load(basePrefab);
		if (!baseResource.IsValid())
			return;
		
		IEntitySource baseSource = baseResource.GetResource().ToEntitySource();
		if (!baseSource)
			return;
		
		array<ref SCR_BasePreviewEntry> entries = {};
		GetPreviewEntries(baseSource, entries, flags: EPreviewEntityFlag.IGNORE_PREFAB);
		
		//--- Create prefab
		IEntitySource child, parent;
		IEntityComponentSource component;
		array<ref IEntitySource> children = {};
		string className = "SCR_PrefabPreviewEntity";
		foreach (int i, SCR_BasePreviewEntry entry: entries)
		{
			if (entry.m_iParentID == -1) 
				parent = null;
			else
				parent = children[entry.m_iParentID];
			
			child = api.CreateEntity(className, "", 0, parent, entry.m_vPosition, entry.m_vAngles);
			children.Insert(child);
			
			child.Set("scale", entry.GetScale());
			child.Set("m_Flags", entry.m_Flags);
			
			if (i == 0)
			{
				className = "SCR_BasePreviewEntity";
			}
			else
			{
				api.CreateComponent(child, "Hierarchy");
				
				if (!entry.m_iPivotID.IsEmpty())
					api.SetVariableValue(child, {ContainerIdPathEntry("Hierarchy")}, "PivotID", entry.m_iPivotID);
				
				//EntityFlags flags;
				//child.Get("Flags", flags);
				//child.Set("Flags", flags | EntityFlags.PROXY);
			}
			
			if (entry.m_Mesh)
			{
				component = api.CreateComponent(child, "MeshObject");
				api.SetVariableValue(child, {ContainerIdPathEntry("MeshObject")}, "Object", entry.m_Mesh);

				Resource resource = Resource.Load(entry.m_Mesh);
				if (!resource.IsValid())
				{
					Print("Cannot load " + entry.m_Mesh + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
					return;
				}

				VObject mesh = resource.GetResource().ToVObject();
				string materials[256];
				for (int m = 0, materialsCount = mesh.GetMaterials(materials); m < materialsCount; m++)
				{
					api.CreateObjectArrayVariableMember(child, {ContainerIdPathEntry("MeshObject")}, "Materials", "MaterialAssignClass", m);
					api.SetVariableValue(child, {ContainerIdPathEntry("MeshObject"), ContainerIdPathEntry("Materials", m)}, "SourceMaterial", materials[m]);
					api.SetVariableValue(child, {ContainerIdPathEntry("MeshObject"), ContainerIdPathEntry("Materials", m)}, "AssignedMaterial", m_PreviewMaterial);
				}
			}
		}
		
		IEntitySource previewSource = children[0];
		api.SetVariableValue(previewSource, null, "m_SourcePrefab", basePrefab);
		
		//--- Save to file
		ResourceName prefab = SCR_BaseContainerTools.GetPrefabResourceName(api.EntityToSource(this));
		string absolutePath;
		Workbench.GetAbsolutePath(prefab.GetPath(), absolutePath);
		api.CreateEntityTemplate(previewSource, absolutePath);
		
		api.DeleteEntity(previewSource);
	}
	
	override void _WB_OnContextMenu(int id)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource entitySource = api.EntityToSource(this);
		
		ResourceName basePrefab;
		entitySource.Get("m_SourcePrefab", basePrefab);
		
		if (!basePrefab)
		{
			Debug.Error2(Type().ToString(), "m_SourcePrefab is empty!");
			return;
		}
		
		api.BeginEntityAction();
		CreatePrefabFromSource(api, basePrefab);
		api.EndEntityAction();
	}
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems()
	{
		return { new WB_UIMenuItem("Generate preview prefab", 0) };
	}
#endif
};