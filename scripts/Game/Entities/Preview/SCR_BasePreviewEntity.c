[EntityEditorProps(category: "GameScripted/Preview", description: "", color: "0 0 0 0", dynamicBox: true)]
class SCR_BasePreviewEntityClass: GenericEntityClass
{
};
/*!
Generic preview entity used to represent another entity is simplified form.
Uses array of SCR_BasePreviewEntry for configuration. Such array can be generated from prefab or existing entities, see inherited classes for details.
*/
class SCR_BasePreviewEntity: GenericEntity
{
	[Attribute(uiwidget: UIWidgets.Flags, category: "Preview Entity",  enums: ParamEnumArray.FromEnum(EPreviewEntityFlag))]
	protected EPreviewEntityFlag m_Flags;
	
	protected IEntity m_Entity;
	protected IEntitySource m_EntitySource;
	protected float m_fHeightTerrain = -1;
	protected bool m_bIsOnOrigTransform = true;
	protected vector m_vLocalTransform[4];
	protected vector m_vTerrainTransform[4];
	protected ref array<SCR_BasePreviewEntity> m_aChildren;
	protected vector m_vBounds[2];
	protected bool m_bHasMultipleEditableEntities;
	
	/*!
	Spawn preview entity from entries.
	To get entries, see specialized inherited classes.
	\param entries List of entity entries
	\param previewPrefab Prefab of preview entity. Can also be a class name, e.g., "SCR_RefPreviewEntity"
	\param world World in which the preview will be spawned
	\param spawnParams Spawn params of the preview
	\param material Material of the preview. When empty, meshes won't be created at all.
	\return Entity preview
	*/
	static SCR_BasePreviewEntity SpawnPreview(notnull array<ref SCR_BasePreviewEntry> entries, ResourceName previewPrefab, BaseWorld world = null, EntitySpawnParams spawnParams = null, ResourceName material = ResourceName.Empty, EPreviewEntityFlag flags = 0)
	{
		if (entries.IsEmpty())
		{
			Print("No entries defined!", LogLevel.WARNING);
			return null;
		}
		
		if (!world)
			world = GetGame().GetWorld();
		
		EntitySpawnParams spawnParamsLocal = spawnParams;
		if (!spawnParamsLocal)
		{
			spawnParamsLocal = new EntitySpawnParams();
			spawnParamsLocal.Transform[3] = entries[0].m_vPosition;
		}
		
		bool applyMesh = !material.IsEmpty();

		//--- Create root entity
		string ext;
		FilePath.StripExtension(previewPrefab, ext);
		Resource previewResource;
		typename previewType;
		bool spawnFromResource;
		if (!ext.IsEmpty())
		{
			previewResource = Resource.Load(previewPrefab);
			spawnFromResource = previewResource.IsValid();
		}
		
		SCR_BasePreviewEntity rootEntity;
		if (spawnFromResource)
		{
			//--- From prefab
			rootEntity = SCR_BasePreviewEntity.Cast(GetGame().SpawnEntityPrefab(previewResource, world, spawnParamsLocal));
		}
		else
		{
			//--- From class name
			previewType = previewPrefab.ToType();
			if (!previewType)
				previewType = SCR_BasePreviewEntity;
			rootEntity = SCR_BasePreviewEntity.Cast(GetGame().SpawnEntity(previewType, world, spawnParamsLocal));
		}
		
		if (!rootEntity)
		{
			Debug.Error2("SCR_BasePreviewEntity", string.Format("Unable to create preview entity from prefab/type '%1'!", previewPrefab));
			return null;
		}
		
		vector rootTransform[4];
		Math3D.MatrixCopy(spawnParamsLocal.Transform, rootTransform);
		
		rootEntity.m_Flags = flags;
		
		vector rootBoundMin = vector.One * float.MAX;
		vector rootBoundMax = -rootBoundMin;
		
		int editableEntityCount;
		
		array<SCR_BasePreviewEntity> children = {};
		SCR_BasePreviewEntity entity, parent;
		foreach (int i, SCR_BasePreviewEntry entry: entries)
		{
			//entry.Log(i);
			
			//--- Get local transformation matrix
			spawnParamsLocal = new EntitySpawnParams();
			entry.LoadTransform(spawnParamsLocal.Transform);
			
			if (entry.m_Shape != EPreviewEntityShape.ELLIPSE && entry.m_Shape != EPreviewEntityShape.RECTANGLE) //--- Don't scale area meshes; instead, the scale is applied on the actual area
				Math3D.MatrixScale(spawnParamsLocal.Transform, entry.GetScale()); //--- Apply scale on matrix, SetScale() doesn't work reliably
			
			//--- Get parent (don't apply it to spawn params, it doesn't create true hierarchy link)
			if (entry.m_iParentID == -1)
				parent = rootEntity;
			else
				parent = children[entry.m_iParentID];
			
			//--- Count how manuy editable entities are inside. When more than 1, editing along geometry is disabled.
			if (entry.m_Flags & EPreviewEntityFlag.EDITABLE)
				editableEntityCount++;
			
			if (applyMesh && entry.m_Shape == EPreviewEntityShape.PREFAB && entry.m_Mesh)
			{
				//--- Use preview prefab
				entity = SCR_BasePreviewEntity.Cast(GetGame().SpawnEntityPrefabLocal(Resource.Load(entry.m_Mesh), world, spawnParamsLocal));
			}
			else
			{
				//--- Create entity
				if (spawnFromResource)
					entity = SCR_BasePreviewEntity.Cast(GetGame().SpawnEntityPrefabLocal(previewResource, world, spawnParamsLocal));
				else
					entity = SCR_BasePreviewEntity.Cast(GetGame().SpawnEntity(previewType, world, spawnParamsLocal));
				
				if (entity && applyMesh)
				{
					switch (entry.m_Shape)
					{
						case EPreviewEntityShape.MESH:
						{
							//--- Set mesh from a file
							if (entry.m_Mesh)
							{
								Resource meshResource = Resource.Load(entry.m_Mesh);
								if (meshResource)
								{
									BaseResourceObject res = meshResource.GetResource();
									
									if (res)
										entity.SetPreviewObject(res.ToVObject(), material);
								}
							}
							break;
						}
						case EPreviewEntityShape.ELLIPSE:
						case EPreviewEntityShape.RECTANGLE:
						{
							int resolution = SCR_BaseAreaMeshComponent.PREVIEW_RESOLUTION;
							vector dimensions = entry.m_vScale;
							
							if (resolution <= 0 || dimensions[0] <= 0 || dimensions[1] <= 0 || dimensions[2] <= 0)
							{
								break;
							}
							
							array<vector> positions = {};
							
							if (entry.m_Shape == EPreviewEntityShape.ELLIPSE)
							{
								float dirStep = Math.PI2 / resolution;
							
								//--- Get positions
								for (int v = 0; v < resolution; v++)
								{
									float dir = dirStep * v;
									vector pos = Vector(Math.Sin(dir) * dimensions[0], -dimensions[1], Math.Cos(dir) * dimensions[2]);
									positions.Insert(pos);
								}
							}
							else if (entry.m_Shape == EPreviewEntityShape.RECTANGLE)
							{
								//~ Resulution is always 4 for Rectangles
								resolution = 4;
								
								//~ Make sure it uses half of the width and lenght
								float width = dimensions[0] / 2;
								float lenght = dimensions[2] / 2;
								
								array<vector> corners = {
									Vector(-width, -dimensions[1], -lenght),
									Vector(width, -dimensions[1], -lenght),
									Vector(width, -dimensions[1], lenght),
									Vector(-width, -dimensions[1], lenght)
								};
								
								//~ Set positions
								for (int p = 0; p < resolution; p++)
								{
									vector start = corners[p];
									vector end = corners[(p + 1) % resolution];
									
									for (float s = 0; s < 4; s++)
									{
										vector pos = vector.Lerp(start, end, s / resolution);
										positions.Insert(pos);
									}
								}
							}

							Resource res = SCR_Shape.CreateAreaMesh(positions, dimensions[1] * 2, material, true);
							MeshObject meshObject = res.GetResource().ToMeshObject();
							if (meshObject)
								entity.SetObject(meshObject, "");
						}
							
					}
				}
			}
			children.Insert(entity);
			
			//--- Update root bounding box
			vector boundMin, boundMax;
			entity.GetBounds(boundMin, boundMax);
			boundMin += entry.m_vPosition;
			boundMax += entry.m_vPosition;
			
			rootBoundMin[0] = Math.Min(rootBoundMin[0], boundMin[0]);
			rootBoundMin[1] = Math.Min(rootBoundMin[1], boundMin[1]);
			rootBoundMin[2] = Math.Min(rootBoundMin[2], boundMin[2]);
			
			rootBoundMax[0] = Math.Max(rootBoundMax[0], boundMax[0]);
			rootBoundMax[1] = Math.Max(rootBoundMax[1], boundMax[1]);
			rootBoundMax[2] = Math.Max(rootBoundMax[2], boundMax[2]);
			
			//--- Add to parent (spawn params won't do that on their own)
			int pivot = -1;
			if (!entry.m_iPivotID.IsEmpty() && parent.GetAnimation())
				pivot = parent.GetAnimation().GetBoneIndex(entry.m_iPivotID);
			parent.AddChild(entity, pivot, EAddChildFlags.AUTO_TRANSFORM);
			
			//--- Cache the child in parent's array
			if (!parent.m_aChildren)
				parent.m_aChildren = {};
			parent.m_aChildren.Insert(entity);
			
			//--- Set coordinates local to terrain
			Math3D.AnglesToMatrix(entry.m_vAnglesTerrain, entity.m_vTerrainTransform);
			entity.m_fHeightTerrain = entry.m_vHeightTerrain;
			
			//--- Make a copy of local transform for use in SetPreviewTransform()
			Math3D.MatrixCopy(spawnParamsLocal.Transform, entity.m_vLocalTransform);
			
			//--- Initialize
			entity.m_Flags |= entry.m_Flags;
			entity.m_Entity = entry.m_Entity;
			
			entity.EOnPreviewInit(entry, rootEntity);
		}
		rootEntity.m_vBounds[0] = rootBoundMin;
		rootEntity.m_vBounds[1] = rootBoundMax;
			
		//--- When there is more than one editable entity, mark it. Editing along geometry is disallowed in such case.
		rootEntity.m_bHasMultipleEditableEntities = editableEntityCount > 1;
		
		rootEntity.EOnRootPreviewInit(entries);
		
		Print(string.Format("Preview entity created from %1 entries, at %2, using '%3' with material '%4'", entries.Count(), rootTransform, previewPrefab, material), LogLevel.VERBOSE);
		
		return rootEntity;
	}
	
	/*!
	Set transformation of the preview.
	Entities will be oriented to terrain when when they have the functionality enabled.
	\param worldTransform Transformation matrix in world space
	\param verticalMode Type of vertical editing
	\param heightTerrain Height above terrain
	\param isUnderwater True when the preview entity is under ocean surface
	*/
	void SetPreviewTransform(vector worldTransform[4], EEditorTransformVertical verticalMode, float heightTerrain = 0, bool isUnderwater = false, TraceParam trace = null)
	{
		//--- Get height difference
		if (m_fHeightTerrain == -1)
			m_fHeightTerrain = heightTerrain;
		heightTerrain -= m_fHeightTerrain;
		
		SetWorldTransform(worldTransform);
		SetChildTransform(verticalMode, heightTerrain, isUnderwater, trace);
		Update();
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG))
		{
			DbgUI.Begin(this.ToString(), 0, 0);
			SCR_DbgUI.Matrix("worldTransform", worldTransform);
			SCR_Global.DrawMatrix(worldTransform, 1);
			DbgUI.End();
		}
	}
	protected void SetChildTransform(EEditorTransformVertical verticalMode, float heightTerrain, bool isUnderwater = false, TraceParam trace = null)
	{
		if (GetParent())
		{
			//--- Actual preview entity, not root
			switch (verticalMode)
			{
				case EEditorTransformVertical.GEOMETRY:
				case EEditorTransformVertical.TERRAIN:
				{
					//--- Restore local transformation and convert it to world transformation
					vector transform[4], surfaceBasis[4];
					GetParent().GetWorldTransform(transform);
					Math3D.MatrixMultiply4(transform, m_vLocalTransform, transform);
					
					float scale = transform[0].Length();
					
					//--- Get surface basis
					if (!SCR_TerrainHelper.GetTerrainBasis(transform[3], surfaceBasis, GetWorld(), !isUnderwater, trace))
						return;

					vector angles = Math3D.MatrixToAngles(transform);
					if (SCR_Enum.HasFlag(m_Flags, EPreviewEntityFlag.HORIZONTAL))
					{
						//--- Orient to horizontal normal
						angles[1] = 0;
						angles[2] = 0;
						Math3D.AnglesToMatrix(angles, transform);
					}
					else
					{
						//--- Orient to terrain normal
						//--- Get identity matrix rotated according to the entity
						Math3D.AnglesToMatrix(Vector(angles[0], 0, 0), transform);
						
						//--- Rotate surface basis
						Math3D.MatrixMultiply3(surfaceBasis, transform, surfaceBasis);
						
						//--- Apply local transformation relative to terrain
						Math3D.MatrixMultiply3(surfaceBasis, m_vTerrainTransform, transform);
					}
					
					//--- Apply height
					transform[3][1] = surfaceBasis[3][1] + m_fHeightTerrain + heightTerrain;
					
					//--- Preserve scale
					if (scale != 1)
						Math3D.MatrixScale(transform, scale);
					
					//--- Apply
					SetWorldTransform(transform);
					Update();
					
					m_bIsOnOrigTransform = false;
					break;
				}
				default:
				{
					//--- Reset transformation
					if (!m_bIsOnOrigTransform)
					{
						SetLocalTransform(m_vLocalTransform);
						m_bIsOnOrigTransform = true;
					}
					break;
				}
			}
		}
		if (m_aChildren && (!GetParent() || SCR_Enum.HasFlag(m_Flags, EPreviewEntityFlag.ORIENT_CHILDREN)))
		{
			//--- Is root, or has children and is allowed to orient them
			for (int i = 0, count = m_aChildren.Count(); i < count; i++)
			{
				m_aChildren[i].SetChildTransform(verticalMode, heightTerrain, isUnderwater, trace);
			}
		}
		
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG))
		{
			vector matrix[4];
			GetWorldTransform(matrix);
			SCR_Global.DrawMatrix(matrix, 1, colorX: Color.CYAN, colorY: Color.MAGENTA, colorZ: 0xffffff00);
		}
	}
	protected void SetPreviewObject(VObject mesh, ResourceName material)
	{
		if (!mesh)
			return;
		
		string remap = string.Empty;
		string materials[256];
		int numMats = mesh.GetMaterials(materials);
		for (int i = 0; i < numMats; i++)
		{
			remap += string.Format("$remap '%1' '%2';", materials[i], material);
		}
		SetObject(mesh, remap);
	}
	
	/*!
	Get all direct preview entity children.
	\return Array of children
	*/
	array<SCR_BasePreviewEntity> GetPreviewChildren()
	{
		return m_aChildren;
	}
	/*!
	Get entity from which the preview was copied from.
	\return Source entity, or null if the preview was not created from an entity
	*/
	IEntity GetSourceEntity()
	{
		return m_Entity;
	}
	/*!
	Get local bounding box of the preview entity (including all children)
	\param[out] outBoundMin Lower corner
	\param[out] outBoundMax Upper corner
	*/
	void GetPreviewBounds(out vector outBoundMin, out vector outBoundMax)
	{
		outBoundMin = m_vBounds[0];
		outBoundMax = m_vBounds[1]
	}
	 /*!
	\return True if the preview entity represents multiple editable entities (e.g., composition with editable children, or editing multiple editable entities at once).
	*/
	bool HasMultipleEditableEntities()
	{
		return m_bHasMultipleEditableEntities;
	}
	/*!
	Init event called when the preview is created.
	To be overridden by child classes
	*/
	protected void EOnPreviewInit(SCR_BasePreviewEntry entry, SCR_BasePreviewEntity root);
	/*!
	Init event called on preview root entity.
	To be overridden by child classes
	*/
	protected void EOnRootPreviewInit(array<ref SCR_BasePreviewEntry> entries);
	
	void SCR_BasePreviewEntity(IEntitySource src, IEntity parent)
	{
	}
	void ~SCR_BasePreviewEntity()
	{
		while (GetChildren())
		{
			delete GetChildren();
		}
	}
	
#ifdef WORKBENCH
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		GetBounds(min, max);
	}
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		//--- Show mesh name for easier World Editor debugging
		if (_WB_GetEditorAPI() && _WB_GetEditorAPI().IsEntitySelectedAsMain(_WB_GetEditorAPI().EntityToSource(this)))
		{
			string text = "<No Mesh>";
			if (GetVObject())
				text = FilePath.StripPath(GetVObject().GetResourceName());
			
			vector pos = GetOrigin();
			DebugTextWorldSpace.Create(GetWorld(), text, DebugTextFlags.ONCE | DebugTextFlags.CENTER, pos[0], pos[1], pos[2], 12, Color.WHITE, Color.BLACK);
		}
	}
#endif
};